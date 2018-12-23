///////////////////////////////////////////////////////////
// Shade Slider: A color picking matching game
//  Goal: Move the picker to match the reference image
//
// BSD-3-Clause LICENSE
// Zach Mertens-McConnell @github/zmertens
///////////////////////////////////////////////////////////

#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include <GLES3/gl3.h>

#include <imgui.h>

#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>

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
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s",
            "This device does not support haptic feedback\n");
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
        current.w, current.h, 
        SDL_WINDOW_OPENGL);
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

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    static bool drag_and_drop = false;
    static bool options_menu = false;
    static bool alpha_half_preview = false;
    static bool alpha_preview = true;
    static bool hdr = false;
    static ImVec4 color_to_match = ImColor(114, 144, 154, 200);

    int misc_flags = (hdr ? ImGuiColorEditFlags_HDR : 0) \
        | (drag_and_drop ? 0 : ImGuiColorEditFlags_NoDragDrop) \
        | (alpha_half_preview ? ImGuiColorEditFlags_AlphaPreviewHalf : (alpha_preview ? ImGuiColorEditFlags_AlphaPreview : 0)) \
        | (options_menu ? 0 : ImGuiColorEditFlags_NoOptions);


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
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            static float f = 0.0f;
            static int counter = 0;
            
            ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.05, io.DisplaySize.y * 0.05), ImGuiSetCond_Once);
            
            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.
    
            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &hdr);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &hdr);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f    
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            if (counter == 1) {
                if (SDL_HapticRumblePlay(haptic, 0.75, 500) != 0)
                    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s", SDL_GetError());
            }

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // the color to match to
        // ImGui::ColorButton("MyColor##3c", *(ImVec4*)&color_to_match, misc_flags, ImVec2(80,80));

    //  ImGuiColorEditFlags flags = misc_flags;
    //     if (!alpha) flags |= ImGuiColorEditFlags_NoAlpha; // This is by default if you call ColorPicker3() instead of ColorPicker4()
    //     if (alpha_bar) flags |= ImGuiColorEditFlags_AlphaBar;
    //     if (!side_preview) flags |= ImGuiColorEditFlags_NoSidePreview;
    //     if (picker_mode == 1) flags |= ImGuiColorEditFlags_PickerHueBar;
    //     if (picker_mode == 2) flags |= ImGuiColorEditFlags_PickerHueWheel;
    //     if (inputs_mode == 1) flags |= ImGuiColorEditFlags_NoInputs;
    //     if (inputs_mode == 2) flags |= ImGuiColorEditFlags_RGB;
    //     if (inputs_mode == 3) flags |= ImGuiColorEditFlags_HSV;
    //     if (inputs_mode == 4) flags |= ImGuiColorEditFlags_HEX;
    //     ImGui::ColorPicker4("MyColor##4", (float*)&color, flags, ref_color ? &ref_color_v.x : NULL);

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