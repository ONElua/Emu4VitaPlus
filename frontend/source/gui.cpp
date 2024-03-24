
#include "gui.h"
#include "log.h"

Gui::Gui() : _running(true)
{
    LogFunctionName;

    ImGui::CreateContext();
    ImGui_ImplVita2D_Init();

    ImGui_ImplVita2D_TouchUsage(false);
    ImGui_ImplVita2D_UseIndirectFrontTouch(false);
    ImGui_ImplVita2D_UseRearTouch(false);
    ImGui_ImplVita2D_GamepadUsage(false);
    // ImGuiIO &io = ImGui::GetIO();
    // io.Fonts->AddFontFromFileTTF("sa0:/data/font/pvf/jpn0.pvf",
    //                              20.0f,
    //                              0,
    //                              io.Fonts->GetGlyphRangesJapanese());

    _input.SetCallback(SCE_CTRL_CROSS, std::bind(&Gui::_OnCrossPress, this));

    LogDebug("Gui init end");
}

void Gui::_OnCrossPress()
{
    LogFunctionName;
    _running = false;
}

Gui::~Gui()
{
    LogFunctionName;
    ImGui_ImplVita2D_Shutdown();
    ImGui::DestroyContext();
}

bool Gui::Run()
{
    LogFunctionNameLimited;

    _input.Run();

    ImGui_ImplVita2D_NewFrame();
    ImGui::SetMouseCursor(ImGuiMouseCursor_None);

    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;

    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize({960, 544});
    ImGui::Begin("Hello",
                 NULL,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoDecoration);
    if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
    {
        if (ImGui::BeginTabItem("Avocado"))
        {
            ImGui::Text("This is the Avocado tab!\nblah blah blah blah blah");
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Broccoli"))
        {
            ImGui::Text("This is the Broccoli tab!\nblah blah blah blah blah");
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Cucumber"))
        {
            ImGui::Text("This is the Cucumber tab!\nblah blah blah blah blah");
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();

    ImGui::Render();
    ImGui_ImplVita2D_RenderDrawData(ImGui::GetDrawData());

    return _running;
}