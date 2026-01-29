// Dear ImGui: standalone example application for SDL2 + SDL_Renderer
// (SDL is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)

// Important to understand: SDL_Renderer is an _optional_ component of SDL2.
// For a multi-platform app consider using e.g. SDL+DirectX on Windows and SDL+OpenGL on Linux/OSX.

#define SDL_MAIN_HANDLED 
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#include <cstdio>
#include <cmath>
#include <vector>
#include <SDL.h>

// Windows specific includes for debugging popups and console allocation
#if defined(_WIN32)
#include <windows.h>
#endif

#if !SDL_VERSION_ATLEAST(2,0,17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

// Helper function to show a message box on error
void ShowError(const char* title, const char* message)
{
    #if defined(_WIN32)
    MessageBoxA(NULL, message, title, MB_ICONERROR);
    #else
    fprintf(stderr, "%s: %s\n", title, message);
    #endif
}

// Main code
int main(int, char**)
{
    // --- DEBUG SETUP ---
    // Force open a console window so we can see printf output if we run from the .exe directly
    #if defined(_WIN32)
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
    printf("Debug console attached.\n");
    #endif

    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        char errorMsg[256];
        snprintf(errorMsg, sizeof(errorMsg), "SDL_Init failed: %s", SDL_GetError());
        ShowError("SDL Error", errorMsg);
        return -1;
    }

    // From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    // Create window with SDL_Renderer graphics context
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("Fourier", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    
    // FIX: Check if window creation failed
    if (window == nullptr)
    {
        char errorMsg[256];
        snprintf(errorMsg, sizeof(errorMsg), "SDL_CreateWindow failed: %s", SDL_GetError());
        ShowError("Window Error", errorMsg);
        SDL_Quit();
        return -1;
    }

    // Try to create a hardware accelerated renderer first
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    
    // FIX: If hardware renderer fails, try software renderer (useful for VMs or bad drivers)
    if (renderer == nullptr)
    {
        printf("Hardware renderer failed (%s), trying software renderer...\n", SDL_GetError());
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    }

    if (renderer == nullptr)
    {
        char errorMsg[256];
        snprintf(errorMsg, sizeof(errorMsg), "SDL_CreateRenderer failed: %s", SDL_GetError());
        ShowError("Renderer Error", errorMsg);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    SDL_RendererInfo info;
    SDL_GetRendererInfo(renderer, &info);
    printf("Current SDL_Renderer: %s\n", info.name);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    bool show_circle_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle events (inputs, window resize, etc.)
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        // Start the Dear ImGui frame
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);
            ImGui::Checkbox("Circle Window", &show_circle_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        if (show_circle_window)
        {
            ImGui::Begin("Circle Window", &show_circle_window);
            
            static float scale = 1.0f;
            ImGui::SliderFloat("Scale", &scale, 0.5f, 2.0f);

            // Get current draw list
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            
            // Parameters
            ImVec2 center = ImGui::GetCursorScreenPos();
            center.x += 100 * scale; // offset from cursor
            center.y += 150 * scale;
            float r1 = 60.0f * scale;
            float r2 = 30.0f * scale;
            int segments = 64; // Higher = smoother

            double time = ImGui::GetTime();

            // Circle 1 (Large)
            float angle1 = (float)(-time * 1.0);
            ImVec2 p1 = ImVec2(center.x + r1 * cosf(angle1), center.y + r1 * sinf(angle1));
            
            draw_list->AddCircle(center, r1, IM_COL32(255, 255, 255, 100), segments);
            draw_list->AddLine(center, p1, IM_COL32(255, 255, 255, 100));

            // Circle 2 (Small, circling the larger one)
            float angle2 = (float)(-time * 3.0);
            ImVec2 p2 = ImVec2(p1.x + r2 * cosf(angle2), p1.y + r2 * sinf(angle2));

            draw_list->AddCircle(p1, r2, IM_COL32(255, 255, 0, 255), segments);
            draw_list->AddLine(p1, p2, IM_COL32(255, 255, 0, 255));

            // Tangent on smaller circle at p2
            ImVec2 radius_vec = ImVec2(p2.x - p1.x, p2.y - p1.y);
            ImVec2 tangent_vec = ImVec2(-radius_vec.y, radius_vec.x); // Perpendicular
            float len = sqrtf(tangent_vec.x * tangent_vec.x + tangent_vec.y * tangent_vec.y);
            if (len > 0) { tangent_vec.x /= len; tangent_vec.y /= len; }
            
            float tangent_len = 50.0f * scale;
            ImVec2 t1 = ImVec2(p2.x - tangent_vec.x * tangent_len, p2.y - tangent_vec.y * tangent_len);
            ImVec2 t2 = ImVec2(p2.x + tangent_vec.x * tangent_len, p2.y + tangent_vec.y * tangent_len);
            
            draw_list->AddLine(t1, t2, IM_COL32(0, 255, 255, 255), 2.0f);
            draw_list->AddCircleFilled(p2, 4.0f * scale, IM_COL32(255, 0, 0, 255));

            // Graph
            static std::vector<float> wave_data;
            if (wave_data.size() > 500)
                wave_data.erase(wave_data.begin());
            wave_data.push_back(p2.y);

            float graph_x_start = center.x + r1 + r2 + 50.0f * scale;
            draw_list->AddLine(p2, ImVec2(graph_x_start, p2.y), IM_COL32(255, 255, 255, 50));

            if (wave_data.size() > 1)
            {
                for (size_t i = 0; i < wave_data.size() - 1; i++)
                {
                    float x1 = graph_x_start + (wave_data.size() - 1 - i);
                    float y1 = wave_data[i];
                    float x2 = graph_x_start + (wave_data.size() - 1 - (i + 1));
                    float y2 = wave_data[i + 1];
                    draw_list->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), IM_COL32(255, 0, 0, 255), 1.5f);
                }
            }

            ImGui::Dummy(ImVec2(700 * scale, 300 * scale));
            ImGui::Text("Epicycles with Tangent and Real-time Graph");
            if (ImGui::Button("Close Me"))
                show_circle_window = false;
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        SDL_SetRenderDrawColor(renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
        SDL_RenderPresent(renderer);
    }

    // Cleanup
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}