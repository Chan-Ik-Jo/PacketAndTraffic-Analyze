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
#include "DnsLayer.h"
#include "UdpLayer.h"
#include "string.h"
#include <math.h>           // sqrtf, powf, cosf, sinf, floorf, ceilf
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <stdio.h>
#include "imgui/glfw3.h" 
#include <Windows.h>
#include <sstream>
#include <chrono>
#include <thread>
#include <future>
//imgui + pcapplusplus + glfw header

// GLFW 에러 콜백 함수
static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}
//PacketInfo 구조체
struct PacketInfo {
    std::vector<pcpp::Packet> SrcPacketList;
    std::vector<pcpp::Packet> DstPacketList;
    std::string summarySrcPacket="";
	std::string summaryDstPacket="";
	float packetCount = 0.0f;
	float count = 0.0f;

	void clear() {
		SrcPacketList.clear();
		DstPacketList.clear();
		packetCount = 0.0f;
		count = 0.0f;
	}
    std::string printTcpFlags(pcpp::TcpLayer* tcpLayer)
    {
        std::string result = "";
        if (tcpLayer->getTcpHeader()->synFlag == 1)
            result += "SYN ";
        if (tcpLayer->getTcpHeader()->ackFlag == 1)
            result += "ACK ";
        if (tcpLayer->getTcpHeader()->pshFlag == 1)
            result += "PSH ";
        if (tcpLayer->getTcpHeader()->cwrFlag == 1)
            result += "CWR ";
        if (tcpLayer->getTcpHeader()->urgFlag == 1)
            result += "URG ";
        if (tcpLayer->getTcpHeader()->eceFlag == 1)
            result += "ECE ";
        if (tcpLayer->getTcpHeader()->rstFlag == 1)
            result += "RST ";
        if (tcpLayer->getTcpHeader()->finFlag == 1)
            result += "FIN ";

        return result;
    }
};
//시간 측정
auto start = std::chrono::system_clock::now();

//패킷 도착시 호출되는 콜백함수
static void onPacketArrives(pcpp::RawPacket* packet, pcpp::PcapLiveDevice* dev, void* cookie) {
	pcpp::Packet parsedpacket(packet);
	PacketInfo* packetInfo = (PacketInfo*)cookie;
    packetInfo->count++;
	

	pcpp::IPv4Layer* ipLayer = parsedpacket.getLayerOfType<pcpp::IPv4Layer>();
    //packetInfo->PrinttoConsole();
	if (ipLayer != NULL) {
        if (ipLayer->getIPv4Header() != nullptr) {
            if (ipLayer->getSrcIPv4Address() == dev->getIPv4Address()) {
                if (packetInfo->SrcPacketList.size() < 2000) {
                    packetInfo->SrcPacketList.push_back(parsedpacket);
                }
                else {
                    packetInfo->SrcPacketList.erase(packetInfo->SrcPacketList.begin());
                    packetInfo->SrcPacketList.push_back(parsedpacket);
                }
            }
            else if (ipLayer->getDstIPv4Address() == dev->getIPv4Address()) {
                if (packetInfo->DstPacketList.size() < 2000) {
                    packetInfo->DstPacketList.push_back(parsedpacket);
                }
                else {
                    packetInfo->DstPacketList.erase(packetInfo->DstPacketList.begin());
                    packetInfo->DstPacketList.push_back(parsedpacket);
                }

            }
        }

	}
	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = end - start;
	if (elapsed_seconds.count() >= 1) {
		packetInfo->packetCount = packetInfo->count;
		packetInfo->count = 0.0f;
		start = std::chrono::system_clock::now();
	}

}
// Main code
int main(int, char**)
{
    // GLFW 에러 콜백 설정
	PacketInfo packetInfo;
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
    GLFWwindow* window = glfwCreateWindow(1010, 640, "P&T Analyze ", NULL, NULL);
    if (window == NULL)
        return -1;
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // V-Sync 활성화
    glfwSetWindowSizeLimits(window, 1010, 640, 1010, 640);
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
    pcpp::PcapLiveDevice::DeviceConfiguration config;
    config.mode = pcpp::PcapLiveDevice::DeviceMode::Promiscuous;
    config.packetBufferTimeoutMs = 1000;

    std::string SrcEthMac;
    std::string DstEthMac;
    std::string SrcIp;
    std::string DstIp;
    std::string SrcTcpPort;
    std::string DstTcpPort;
    std::string SrcUdpPort;
    std::string DstUdpPort;
	std::string TcpFlag;
    std::string PacketClass;
    static bool animate =false;
    


    // 메인 루프
    while (!glfwWindowShouldClose(window))
    {   

        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ImGui 내용 여기에 작성
		//device list 창
        if (DeviceListBegin) {
            static int Device_idx = -1;
			ImGui::SetNextWindowPos(ImVec2(230,100), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(550, 300), ImGuiCond_Once);
            ImGui::Begin("DeviceList", &DeviceListBegin,ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
            for (int i = 0; i < list.size(); i++) {
                std::stringstream ss;
                ss << "[" << (i + 1) << "] " << list[i]->getDesc();
                std::string buf = ss.str();              
                if (ImGui::Selectable(buf.c_str(), Device_idx == i, ImGuiSelectableFlags_AllowDoubleClick)) {
                    if (ImGui::IsMouseDoubleClicked(0)) {
                        std::stringstream ss;
                        ss << list[i]->getDesc()<<" is Open?";
                        std::string buf = ss.str();
                        if (MessageBox(NULL,buf.c_str(), "Select Device", MB_OKCANCEL) == IDOK) {
                            Device_idx = i;
                            dev = list[Device_idx];
                            if (!dev->open(config)) {
                                if (MessageBox(NULL, "Cannot open device", "Error", MB_OK) == IDOK) {
                                    DeviceListBegin = true;

                                }
                            }
                            else {
								if (MessageBox(NULL, "Device opened successfully", "Success", MB_OK) == IDOK) {
                                    DeviceListBegin = false;
                                    Packet_Traffic_Graph = true;
                                    Packet_Log = true;
                                    More_Information = true;
								}   
                            }
                        }

                    }
                }
            }
            ImGui::End();
        }
		//패킷 트래픽 그래프
        if (Packet_Traffic_Graph) {

            ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(650, 300), ImGuiCond_Once);
            ImGui::Begin("Packet Traffic Graph", &Packet_Traffic_Graph, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
            static float values[90] = {};
            static int values_offset = 0;
            static double refresh_time = 0.0;
            if (!animate || refresh_time == 0.0)
                refresh_time = ImGui::GetTime();
            while (refresh_time < ImGui::GetTime()) 
            {

                if (values_offset >= 0 && values_offset < sizeof(values) / sizeof(values[0])) {
                    values[values_offset] = packetInfo.packetCount;
                    values_offset = (values_offset + 1) % IM_ARRAYSIZE(values);
                    refresh_time += 1.0f / 1.0f;
                }
            }
            {
                ImGui::PushItemWidth(-1);
                ImGui::PlotLines("", values, IM_ARRAYSIZE(values), values_offset, "", -1.0f,400.0f, ImVec2(0, 284.0f));
            }
            ImGui::End();
        }
		//패킷 로그
        if (Packet_Log) {
            ImGui::SetNextWindowPos(ImVec2(10, 320), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(650, 310), ImGuiCond_Once);
            ImGui::Begin("Packet Log", &Packet_Log, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar);
            if (ImGui::BeginMenuBar())
            {
                if (ImGui::BeginMenu("Capture")) {
					if (ImGui::MenuItem("Start")) {
						dev->startCapture(onPacketArrives, &packetInfo);
						animate = true;
					}
					if (ImGui::MenuItem("Stop")) {
						dev->stopCapture();
						animate = false;
					}
					ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }

            static int item_current1 = 0;
            if (ImGui::BeginListBox("##listbox1", ImVec2(310, 16 * ImGui::GetTextLineHeightWithSpacing())))
            {
                for (int n = 0; n < packetInfo.SrcPacketList.size(); n++)
                {
                    std::string summarySrcPacket = "";
                    try {
                        pcpp::IPv4Layer* ipLayer = packetInfo.SrcPacketList[n].getLayerOfType<pcpp::IPv4Layer>();
                        if (ipLayer != NULL) {
                            summarySrcPacket = "In ------> " + ipLayer->getDstIPv4Address().toString();
                        }
                    }
					catch (const std::exception& e) {
						summarySrcPacket = "In ------> NULL";
					}
                    std::stringstream ss;
					ss << "[" << (n + 1) << "] "<< summarySrcPacket;
					std::string buf = ss.str();
                    const bool is_selected = (item_current1 == n);
                    if (ImGui::Selectable(buf.c_str(), is_selected, ImGuiSelectableFlags_AllowDoubleClick)) {
						if (ImGui::IsMouseDoubleClicked(0)) {
                            item_current1 = n;
                            PacketClass = "Src Packet";
							pcpp::EthLayer* ethLayer = packetInfo.SrcPacketList[item_current1].getLayerOfType<pcpp::EthLayer>();
							if (ethLayer != NULL) {
								SrcEthMac = ethLayer->getSourceMac().toString();
								DstEthMac = ethLayer->getDestMac().toString();
							}
							else {
								SrcEthMac = "";
								DstEthMac = "";
							}
							pcpp::IPv4Layer* ipLayer = packetInfo.SrcPacketList[item_current1].getLayerOfType<pcpp::IPv4Layer>();
							if (ipLayer != NULL) {
								SrcIp = ipLayer->getSrcIPv4Address().toString();
								DstIp = ipLayer->getDstIPv4Address().toString();
							}
							else {
								SrcIp = "";
                                DstIp="";
							}
							pcpp::TcpLayer* tcpLayer = packetInfo.SrcPacketList[item_current1].getLayerOfType<pcpp::TcpLayer>();
							if (tcpLayer != NULL) {
								SrcTcpPort = std::to_string(tcpLayer->getSrcPort());
								DstTcpPort = std::to_string(tcpLayer->getDstPort());
								TcpFlag = packetInfo.printTcpFlags(tcpLayer);
							}
							else {
								SrcTcpPort = "";
								DstTcpPort = "";
							}
							pcpp::UdpLayer* udpLayer = packetInfo.SrcPacketList[item_current1].getLayerOfType<pcpp::UdpLayer>();
							if (udpLayer != NULL) {
								SrcUdpPort = std::to_string(udpLayer->getSrcPort());
								DstUdpPort = std::to_string(udpLayer->getDstPort());
							}
							else {
								SrcUdpPort = "";
								DstUdpPort = "";
							}
						}
                    }
                }
                ImGui::EndListBox();
            }
            ImGui::SameLine();
            
            static int item_current2 = 0;
            if (ImGui::BeginListBox("##listbox2", ImVec2(-FLT_MIN/2, 16 * ImGui::GetTextLineHeightWithSpacing())))
            {
                for (int n = 0; n < packetInfo.DstPacketList.size(); n++)
                {   
					std::string summaryDstPacket = "";  
                    try {
						pcpp::IPv4Layer* ipLayer = packetInfo.DstPacketList[n].getLayerOfType<pcpp::IPv4Layer>();
						if (ipLayer != NULL) {
							summaryDstPacket = ipLayer->getSrcIPv4Address().toString() + " ------> In";
						}
					}
                    catch (const std::exception& e) {
                        summaryDstPacket = "NULL ------> In";
                    }
                    std::stringstream ss;
                    ss << "[" << (n + 1) << "] "<< summaryDstPacket;
					std::string buf = ss.str();
					const bool is_selected = (item_current2 == n);  
                    if (ImGui::Selectable(buf.c_str(), is_selected, ImGuiSelectableFlags_AllowDoubleClick))
                        if (ImGui::IsMouseDoubleClicked(0)) {
                            item_current2 = n;
                            PacketClass = "Dst Packet";
                            pcpp::EthLayer* ethLayer = packetInfo.DstPacketList[item_current2].getLayerOfType<pcpp::EthLayer>();
                            if (ethLayer != NULL) {
                                SrcEthMac = ethLayer->getSourceMac().toString();
								DstEthMac = ethLayer->getDestMac().toString();
                            }
                            else {
                                SrcEthMac = "";
								DstEthMac = "";
                            }
                            pcpp::IPv4Layer* ipLayer = packetInfo.DstPacketList[item_current2].getLayerOfType<pcpp::IPv4Layer>();
                            if (ipLayer != NULL) {
                                SrcIp = ipLayer->getSrcIPv4Address().toString();
								DstIp = ipLayer->getDstIPv4Address().toString();
                            }
                            else {
                                SrcIp = "";
								DstIp = "";
                            }
                            pcpp::TcpLayer* tcpLayer = packetInfo.DstPacketList[item_current2].getLayerOfType<pcpp::TcpLayer>();
                            if (tcpLayer != NULL) {
								
                                SrcTcpPort = std::to_string(tcpLayer->getSrcPort());
								DstTcpPort = std::to_string(tcpLayer->getDstPort());
								TcpFlag = packetInfo.printTcpFlags(tcpLayer);
                            }
                            else {
                                SrcTcpPort = "";
								DstTcpPort = "";
                                TcpFlag = "";
                            }
                            pcpp::UdpLayer* udpLayer = packetInfo.DstPacketList[item_current2].getLayerOfType<pcpp::UdpLayer>();
                            if (udpLayer != NULL) {
                                SrcUdpPort = std::to_string(udpLayer->getSrcPort());
								DstUdpPort = std::to_string(udpLayer->getDstPort());
                             
                            }
                            else {
                                SrcUdpPort = "";
								DstUdpPort = "";
                            }
                        }

                    
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndListBox();
            }
            ImGui::End();
        }
        //패킷 상세 정보 와 네트워크 어뎁터 상세 정보
        if (More_Information) {
            ImGui::SetNextWindowPos(ImVec2(670, 10), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(330, 620), ImGuiCond_Once);
            ImGui::Begin("More Information", &More_Information, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar);
   //         if (ImGui::BeginMenuBar())
			//{

   //             if (ImGui::BeginMenu("File"))
   //             {
   //                 if (ImGui::MenuItem("Open", "Ctrl+O")) { /* "Open" 메뉴 아이템 선택 시 처리 */ }
   //                 if (ImGui::MenuItem("Save", "Ctrl+S")) { /* "Save" 메뉴 아이템 선택 시 처리 */ }
   //                 ImGui::EndMenu();
   //             }
   //             ImGui::EndMenuBar();
   //         }
            if (ImGui::CollapsingHeader("Packet Information")) {
				ImGui::SeparatorText("Packet Information");
				ImGui::Text("Packet Class : %s", PacketClass.c_str());
				if (ImGui::TreeNode("Ethernet Layer")) {
					ImGui::Text("Source Mac Address : %s", SrcEthMac.c_str());
					ImGui::Text("Destination Mac Address : %s", DstEthMac.c_str());
					ImGui::TreePop();
				}
                if (ImGui::TreeNode("IPv4 Layer")) {
					ImGui::Text("Source IP Address : %s", SrcIp.c_str());
					ImGui::Text("Destination IP Address : %s", DstIp.c_str());
					ImGui::TreePop();
                }
                if (ImGui::TreeNode("TCP Layer")) {
					ImGui::Text("Source Port : %s", SrcTcpPort.c_str());
					ImGui::Text("Destination Port : %s", DstTcpPort.c_str());
					ImGui::Text("TCP Flags : %s", TcpFlag.c_str());
					ImGui::TreePop();
                }
                if (ImGui::TreeNode("UDP Layer")) {
					ImGui::Text("Source Port : %s", SrcUdpPort.c_str());
					ImGui::Text("Destination Port : %s", DstUdpPort.c_str());
					ImGui::TreePop();
                }

            }
            if (ImGui::CollapsingHeader("NetworkAdepter Information")) {
                int max_length = 40;
                std::string text = dev->getName();

                if (text.length() > max_length) {
                    text = text.substr(0, max_length) + " ...";
                }
				ImGui::SeparatorText("Name");
                ImGui::Text("%s", text.c_str());
                ImGui::SeparatorText("Description");
                ImGui::Text("%s", dev->getDesc().c_str());
                ImGui::SeparatorText("MAC Address");
                ImGui::Text("%s", dev->getMacAddress().toString().c_str());
                ImGui::SeparatorText("IPv4 Address");
                ImGui::Text("%s", dev->getIPv4Address().toString().c_str());
                ImGui::SeparatorText("IPv6 Address");
                ImGui::Text("%s", dev->getIPv6Address().toString().c_str());
                ImGui::SeparatorText("Gateway");
                ImGui::Text("%s", dev->getDefaultGateway().toString().c_str());
                ImGui::SeparatorText("MTU");
                ImGui::Text("%d", dev->getMtu());
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
