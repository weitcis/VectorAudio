#define CPPHTTPLIB_OPENSSL_SUPPORT

#include <cstdio>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <memory>
#include <thread>

#include "application.h"
#include "config.h"
#include "data_file_handler.h"
#include "imgui-SFML.h"
#include "imgui.h"
#include "shared.h"
#include "spdlog/spdlog.h"
#include "style.h"
#include "updater.h"

// Main code
int main(int, char**)
{
    vector_audio::configuration::build_logger();
    sf::RenderWindow window(sf::VideoMode(800, 450), "Vector Audio");
    window.setFramerateLimit(30);

    auto image = sf::Image {};

#ifdef SFML_SYSTEM_WINDOWS
    std::string icon_name = "icon_win.png";
#else
    std::string icon_name = "icon_mac.png";
#endif

    if (!image.loadFromFile(vector_audio::configuration::get_resource_folder() + icon_name)) {
        spdlog::error("Could not load application icon");
    } else {
        window.setIcon(image.getSize().x, image.getSize().y, image.getPixelsPtr());
    }

    ImGui::SFML::Init(window, false);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable
    // Keyboard Controls io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; //
    // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can
    // also load multiple fonts and use ImGui::PushFont()/PopFont() to select
    // them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you
    // need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please
    // handle those errors in your application (e.g. use an assertion, or display
    // an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored
    // into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which
    // ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string
    // literal you need to write a double backslash \\ !
    // io.Fonts->AddFontDefault();
    io.Fonts->AddFontFromFileTTF(
        (vector_audio::configuration::get_resource_folder() + "JetBrainsMono-Regular.ttf")
            .c_str(),
        18.0);

    ImGui::SFML::UpdateFontTexture();

    // Our state

    vector_audio::style::apply_style();
    vector_audio::configuration::build_config();

    spdlog::info("Starting Vector Audio...");

    auto updater_instance = std::make_unique<vector_audio::updater>();

    auto data_file_handler = std::make_unique<vector_audio::data_file::Handler>();

    auto current_app = std::make_unique<vector_audio::application::App>();

    // Main loop
    sf::Clock delta_clock;
    while (window.isOpen()) {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to
        // tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to
        // your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input
        // data to your main application. Generally you may always pass all inputs
        // to dear imgui, and hide them from your application based on those two
        // flags.
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(window, event);

            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::KeyPressed) {
                // Capture the new Ptt key
                if (vector_audio::shared::capture_ptt_flag) {
                    vector_audio::shared::ptt = event.key.code;

                    vector_audio::configuration::config["user"]["ptt"] = static_cast<int>(vector_audio::shared::ptt);
                    vector_audio::configuration::write_config_async();
                    vector_audio::shared::capture_ptt_flag = false;
                }
            }
        }

        ImGui::SFML::Update(window, delta_clock.restart());

        if (!updater_instance->need_update())
            current_app->render_frame();
        else
            updater_instance->draw();

        //ImGui::ShowDemoWindow(NULL);

        // Rendering
        window.clear();
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();

    return 0;
}