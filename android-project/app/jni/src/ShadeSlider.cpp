///////////////////////////////////////////////////////////
// Shade Slider: A color picking matching game
//  Goal: Move the picker to match the reference image
//
// BSD-3-Clause LICENSE
// Zach Mertens-McConnell @github/zmertens
//
//
// =======================================================
// | (?)                                       |         |
// |                                           |         |
// |                                           |         |
// |                                           |_________|
// |                                                 |   |
// |                                                 |   |
// |                                                 |   |
// |                                                 |   |
// |                                                 |   |
// |                                                 |   |
// |                                                 |   |
// |                                                 |   |
// |                                                 |   |
// |                                                 |   |
// |                                                 |   |
// |                                                 |   |
// |                                                 |   |
// |                                                 |   |
// |                                                 |   |
// |                                                 |   |
// |                                                 |   |
// |                                                 |   |
// | ________________________________________________|___|
// |                 |                |                  |
// |                 |                |                  |
///////////////////////////////////////////////////////////

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <random>

#include <SDL.h>

#include <GLES3/gl3.h>

#include <imgui.h>

#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>

float get_random_float(int low, int high)
{
    // static const int seed = 0; // time(0)
    std::random_device rd;
    std::mt19937_64 mt (rd());
    std::uniform_real_distribution<float> dist (low, high);
    return dist (mt);
}

// Taken from `imgui_demo.cpp`
static void ShowHelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 145.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

// Check if two color vectors (r, g, b) values are within range, ignore alpha
// color values are stored [0, 1], so we scale the range by 255 before calling function
bool colors_are_within_range(const float r1, const float r2,
    const float g1, const float g2,
    const float b1, const float b2, const float range)
{
    if (std::abs(r2 - r1) <= range && std::abs(g2 - g1) <= range && std::abs(b2 - b1) <= range)
        return true;
    else
        return false;
}

// Must use conventional parameters here 
// or else there will be an undefined reference to SDL_main
int main(int argc, char** argv)
{
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC) != 0)
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Error: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Haptic *haptic = SDL_HapticOpen(0);
    if (!haptic)
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s%s",
            "This device does not support haptic feedback\n", SDL_GetError());
    }
    else
    {
        if (SDL_HapticRumbleInit(haptic) < 0)
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s%s",
                "SDL haptic did not initialize the rumble\n", SDL_GetError());
        }
    }

    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);

    // Setup window
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_DisplayMode current;
    SDL_GetCurrentDisplayMode(0, &current);
    SDL_Window* window = SDL_CreateWindow(nullptr,
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        0, 0, 
        SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN | SDL_WINDOW_ALLOW_HIGHDPI);
    if (window == 0)
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create the SDL Window: %s\n", SDL_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Query OpenGL device information
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* vendor = glGetString(GL_VENDOR);
    const GLubyte* version = glGetString(GL_VERSION);
    const GLubyte* glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);
    const char * new_line = "\n-------------------------------------------------------------\n";
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "%s%s%s%s%s%s%s%s%s%s", 
        new_line,
        "GL Vendor : ", vendor,
        "\nGL GLRenderer : ", renderer,
        "\nGL Version : ",  version,
        "\nGLSL Version : ", glslVersion,
        new_line);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    // Android issue: Without NoMouseCursorChange config enabled SDL_SetCursor is called
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;// | ImGuiConfigFlags_NavEnableKeyboard;

    // Setup Platform/Renderer bindings
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init();

    // Setup Style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    static bool window_close_widget = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    // Pick colors that won't trigger the haptic
    // haptic triggers when user the user is within 1-hundreth of number for all x, y, z values
    static ImVec4 color_to_match = ImColor(0.655f, 0.245f, 0.100f, 1.f);
    static ImVec4 color_to_pick = ImColor(0.5f, 0.5f, 0.5f, 1.f);
    static ImVec4 ref_color_v (1.0f, 0.0f, 1.0f, 0.5f);

    int window_flags = ImGuiWindowFlags_NoMove \
        | ImGuiWindowFlags_NoDecoration \
        | ImGuiWindowFlags_NoNav \
        // don't save settings in .ini file
        | ImGuiWindowFlags_NoSavedSettings \
        | ImGuiWindowFlags_NoBackground;

    int color_button_flags = ImGuiColorEditFlags_NoAlpha \
        | ImGuiColorEditFlags_NoPicker \
        | ImGuiColorEditFlags_NoOptions \ 
        | ImGuiColorEditFlags_NoSmallPreview \
        | ImGuiColorEditFlags_NoInputs \
        | ImGuiColorEditFlags_NoTooltip \ 
        | ImGuiColorEditFlags_NoSidePreview \
        | ImGuiColorEditFlags_NoDragDrop;
        // ImGuiColorEditFlags_HDR \
        // | ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_AlphaPreview \

    // These flags are separated because they can't all be used at once
    // See imgui_widgets.cpp:4100 `ImIsPowerOfTwo` assertion
    ImGuiColorEditFlags color_picker_flags;
    // This is by default if you call ColorPicker3() instead of ColorPicker4()
    color_picker_flags |= ImGuiColorEditFlags_NoAlpha;
    // flags |= ImGuiColorEditFlags_AlphaBar;
    color_picker_flags |= ImGuiColorEditFlags_NoLabel;

    color_picker_flags |= ImGuiColorEditFlags_NoSidePreview;
    
    // color_picker_flags |= ImGuiColorEditFlags_PickerHueBar;
    color_picker_flags |= ImGuiColorEditFlags_PickerHueWheel;

    color_picker_flags |= ImGuiColorEditFlags_NoInputs;
    // color_picker_flags |= ImGuiColorEditFlags_RGB;
    // color_picker_flags |= ImGuiColorEditFlags_HSV;
    // color_picker_flags |= ImGuiColorEditFlags_HEX;

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);

            if (event.type == SDL_QUIT)
                done = true;
            else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
            {
                done = true;
                SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "\n\nSDL Window is closing!!\n\n");
            }
            else if (event.type == SDL_WINDOWEVENT_RESIZED && event.window.windowID == SDL_GetWindowID(window))
            {
                SDL_GetCurrentDisplayMode(0, &current);
                io.DisplaySize.x = current.w; io.DisplaySize.y = current.h;
                SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
                    "Resize Event:\ncurrent.w: %d, current.h: %d", current.w, current.h);
            }
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        // the game windows
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiSetCond_Always);
        ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y), ImGuiSetCond_Always);
        ImGui::Begin("Hello, world!", &window_close_widget, window_flags);                          // Create a window called "Hello, world!" and append into it.
        // ShowHelpMarker("Use the color picker to make the box on the right look like the box on the left");
        // ImGui::SameLine();
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 
            1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        // the color to match to
        ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.98f);
        ImGui::ColorButton("MyColor##3c", *(ImVec4*)&color_to_match, 
            color_button_flags, ImVec2(ImGui::GetWindowWidth() * 0.50f, 128));
        ImGui::SameLine();
        // the current color getting picked
        ImGui::ColorButton("MyColor##4c", *(ImVec4*)&color_to_pick, 
            color_button_flags, ImVec2(ImGui::GetWindowWidth() * 0.50f, 128));
        // ImGui::SameLine();
        ImGui::ColorPicker4("MyColor##4", (float*)&color_to_pick, color_picker_flags, &ref_color_v.x);
        ImGui::PopItemWidth();
        
        ImGui::Text("Make the phone vibrate!"); 
        ImGui::Text("Slide the color wheel around and get the colors to match");
        
        ImGui::End();

        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
            "\ncolor_to_match.x: %f\ncolor_to_match.y: %f\ncolor_to_match.z: %f \\
            \ncolor_to_pick.x: %f\ncolor_to_pick.y: %f\ncolor_to_pick.z: %f\n",
            color_to_match.x * 255.f, color_to_match.y * 255.f, color_to_match.z * 255.f,
            color_to_pick.x * 255.f, color_to_pick.y * 255.f, color_to_pick.z * 255.f);

        // game logic
        static float range_to_activate_haptic = 25.0f;
        static bool create_new_color = false;
        if (colors_are_within_range(color_to_match.x * 255.f, color_to_pick.x * 255.f,
            color_to_match.y * 255.f, color_to_pick.y * 255.f,
            color_to_match.z * 255.f, color_to_pick.z * 255.f, range_to_activate_haptic))
        {
            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "\n\n\nCOLORS ARE WITHIN RANGE\n\n\n\n");  
            create_new_color = !create_new_color;  
        }

        // activate haptic, reset colors, disable touch during reset
        if (create_new_color)
        {
            create_new_color = !create_new_color;
            // create new colors
            color_to_match.x = get_random_float(0, 1);
            color_to_match.y = get_random_float(0, 1);
            color_to_match.x = get_random_float(0, 1);
            color_to_pick.x = get_random_float(0, 1);
            color_to_pick.y = get_random_float(0, 1);
            color_to_pick.z = get_random_float(0, 1);
            if (SDL_HapticRumblePlay(haptic, 0.69, 2000) != 0)
                SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Haptic Error: %s", SDL_GetError()); 
        }

        clear_color.x = std::sin(static_cast<float>(SDL_GetTicks()) * 0.0001f);
        clear_color.y = std::cos(static_cast<float>(SDL_GetTicks()) * 0.0001f);
        clear_color.z = std::sin(static_cast<float>(SDL_GetTicks()) * 0.0001f);

        // SDL_GetCurrentDisplayMode(0, &current);
        // io.DisplaySize.x = current.w; io.DisplaySize.y = current.h;
        // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
        //     "current.w: %d,\tcurrent.h: %d", current.w, current.h);

        glViewport(0, 0, (int) io.DisplaySize.x, (int) io.DisplaySize.y);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        // Rendering
        ImGui::Render();
        
        //glUseProgram(0); // You may want this if using this code in an OpenGL 3+ context where shaders may be bound
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_HapticClose(haptic);
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
} // main