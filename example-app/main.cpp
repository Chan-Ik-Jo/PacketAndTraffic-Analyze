#include<iostream>
#include "PcapLiveDevice.h"
#include "PcapLiveDeviceList.h"
#include "stdlib.h"
#include "SystemUtils.h"
#include "IPv4Layer.h"
#include "IPv6Layer.h"
#include "Packet.h"
#include "EthLayer.h"
#include "TcpLayer.h"
#include "string.h"
#include <math.h>           // sqrtf, powf, cosf, sinf, floorf, ceilf
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <stdio.h>
#include "imgui/glfw3.h" // Will drag system OpenGL headers
#include <Windows.h>

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// Main code
void ShowNewWindow(const char* title = "New Window")
{
	ImGui::SetNextWindowSize(ImVec2(500, 500)); // 창 크기를 설정합니다.
	ImGui::Begin(title); // 창을 생성합니다.

	// 여기에 창에 추가할 내용을 작성하세요.

	ImGui::End(); // 창을 닫습니다.
}
int main(int, char**)
{
    // GLFW 에러 콜백 설정

	std::vector<pcpp::PcapLiveDevice*> list = pcpp::PcapLiveDeviceList::getInstance().getPcapLiveDevicesList();

    bool DeviceListBegin = true;
    bool Packet_Traffic_Graph = false;
	bool Packet_Log = false;
	bool More_Information = false;
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return -1;
    ImVec4 clear_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

    // OpenGL 버전을 3.3으로 설정 (필요에 따라 변경 가능)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // GLFW 윈도우 생성
    GLFWwindow* window = glfwCreateWindow(1000, 640, "P&T Analyze ", NULL, NULL);
    if (window == NULL)
        return -1;
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // V-Sync 활성화
    glfwSetWindowSizeLimits(window, 1000, 640, 1000, 640);
    // ImGui 컨텍스트 초기화
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // ImGui 스타일 설정
    ImGui::StyleColorsDark();

    // 플랫폼/렌더러 바인딩
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    pcpp::PcapLiveDevice* dev;

    // 메인 루프
    while (!glfwWindowShouldClose(window))
    {   
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ImGui 내용 여기에 작성
        if (DeviceListBegin) {
            static int Device_idx = -1;
            ImGui::SetNextWindowSize(ImVec2(650, 300), ImGuiCond_Once);
            ImGui::Begin("DeviceList", &DeviceListBegin);
            for (int i = 0; i < list.size(); i++) {
                char buf[256];
                sprintf(buf, "[%d] %s", i+1, list[i]->getDesc().c_str());
                if (ImGui::Selectable(buf,Device_idx==i, ImGuiSelectableFlags_AllowDoubleClick)) {
                    if (ImGui::IsMouseDoubleClicked(0)) {
                        if (MessageBox(NULL, "Do you want to open that network adapter?", "Select Device", MB_OKCANCEL)==IDOK) {
                            Device_idx = i;
                            DeviceListBegin = false;
                            Packet_Traffic_Graph = true;
                            Packet_Log = true;
                            More_Information = true;
                            dev = list[Device_idx];
                        }

                    }
                }
            }
            ImGui::End();
        }
        if (Packet_Traffic_Graph) {
            std::cout << "Device opened successfully" << std::endl;
	        std::cout << "Device info: " << dev->getDesc() << std::endl;
	        std::cout << "Device MAC address: " << dev->getMacAddress().toString() << std::endl;
	        std::cout << "Device IP address: " << dev->getIPv4Address().toString() << std::endl;
	        std::cout << "Device gateway IP address: " << dev->getDefaultGateway().toString() << std::endl;
	        std::cout << "Device Type: " << dev->getDeviceType() << std::endl;
            ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(650, 300), ImGuiCond_Once);
            ImGui::Begin("Packet Traffic Graph", &Packet_Traffic_Graph, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
            static bool animate = true;
            ImGui::Checkbox("Animate", &animate);
            static float arr[] = { 0.6f, 0.1f, 1.0f, 0.5f, 0.92f, 0.1f, 0.2f };
            static float values[90] = {};
            static int values_offset = 0;
            static double refresh_time = 0.0;
            if (!animate || refresh_time == 0.0)
                refresh_time = ImGui::GetTime();
            while (refresh_time < ImGui::GetTime()) // Create data at fixed 60 Hz rate for the demo
            {
                static float phase = 0.0f;
                values[values_offset] = cosf(phase);
                values_offset = (values_offset + 1) % IM_ARRAYSIZE(values);
                phase += 0.10f * values_offset;
                refresh_time += 1.0f / 60.0f;
            }

            // Plots can display overlay texts
            // (in this example, we will display an average value)
            {
                float average = 0.0f;
                for (int n = 0; n < IM_ARRAYSIZE(values); n++)
                    average += values[n];
                average /= (float)IM_ARRAYSIZE(values);
                char overlay[32];
                sprintf(overlay, "avg %f", average);
                ImGui::PushItemWidth(-1);
                ImGui::PlotLines("", values, IM_ARRAYSIZE(values), values_offset, overlay, -1.0f, 1.0f, ImVec2(0, 250.0f));
            }
            ImGui::End();
        }
        if (Packet_Log) {
            ImGui::SetNextWindowPos(ImVec2(10, 320), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(650, 310), ImGuiCond_Once);
            ImGui::Begin("Packet Log", &Packet_Log, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar);
            if (ImGui::BeginMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    if (ImGui::MenuItem("Clear", "Ctrl+D")) { /* "Open" 메뉴 아이템 선택 시 처리 */ }
                    if (ImGui::MenuItem("Save", "Ctrl+S")) { /* "Save" 메뉴 아이템 선택 시 처리 */ }
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }
            const char* items[] = { "AAAA", "BBBB", "CCCC","DDDD","CCCC","frdfdasf" };
            static int item_current = 0;
            if (ImGui::BeginListBox("##listbox 2", ImVec2(-FLT_MIN, 16 * ImGui::GetTextLineHeightWithSpacing())))
            {
                for (int n = 0; n < IM_ARRAYSIZE(items); n++)
                {
                    const bool is_selected = (item_current == n);
                    if (ImGui::Selectable(items[n], is_selected))
                        item_current = n;

                    // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndListBox();
            }
            ImGui::End();
        }
        if (More_Information) {
            ImGui::SetNextWindowPos(ImVec2(670, 10), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(320, 620), ImGuiCond_Once);
            ImGui::Begin("More Information", &More_Information, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar);
            if (ImGui::BeginMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    if (ImGui::MenuItem("Open", "Ctrl+O")) { /* "Open" 메뉴 아이템 선택 시 처리 */ }
                    if (ImGui::MenuItem("Save", "Ctrl+S")) { /* "Save" 메뉴 아이템 선택 시 처리 */ }
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }
            ImGui::End();
        }
        // 렌더링
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(1.0f, 1.0f, 1.0f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // 정리
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

 
// 
// 
// 
// 
// 
// 
// 
//static void glfw_error_callback(int error, const char* description)
//{
//    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
//}
//
//// Main code
//int main(int, char**)
//{
//    glfwSetErrorCallback(glfw_error_callback);
//    if (!glfwInit())
//        return 1;
//
//    // Decide GL+GLSL versions
//#if defined(IMGUI_IMPL_OPENGL_ES2)
//    // GL ES 2.0 + GLSL 100
//    const char* glsl_version = "#version 100";
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
//    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
//#elif defined(__APPLE__)
//    // GL 3.2 + GLSL 150
//    const char* glsl_version = "#version 150";
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
//    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
//    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
//#else
//    // GL 3.0 + GLSL 130
//    const char* glsl_version = "#version 130";
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
//    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
//    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
//#endif
//
//    // Create window with graphics context
//    GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);
//    if (window == nullptr)
//        return 1;
//    glfwMakeContextCurrent(window);
//    glfwSwapInterval(1); // Enable vsync
//
//    // Setup Dear ImGui context
//    IMGUI_CHECKVERSION();
//    ImGui::CreateContext();
//    ImGuiIO& io = ImGui::GetIO(); (void)io;
//    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
//    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
//
//    // Setup Dear ImGui style
//    ImGui::StyleColorsDark();
//    //ImGui::StyleColorsLight();
//
//    // Setup Platform/Renderer backends
//    ImGui_ImplGlfw_InitForOpenGL(window, true);
//#ifdef __EMSCRIPTEN__
//    ImGui_ImplGlfw_InstallEmscriptenCanvasResizeCallback("#canvas");
//#endif
//    ImGui_ImplOpenGL3_Init(glsl_version);
//
//    // Load Fonts
//    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
//    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
//    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
//    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
//    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
//    // - Read 'docs/FONTS.md' for more instructions and details.
//    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
//    // - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
//    //io.Fonts->AddFontDefault();
//    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
//    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
//    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
//    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
//    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
//    //IM_ASSERT(font != nullptr);
//
//    // Our state
//    bool show_demo_window = true;
//    bool show_another_window = false;
//    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
//
//    // Main loop
//#ifdef __EMSCRIPTEN__
//    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
//    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
//    io.IniFilename = nullptr;
//    EMSCRIPTEN_MAINLOOP_BEGIN
//#else
//    while (!glfwWindowShouldClose(window))
//#endif
//    {
//        // Poll and handle events (inputs, window resize, etc.)
//        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
//        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
//        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
//        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
//        glfwPollEvents();
//
//        // Start the Dear ImGui frame
//        ImGui_ImplOpenGL3_NewFrame();
//        ImGui_ImplGlfw_NewFrame();
//        ImGui::NewFrame();
//
//        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
//        if (show_demo_window)
//            ImGui::ShowDemoWindow(&show_demo_window);
//
//        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
//        {
//            static float f = 0.0f;
//            static int counter = 0;
//
//            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.
//
//            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
//            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
//            ImGui::Checkbox("Another Window", &show_another_window);
//
//            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
//            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color
//
//            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
//                counter++;
//            ImGui::SameLine();
//            ImGui::Text("counter = %d", counter);
//
//            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
//            ImGui::End();
//        }
//
//        // 3. Show another simple window.
//        if (show_another_window)
//        {
//            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
//            ImGui::Text("Hello from another window!");
//            if (ImGui::Button("Close Me"))
//                show_another_window = false;
//            ImGui::End();
//        }
//
//        // Rendering
//        ImGui::Render();
//        int display_w, display_h;
//        glfwGetFramebufferSize(window, &display_w, &display_h);
//        glViewport(0, 0, display_w, display_h);
//        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
//        glClear(GL_COLOR_BUFFER_BIT);
//        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
//        glfwSwapBuffers(window);
//    }
//#ifdef __EMSCRIPTEN__
//    EMSCRIPTEN_MAINLOOP_END;
//#endif
//
//    // Cleanup
//    ImGui_ImplOpenGL3_Shutdown();
//    ImGui_ImplGlfw_Shutdown();
//    ImGui::DestroyContext();
//
//    glfwDestroyWindow(window);
//    glfwTerminate();
//
//    return 0;
//}
