#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_PORT 8888
#define BUFFER_SIZE 1024
#define HEARTBEAT_INTERVAL 30 * 60 * 1000 // 30分钟

std::atomic<bool> running(true);
std::string client_id;
std::string server_ip;
std::string serial_number;
SOCKET client_socket;
std::mutex socket_mutex;

bool connect_to_server() {
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return false;
    }
    
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return false;
    }
    
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    
    if (inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr) <= 0) {
        std::cerr << "Invalid server address" << std::endl;
        closesocket(client_socket);
        WSACleanup();
        return false;
    }
    
    if (connect(client_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Failed to connect to server: " << WSAGetLastError() << std::endl;
        closesocket(client_socket);
        WSACleanup();
        return false;
    }
    
    return true;
}

bool authenticate() {
    std::cout << "\n=== [许可证认证流程] ===" << std::endl;
    std::cout << "步骤1/3: 构造认证请求" << std::endl;
    std::cout << "         请求格式: AUTH:<序列号>" << std::endl;
    std::cout << "         发送内容: AUTH:" << serial_number << std::endl;
    
    std::string request = "AUTH:" + serial_number;
    
    std::lock_guard<std::mutex> lock(socket_mutex);
    if (send(client_socket, request.c_str(), request.length(), 0) == SOCKET_ERROR) {
        std::cerr << "发送失败: " << WSAGetLastError() << std::endl;
        return false;
    }
    
    std::cout << "步骤2/3: 等待服务器响应..." << std::endl;
    
    char buffer[BUFFER_SIZE];
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_received <= 0) {
        std::cerr << "接收失败: " << WSAGetLastError() << std::endl;
        return false;
    }
    
    buffer[bytes_received] = '\0';
    std::string response(buffer);
    
    std::cout << "步骤3/3: 解析服务器响应" << std::endl;
    std::cout << "         响应内容: " << response << std::endl;
    
    if (response.find("OK:") == 0) {
        client_id = response.substr(3);
        std::cout << "\n[认证成功]" << std::endl;
        std::cout << "├─ 服务器已验证序列号有效性" << std::endl;
        std::cout << "├─ 许可证容量充足" << std::endl;
        std::cout << "└─ 授予客户端ID: " << client_id << std::endl;
        return true;
    } else if (response.find("DENY:") == 0) {
        std::cerr << "\n[认证失败]" << std::endl;
        std::cerr << "└─ 原因: " << response.substr(5) << std::endl;
        return false;
    }
    
    return false;
}

void heartbeat_thread() {
    while (running) {
        Sleep(HEARTBEAT_INTERVAL);
        
        if (!running) break;
        
        std::string request = "HEARTBEAT:" + client_id;
        
        std::lock_guard<std::mutex> lock(socket_mutex);
        if (send(client_socket, request.c_str(), request.length(), 0) == SOCKET_ERROR) {
            std::cerr << "Heartbeat failed, reconnecting..." << std::endl;
            
            closesocket(client_socket);
            WSACleanup();
            
            if (connect_to_server()) {
                authenticate();
            } else {
                std::cerr << "Reconnection failed" << std::endl;
            }
        } else {
            char buffer[BUFFER_SIZE];
            int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
            if (bytes_received <= 0 || std::string(buffer, bytes_received).find("OK") != 0) {
                std::cerr << "Heartbeat response error, re-authenticating..." << std::endl;
                authenticate();
            }
        }
    }
}

void release_license() {
    if (!client_id.empty()) {
        std::string request = "RELEASE:" + client_id;
        
        std::lock_guard<std::mutex> lock(socket_mutex);
        send(client_socket, request.c_str(), request.length(), 0);
    }
}

BOOL WINAPI console_handler(DWORD signal) {
    if (signal == CTRL_C_EVENT || signal == CTRL_BREAK_EVENT || signal == CTRL_CLOSE_EVENT) {
        running = false;
        release_license();
        
        closesocket(client_socket);
        WSACleanup();
        
        std::cout << "License released, exiting..." << std::endl;
        exit(0);
    }
    return TRUE;
}

int main(int argc, char* argv[]) {
    // 设置控制台输出为UTF-8编码，解决中文乱码问题
    SetConsoleOutputCP(CP_UTF8);
    
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <ServerIP> <SerialNumber>" << std::endl;
        return 1;
    }
    
    server_ip = argv[1];
    serial_number = argv[2];
    
    if (SetConsoleCtrlHandler(console_handler, TRUE) == FALSE) {
        std::cerr << "Failed to set console handler" << std::endl;
        return 1;
    }
    
    std::cout << "Connecting to license server " << server_ip << ":" << SERVER_PORT << std::endl;
    
    if (!connect_to_server()) {
        return 1;
    }
    
    std::cout << "Connected, authenticating..." << std::endl;
    
    if (!authenticate()) {
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }
    
    std::thread hb_thread(heartbeat_thread);
    hb_thread.detach();
    
    std::cout << "Software A started, press Ctrl+C to exit" << std::endl;
    
    while (running) {
        Sleep(1000);
    }
    
    release_license();
    closesocket(client_socket);
    WSACleanup();
    
    return 0;
}