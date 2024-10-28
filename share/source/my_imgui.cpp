#include <stdint.h>
#include <vita2d.h>
#include <imgui_vita2d/imgui_vita_touch.h>
#include <zlib.h>
#include "my_imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_vita2d/imgui_internal.h>
#include "language_define.h"
#include "log.h"
#include "gb2312.i"

#define APP_ASSETS_DIR "app0:assets"
#define TEXT_FONT_NAME "FZLantingHei.ttf"
#define GAMEPAD_FONT_NAME "promptfont.ttf"
#define ICON_FONT_NAME "fontello.ttf"
#define FONT_CACHE_VERSION 1

extern SceGxmProgram _binary_assets_imgui_v_cg_gxp_start;
extern SceGxmProgram _binary_assets_imgui_f_cg_gxp_start;
static vita2d_texture *gFontTexture = nullptr;

const static ImWchar GamePadCharset[] = {0x219c, 0x21a1,
                                         0x21b0, 0x21b3,
                                         0x21bc, 0x21c3,
                                         0x21cb, 0x21cc,
                                         0x21d0, 0x21d3,
                                         0x21e0, 0x21e3,
                                         0x21f7, 0x21f8,
                                         0xe000, 0xe000,
                                         0x0000};

const static ImWchar IconCharset[] = {0xe800, 0xe80c,
                                      0xf08e, 0xf08e,
                                      0xf0c9, 0xf0c9,
                                      0xf11b, 0xf11b,
                                      0xf135, 0xf135,
                                      0xf1de, 0xf1de,
                                      0x0000};

const static ImWchar RomanNumCharset[] = {0x2160, 0x216c, 0x0000};

static void matrix_init_orthographic(float *m,
                                     float left,
                                     float right,
                                     float bottom,
                                     float top,
                                     float near,
                                     float far)
{
    m[0x0] = 2.0f / (right - left);
    m[0x4] = 0.0f;
    m[0x8] = 0.0f;
    m[0xC] = -(right + left) / (right - left);

    m[0x1] = 0.0f;
    m[0x5] = 2.0f / (top - bottom);
    m[0x9] = 0.0f;
    m[0xD] = -(top + bottom) / (top - bottom);

    m[0x2] = 0.0f;
    m[0x6] = 0.0f;
    m[0xA] = -2.0f / (far - near);
    m[0xE] = (far + near) / (far - near);

    m[0x3] = 0.0f;
    m[0x7] = 0.0f;
    m[0xB] = 0.0f;
    m[0xF] = 1.0f;
}

SceGxmShaderPatcherId imguiVertexProgramId;
SceGxmShaderPatcherId imguiFragmentProgramId;

SceGxmVertexProgram *_vita2d_imguiVertexProgram;
SceGxmFragmentProgram *_vita2d_imguiFragmentProgram;

const SceGxmProgramParameter *_vita2d_imguiWvpParam;

float ortho_proj_matrix[16];

constexpr auto ImguiVertexSize = 20;

struct FontCache
{
    uint32_t version;
    float size;
    uint32_t glyphs_size;
    int width;
    int height;
    ImVec2 white_uv;
};

static bool save_font_cache(const char *path)
{
    LogFunctionName;
    if (!gFontTexture)
    {
        return false;
    }

    FILE *fp = fopen(path, "wb");
    if (!fp)
    {
        return false;
    }
    ImFontAtlas *fonts = ImGui::GetIO().Fonts;

    FontCache cache;
    uint8_t *pixels;
    fonts->GetTexDataAsAlpha8(&pixels, &cache.width, &cache.height);
    cache.version = FONT_CACHE_VERSION;
    cache.white_uv = fonts->TexUvWhitePixel;
    cache.glyphs_size = fonts->Fonts[0]->Glyphs.size();
    cache.size = fonts->Fonts[0]->FontSize;

    fwrite(&cache, sizeof(FontCache), 1, fp);
    fwrite(pixels, cache.width * cache.height, 1, fp);
    fwrite(fonts->TexUvLines, sizeof(fonts->TexUvLines), 1, fp);
    fwrite(fonts->Fonts[0]->Glyphs.Data, sizeof(ImFontGlyph), cache.glyphs_size, fp);

    fclose(fp);
    return true;
}

static void gen_font_texture(ImFontAtlas *fonts)
{
    int width, height;
    uint32_t *pixels = NULL;
    fonts->GetTexDataAsRGBA32((uint8_t **)&pixels, &width, &height);

    gFontTexture = vita2d_create_empty_texture(width, height);
    const auto stride = vita2d_texture_get_stride(gFontTexture) / 4;
    uint32_t *texture_data = (uint32_t *)vita2d_texture_get_datap(gFontTexture);

    for (auto y = 0; y < height; ++y)
        for (auto x = 0; x < width; ++x)
            texture_data[y * stride + x] = pixels[y * width + x];

    fonts->TexID = gFontTexture;
}

static bool load_font_cache(const char *path)
{
    LogFunctionName;
    FILE *fp = fopen(path, "rb");
    if (!fp)
    {
        return false;
    }

    FontCache cache;
    fread(&cache, sizeof(FontCache), 1, fp);
    if (cache.version != FONT_CACHE_VERSION)
    {
        fclose(fp);
        return false;
    }

    ImFontAtlas *fonts = ImGui::GetIO().Fonts;
    ImFontConfig config{};
    config.FontData = IM_ALLOC(1);
    config.FontDataSize = 1;
    config.SizePixels = 1.f;

    ImFont *font = fonts->AddFont(&config);

    font->FontSize = cache.size;
    font->ConfigDataCount = 1;
    font->ContainerAtlas = fonts;
    font->ConfigData = &config;
    fonts->TexWidth = cache.width;
    fonts->TexHeight = cache.height;
    fonts->TexUvWhitePixel = cache.white_uv;

    size_t size = cache.width * cache.height;
    fonts->TexPixelsAlpha8 = (unsigned char *)IM_ALLOC(size);

    fread(fonts->TexPixelsAlpha8, size, 1, fp);
    fread(fonts->TexUvLines, sizeof(fonts->TexUvLines), 1, fp);

    for (size_t i = 0; i < cache.glyphs_size; i++)
    {
        ImFontGlyph glyph;
        fread(&glyph, sizeof(ImFontGlyph), 1, fp);
        font->AddGlyph(&config, glyph.Codepoint & 0xffff, glyph.X0, glyph.Y0, glyph.X1, glyph.Y1,
                       glyph.U0, glyph.V0, glyph.U1, glyph.V1, glyph.AdvanceX);
        font->SetGlyphVisible(glyph.Codepoint, glyph.Visible);
    }

    font->BuildLookupTable();
    gen_font_texture(fonts);
    fclose(fp);
    return true;
}

static const ImWchar *get_glyph_ranges(uint32_t language)
{
    switch (language)
    {
    case LANGUAGE_CHINESE:
        return GB_2312;
    // case LANGUAGE_JAPANESE:
    //     glyph_ranges = GetGlyphRangesJapanese();
    //     break;
    case LANGUAGE_ENGLISH:
    default:
        return ImGui::GetIO().Fonts->GetGlyphRangesDefault();
    }
}

static uint32_t get_glyph_ranges_crc32(uint32_t language)
{
    const ImWchar *glyph_ranges = get_glyph_ranges(language);
    const ImWchar *p = glyph_ranges;
    size_t size = 0;
    while (*p)
    {
        p++;
        size += sizeof(ImWchar);
    }

    return crc32(0, (uint8_t *)glyph_ranges, size);
}

void My_Imgui_Create_Font(uint32_t language, const char *cache_path)
{
    LogFunctionName;
    LogDebug("language: %d", language);

    char name[255];

    if (cache_path)
    {
        snprintf(name, 255, "%s/font_%08x.bin", cache_path, get_glyph_ranges_crc32(language));
        if (load_font_cache(name))
        {
            return;
        }
        else
        {
            LogDebug("failed to load cache");
        }
    }

    // Build texture atlas
    ImGuiIO &io = ImGui::GetIO();
    // Build and load the texture atlas into a texture
    uint32_t *pixels = NULL;
    int width, height;

    ImFontConfig font_config;
    font_config.OversampleH = 1;
    font_config.OversampleV = 1;
    font_config.PixelSnapH = 1;

    const ImWchar *glyph_ranges = get_glyph_ranges(language);

    io.Fonts->AddFontFromFileTTF(APP_ASSETS_DIR "/" TEXT_FONT_NAME,
                                 25.0f,
                                 &font_config,
                                 glyph_ranges);
    font_config.MergeMode = true;
    io.Fonts->AddFontFromFileTTF(APP_ASSETS_DIR "/" TEXT_FONT_NAME,
                                 25.0f,
                                 &font_config,
                                 RomanNumCharset);
    io.Fonts->AddFontFromFileTTF(APP_ASSETS_DIR "/" GAMEPAD_FONT_NAME,
                                 26.0f,
                                 &font_config,
                                 GamePadCharset);
    io.Fonts->AddFontFromFileTTF(APP_ASSETS_DIR "/" ICON_FONT_NAME,
                                 24.0f,
                                 &font_config,
                                 IconCharset);
    gen_font_texture(io.Fonts);

    if (cache_path)
    {
        save_font_cache(name);
    }
    return;
}

void My_Imgui_Destroy_Font()
{
    LogFunctionName;
    if (gFontTexture)
    {
        vita2d_wait_rendering_done();
        vita2d_free_texture(gFontTexture);
        gFontTexture = nullptr;
        ImGui::GetIO().Fonts->TexID = nullptr;
        ImGui::GetIO().Fonts->Clear();
    }
}

IMGUI_API void My_ImGui_ImplVita2D_Init(uint32_t language, const char *cache_path)
{
    LogFunctionName;
    My_Imgui_Create_Font(language, cache_path);

    // check the shaders
    sceGxmProgramCheck(&_binary_assets_imgui_v_cg_gxp_start);
    sceGxmProgramCheck(&_binary_assets_imgui_f_cg_gxp_start);

    // register programs with the patcher
    sceGxmShaderPatcherRegisterProgram(vita2d_get_shader_patcher(),
                                       &_binary_assets_imgui_v_cg_gxp_start,
                                       &imguiVertexProgramId);

    sceGxmShaderPatcherRegisterProgram(vita2d_get_shader_patcher(),
                                       &_binary_assets_imgui_f_cg_gxp_start,
                                       &imguiFragmentProgramId);

    // get attributes by name to create vertex format bindings
    const SceGxmProgramParameter *paramTexturePositionAttribute = sceGxmProgramFindParameterByName(&_binary_assets_imgui_v_cg_gxp_start, "aPosition");

    const SceGxmProgramParameter *paramTextureTexcoordAttribute = sceGxmProgramFindParameterByName(&_binary_assets_imgui_v_cg_gxp_start, "aTexcoord");

    const SceGxmProgramParameter *paramTextureColorAttribute = sceGxmProgramFindParameterByName(&_binary_assets_imgui_v_cg_gxp_start, "aColor");

    // create texture vertex format
    SceGxmVertexAttribute textureVertexAttributes[3];
    SceGxmVertexStream textureVertexStreams[1];
    /* x,y,z: 3 float 32 bits */
    textureVertexAttributes[0].streamIndex = 0;
    textureVertexAttributes[0].offset = 0;
    textureVertexAttributes[0].format = SCE_GXM_ATTRIBUTE_FORMAT_F32;
    textureVertexAttributes[0].componentCount = 2; // (x, y)
    textureVertexAttributes[0].regIndex = sceGxmProgramParameterGetResourceIndex(paramTexturePositionAttribute);

    /* u,v: 2 floats 32 bits */
    textureVertexAttributes[1].streamIndex = 0;
    textureVertexAttributes[1].offset = 8; // (x, y) * 4 = 12 bytes
    textureVertexAttributes[1].format = SCE_GXM_ATTRIBUTE_FORMAT_F32;
    textureVertexAttributes[1].componentCount = 2; // (u, v)
    textureVertexAttributes[1].regIndex = sceGxmProgramParameterGetResourceIndex(paramTextureTexcoordAttribute);

    /* r,g,b,a: 4 int 8 bits */
    textureVertexAttributes[2].streamIndex = 0;
    textureVertexAttributes[2].offset =
        16; // (x, y) * 4 + (u, v) * 4 = 20 bytes
    textureVertexAttributes[2].format = SCE_GXM_ATTRIBUTE_FORMAT_U8N;
    textureVertexAttributes[2].componentCount = 4; // (r, g, b, a)
    textureVertexAttributes[2].regIndex = sceGxmProgramParameterGetResourceIndex(paramTextureColorAttribute);

    // 16 bit (short) indices
    textureVertexStreams[0].stride = ImguiVertexSize;
    textureVertexStreams[0].indexSource = SCE_GXM_INDEX_SOURCE_INDEX_16BIT;

    // create texture shaders
    sceGxmShaderPatcherCreateVertexProgram(vita2d_get_shader_patcher(),
                                           imguiVertexProgramId,
                                           textureVertexAttributes,
                                           3,
                                           textureVertexStreams,
                                           1,
                                           &_vita2d_imguiVertexProgram);

    // Fill SceGxmBlendInfo
    static SceGxmBlendInfo blend_info{};
    blend_info.colorFunc = SCE_GXM_BLEND_FUNC_ADD;
    blend_info.alphaFunc = SCE_GXM_BLEND_FUNC_ADD;
    blend_info.colorSrc = SCE_GXM_BLEND_FACTOR_SRC_ALPHA;
    blend_info.colorDst = SCE_GXM_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blend_info.alphaSrc = SCE_GXM_BLEND_FACTOR_SRC_ALPHA;
    blend_info.alphaDst = SCE_GXM_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blend_info.colorMask = SCE_GXM_COLOR_MASK_ALL;

    sceGxmShaderPatcherCreateFragmentProgram(vita2d_get_shader_patcher(),
                                             imguiFragmentProgramId,
                                             SCE_GXM_OUTPUT_REGISTER_FORMAT_UCHAR4,
                                             SCE_GXM_MULTISAMPLE_NONE,
                                             &blend_info,
                                             &_binary_assets_imgui_v_cg_gxp_start,
                                             &_vita2d_imguiFragmentProgram);

    // find vertex uniforms by name and cache parameter information
    _vita2d_imguiWvpParam = sceGxmProgramFindParameterByName(&_binary_assets_imgui_v_cg_gxp_start, "wvp");

    matrix_init_orthographic(ortho_proj_matrix, 0.0f, VITA_WIDTH, VITA_HEIGHT, 0.0f, 0.0f, 1.0f);
}

IMGUI_API void My_ImGui_ImplVita2D_Shutdown()
{
    LogFunctionName;
    My_Imgui_Destroy_Font();
}

IMGUI_API void My_ImGui_ImplVita2D_RenderDrawData(ImDrawData *draw_data)
{
    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
    ImGuiIO &io = ImGui::GetIO();
    int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
    int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
    if (fb_width == 0 || fb_height == 0)
        return;
    draw_data->ScaleClipRects(io.DisplayFramebufferScale);

    auto _vita2d_context = vita2d_get_context();

    vita2d_enable_clipping();

    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const auto cmd_list = draw_data->CmdLists[n];
        const auto vtx_buffer = cmd_list->VtxBuffer.Data;
        const auto vtx_size = cmd_list->VtxBuffer.Size;
        const auto idx_buffer = cmd_list->IdxBuffer.Data;
        const auto idx_size = cmd_list->IdxBuffer.Size;

        const auto vertices = vita2d_pool_memalign(vtx_size * ImguiVertexSize, ImguiVertexSize);
        memcpy(vertices, vtx_buffer, vtx_size * ImguiVertexSize);

        static_assert(sizeof(ImDrawIdx) == 2);
        auto indices = (uint16_t *)vita2d_pool_memalign(idx_size * sizeof(ImDrawIdx), sizeof(void *));
        memcpy(indices, idx_buffer, idx_size * sizeof(ImDrawIdx));

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const auto pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback)
            {
                pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                vita2d_set_clip_rectangle((int)pcmd->ClipRect.x, (int)pcmd->ClipRect.y, (int)pcmd->ClipRect.z, (int)pcmd->ClipRect.w);

                sceGxmSetVertexProgram(_vita2d_context, _vita2d_imguiVertexProgram);
                sceGxmSetFragmentProgram(_vita2d_context, _vita2d_imguiFragmentProgram);

                sceGxmSetVertexStream(_vita2d_context, 0, vertices);

                void *vertex_wvp_buffer;
                sceGxmReserveVertexDefaultUniformBuffer(vita2d_get_context(), &vertex_wvp_buffer);
                sceGxmSetUniformDataF(vertex_wvp_buffer, _vita2d_imguiWvpParam, 0, 16, ortho_proj_matrix);

                const auto texture = (vita2d_texture *)pcmd->TextureId;
                sceGxmSetFragmentTexture(_vita2d_context, 0, &texture->gxm_tex);

                sceGxmDraw(_vita2d_context,
                           SCE_GXM_PRIMITIVE_TRIANGLES,
                           SCE_GXM_INDEX_FORMAT_U16,
                           indices,
                           pcmd->ElemCount);
            }

            indices += pcmd->ElemCount;
        }
    }

    vita2d_disable_clipping();
}

static float CalcMaxPopupHeightFromItemCount(int items_count)
{
    ImGuiContext &g = *GImGui;
    if (items_count <= 0)
        return FLT_MAX;
    return (g.FontSize + g.Style.ItemSpacing.y) * items_count - g.Style.ItemSpacing.y + (g.Style.WindowPadding.y * 2);
}

bool My_Imgui_BeginCombo(const char *label, const char *preview_value, ImGuiComboFlags flags)
{
    // Always consume the SetNextWindowSizeConstraint() call in our early return paths
    ImGuiContext *g = ImGui::GetCurrentContext();
    bool has_window_size_constraint = (g->NextWindowData.Flags & ImGuiNextWindowDataFlags_HasSizeConstraint) != 0;
    g->NextWindowData.Flags &= ~ImGuiNextWindowDataFlags_HasSizeConstraint;

    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    IM_ASSERT((flags & (ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_NoPreview)) != (ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_NoPreview)); // Can't use both flags together

    const ImGuiStyle &style = g->Style;
    const ImGuiID id = window->GetID(label);

    const float arrow_size = (flags & ImGuiComboFlags_NoArrowButton) ? 0.0f : ImGui::GetFrameHeight();
    const ImVec2 label_size = ImGui::CalcTextSize("", NULL, true);
    const float expected_w = ImGui::CalcItemWidth();
    const float w = (flags & ImGuiComboFlags_NoPreview) ? arrow_size : expected_w;
    const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(w, label_size.y + style.FramePadding.y * 2.0f));
    const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));
    ImGui::ItemSize(total_bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(total_bb, id, &frame_bb))
        return false;

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(frame_bb, id, &hovered, &held);
    bool popup_open = ImGui::IsPopupOpen(id, ImGuiPopupFlags_None);

    const ImU32 frame_col = ImGui::GetColorU32(hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
    const float value_x2 = ImMax(frame_bb.Min.x, frame_bb.Max.x - arrow_size);
    ImGui::RenderNavHighlight(frame_bb, id);
    if (!(flags & ImGuiComboFlags_NoPreview))
        window->DrawList->AddRectFilled(frame_bb.Min, ImVec2(value_x2, frame_bb.Max.y), frame_col, style.FrameRounding, (flags & ImGuiComboFlags_NoArrowButton) ? ImDrawCornerFlags_All : ImDrawCornerFlags_Left);
    if (!(flags & ImGuiComboFlags_NoArrowButton))
    {
        ImU32 bg_col = ImGui::GetColorU32((popup_open || hovered) ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
        ImU32 text_col = ImGui::GetColorU32(ImGuiCol_Text);
        window->DrawList->AddRectFilled(ImVec2(value_x2, frame_bb.Min.y), frame_bb.Max, bg_col, style.FrameRounding, (w <= arrow_size) ? ImDrawCornerFlags_All : ImDrawCornerFlags_Right);
        if (value_x2 + arrow_size - style.FramePadding.x <= frame_bb.Max.x)
            ImGui::RenderArrow(window->DrawList, ImVec2(value_x2 + style.FramePadding.y, frame_bb.Min.y + style.FramePadding.y), text_col, ImGuiDir_Down, 1.0f);
    }
    ImGui::RenderFrameBorder(frame_bb.Min, frame_bb.Max, style.FrameRounding);
    if (preview_value != NULL && !(flags & ImGuiComboFlags_NoPreview))
        ImGui::RenderTextClipped(frame_bb.Min + style.FramePadding, ImVec2(value_x2, frame_bb.Max.y), preview_value, NULL, NULL, ImVec2(0.0f, 0.0f));
    if (label_size.x > 0)
        ImGui::RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), label);

    if ((pressed || g->NavActivateId == id) && !popup_open)
    {
        if (window->DC.NavLayerCurrent == 0)
            window->NavLastIds[0] = id;
        ImGui::OpenPopupEx(id, ImGuiPopupFlags_None);
        popup_open = true;
    }

    if (!popup_open)
        return false;

    if (has_window_size_constraint)
    {
        g->NextWindowData.Flags |= ImGuiNextWindowDataFlags_HasSizeConstraint;
        g->NextWindowData.SizeConstraintRect.Min.x = ImMax(g->NextWindowData.SizeConstraintRect.Min.x, w);
    }
    else
    {
        if ((flags & ImGuiComboFlags_HeightMask_) == 0)
            flags |= ImGuiComboFlags_HeightRegular;
        IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiComboFlags_HeightMask_)); // Only one
        int popup_max_height_in_items = -1;
        if (flags & ImGuiComboFlags_HeightRegular)
            popup_max_height_in_items = 8;
        else if (flags & ImGuiComboFlags_HeightSmall)
            popup_max_height_in_items = 4;
        else if (flags & ImGuiComboFlags_HeightLarge)
            popup_max_height_in_items = 20;
        ImGui::SetNextWindowSizeConstraints(ImVec2(w, 0.0f), ImVec2(FLT_MAX, CalcMaxPopupHeightFromItemCount(popup_max_height_in_items)));
    }

    char name[16];
    ImFormatString(name, IM_ARRAYSIZE(name), "##Combo_%02d", g->BeginPopupStack.Size); // Recycle windows based on depth

    // Peak into expected window size so we can position it
    if (ImGuiWindow *popup_window = ImGui::FindWindowByName(name))
        if (popup_window->WasActive)
        {
            ImVec2 size_expected = ImGui::CalcWindowExpectedSize(popup_window);
            if (flags & ImGuiComboFlags_PopupAlignLeft)
                popup_window->AutoPosLastDirection = ImGuiDir_Left;
            ImRect r_outer = ImGui::GetWindowAllowedExtentRect(popup_window);
            ImVec2 pos = ImGui::FindBestWindowPosForPopupEx(frame_bb.GetBL(), size_expected, &popup_window->AutoPosLastDirection, r_outer, frame_bb, ImGuiPopupPositionPolicy_ComboBox);
            ImGui::SetNextWindowPos(pos);
        }

    // We don't use BeginPopupEx() solely because we have a custom name string, which we could make an argument to BeginPopupEx()
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_Popup | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove;

    // Horizontally align ourselves with the framed text
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(style.FramePadding.x, style.WindowPadding.y));
    bool ret = ImGui::Begin(name, NULL, window_flags);
    ImGui::PopStyleVar();
    if (!ret)
    {
        ImGui::EndPopup();
        IM_ASSERT(0); // This should never happen as we tested for IsPopupOpen() above
        return false;
    }
    return true;
}

IMGUI_API void My_Imgui_CenteredText(const char *text, ...)
{
    char buf[0x100];

    va_list args;
    va_start(args, text);
    vsnprintf(buf, 0x100, text, args);
    va_end(args);

    float win_width = ImGui::GetWindowSize().x;
    float text_width = ImGui::CalcTextSize(buf).x;

    ImGui::SetCursorPosX((win_width - text_width) * 0.5);
    ImGui::Text(buf);
}