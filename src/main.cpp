// Dear ImGui: standalone example application for SDL2 + Vulkan
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

// Important note to the reader who wish to integrate imgui_impl_vulkan.cpp/.h in their own engine/app.
// - Common ImGui_ImplVulkan_XXX functions and structures are used to interface with imgui_impl_vulkan.cpp/.h.
//   You will use those if you want to use this rendering backend in your engine/app.
// - Helper ImGui_ImplVulkanH_XXX functions and structures are only used by this example (main.cpp) and by
//   the backend itself (imgui_impl_vulkan.cpp), but should PROBABLY NOT be used by your own engine/app code.
// Read comments in imgui_impl_vulkan.h.

#include "configuration.h"

#include <stdio.h>          // printf, fprintf
#include <stdlib.h>         // abort
#include "renderer/renderer.h"

//#define IMGUI_UNLIMITED_FRAME_RATE




void SetupDearImGui()
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();
}

void SetupBackends()
{
    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForVulkan(Renderer::window);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = Renderer::g_Instance;
    init_info.PhysicalDevice = Renderer::g_PhysicalDevice;
    init_info.Device = Renderer::g_Device;
    init_info.QueueFamily = Renderer::g_QueueFamily;
    init_info.Queue = Renderer::g_Queue;
    init_info.PipelineCache = Renderer::g_PipelineCache;
    init_info.DescriptorPool = Renderer::g_DescriptorPool;
    init_info.Subpass = 0;
    init_info.MinImageCount = Renderer::g_MinImageCount;
    init_info.ImageCount = Renderer::wd->ImageCount;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = Renderer::g_Allocator;
    init_info.CheckVkResultFn = Renderer::CheckVkResult;
    ImGui_ImplVulkan_Init(&init_info, Renderer::wd->RenderPass);
}

void LoadFonts()
{
    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    // Upload Fonts
    {
        // Use any command queue
        VkCommandPool command_pool = Renderer::wd->Frames[Renderer::wd->FrameIndex].CommandPool;
        VkCommandBuffer command_buffer = Renderer::wd->Frames[Renderer::wd->FrameIndex].CommandBuffer;

        Renderer::err = vkResetCommandPool(Renderer::g_Device, command_pool, 0);
        Renderer::CheckVkResult(Renderer::err);
        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        Renderer::err = vkBeginCommandBuffer(command_buffer, &begin_info);
        Renderer::CheckVkResult(Renderer::err);

        ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

        VkSubmitInfo end_info = {};
        end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        end_info.commandBufferCount = 1;
        end_info.pCommandBuffers = &command_buffer;
        Renderer::err = vkEndCommandBuffer(command_buffer);
        Renderer::CheckVkResult(Renderer::err);
        Renderer::err = vkQueueSubmit(Renderer::g_Queue, 1, &end_info, VK_NULL_HANDLE);
        Renderer::CheckVkResult(Renderer::err);

        Renderer::err = vkDeviceWaitIdle(Renderer::g_Device);
        Renderer::CheckVkResult(Renderer::err);
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }
}

int main(int, char**)
{
    Renderer::SetupSDL();
    Renderer::SetupWindow();
    Renderer::SetupVulkan();
    Renderer::CreateWindowSurface();
    Renderer::CreateFrameBuffers();
    SetupDearImGui();
    SetupBackends();
    LoadFonts();

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(Renderer::window))
                done = true;
        }

        // Resize swap chain?
        if (Renderer::g_SwapChainRebuild)
        {
            int width, height;
            SDL_GetWindowSize(Renderer::window, &width, &height);
            if (width > 0 && height > 0)
            {
                ImGui_ImplVulkan_SetMinImageCount(Renderer::g_MinImageCount);
                ImGui_ImplVulkanH_CreateOrResizeWindow(Renderer::g_Instance, Renderer::g_PhysicalDevice, Renderer::g_Device, &Renderer::g_MainWindowData, Renderer::g_QueueFamily, Renderer::g_Allocator, width, height, Renderer::g_MinImageCount);
                Renderer::g_MainWindowData.FrameIndex = 0;
                Renderer::g_SwapChainRebuild = false;
            }
        }

        // Start the Dear ImGui frame
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
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

        // Rendering
        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();
        const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
        if (!is_minimized)
        {
            Renderer::wd->ClearValue.color.float32[0] = clear_color.x * clear_color.w;
            Renderer::wd->ClearValue.color.float32[1] = clear_color.y * clear_color.w;
            Renderer::wd->ClearValue.color.float32[2] = clear_color.z * clear_color.w;
            Renderer::wd->ClearValue.color.float32[3] = clear_color.w;
            Renderer::FrameRender(Renderer::wd, draw_data);
            Renderer::FramePresent(Renderer::wd);
        }
    }

    // Cleanup
    Renderer::err = vkDeviceWaitIdle(Renderer::g_Device);
    Renderer::CheckVkResult(Renderer::err);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    Renderer::CleanupVulkanWindow();
    Renderer::CleanupVulkan();

    SDL_DestroyWindow(Renderer::window);
    SDL_Quit();

    return 0;
}