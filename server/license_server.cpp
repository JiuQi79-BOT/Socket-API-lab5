#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <string>
#include <map>
#include <thread>
#include <mutex>
#include <ctime>
#include <fstream>
#include <sstream>
#include <vector>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8888
#define BUFFER_SIZE 1024
#define HEARTBEAT_INTERVAL 30 // 30分钟
#define MAX_CLIENTS 100
#define DB_FILE "database/licenses.txt"
#define ACTIVE_CLIENTS_FILE "database/active_clients.txt"

struct License {
    std::string serial_number;
    std::string username;
    std::string password;
    int max_users;
    int current_users;
};

struct ClientInfo {
    std::string serial_number;
    std::string client_id;
    std::string client_ip;
    time_t last_heartbeat;
};

std::map<std::string, License> licenses;
std::map<SOCKET, ClientInfo> clients;
std::mutex licenses_mutex;
std::mutex clients_mutex;

void load_licenses() {
    std::ifstream file(DB_FILE);
    if (!file.is_open()) {
        std::cout << "Database file not found, using default licenses" << std::endl;
        licenses["1234567890"] = {"1234567890", "admin", "password123", 10, 0};
        licenses["0987654321"] = {"0987654321", "testorg", "testpass", 5, 0};
        return;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        License lic;
        std::string token;
        std::getline(iss, token, '|'); lic.serial_number = token;
        std::getline(iss, token, '|'); lic.username = token;
        std::getline(iss, token, '|'); lic.password = token;
        std::getline(iss, token, '|'); lic.max_users = std::stoi(token);
        std::getline(iss, token, '|'); lic.current_users = std::stoi(token);
        licenses[lic.serial_number] = lic;
    }
    file.close();
}

void save_licenses() {
    std::ofstream file(DB_FILE);
    if (!file.is_open()) return;
    
    for (auto& pair : licenses) {
        License& lic = pair.second;
        file << lic.serial_number << "|" 
             << lic.username << "|" 
             << lic.password << "|" 
             << lic.max_users << "|" 
             << lic.current_users << std::endl;
    }
    file.close();
}

void save_active_clients() {
    std::ofstream file(ACTIVE_CLIENTS_FILE);
    if (!file.is_open()) return;
    
    for (auto& pair : clients) {
        ClientInfo& client = pair.second;
        file << client.serial_number << "|" 
             << client.client_id << "|" 
             << client.client_ip << "|" 
             << client.last_heartbeat << std::endl;
    }
    file.close();
}

bool validate_serial(const std::string& serial, int& max_users, int& current_users) {
    std::lock_guard<std::mutex> lock(licenses_mutex);
    auto it = licenses.find(serial);
    if (it != licenses.end()) {
        max_users = it->second.max_users;
        current_users = it->second.current_users;
        return true;
    }
    return false;
}

bool update_current_users(const std::string& serial, int delta) {
    std::lock_guard<std::mutex> lock(licenses_mutex);
    auto it = licenses.find(serial);
    if (it != licenses.end()) {
        it->second.current_users += delta;
        if (it->second.current_users < 0) it->second.current_users = 0;
        save_licenses();
        return true;
    }
    return false;
}

void cleanup_dead_clients() {
    while (true) {
        Sleep(60000);
        
        time_t now = time(nullptr);
        std::vector<SOCKET> dead_sockets;
        
        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            for (auto& pair : clients) {
                if (now - pair.second.last_heartbeat > HEARTBEAT_INTERVAL * 60) {
                    dead_sockets.push_back(pair.first);
                }
            }
        }
        
        for (SOCKET sock : dead_sockets) {
            std::lock_guard<std::mutex> lock(clients_mutex);
            auto it = clients.find(sock);
            if (it != clients.end()) {
                update_current_users(it->second.serial_number, -1);
                std::cout << "Client timeout, license released: " << it->second.serial_number << std::endl;
                clients.erase(it);
                save_active_clients();
            }
        }
    }
}

void handle_client(SOCKET client_socket, sockaddr_in client_addr) {
    char buffer[BUFFER_SIZE];
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    
    while (true) {
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            std::lock_guard<std::mutex> lock(clients_mutex);
            if (clients.find(client_socket) != clients.end()) {
                std::string serial = clients[client_socket].serial_number;
                update_current_users(serial, -1);
                clients.erase(client_socket);
                save_active_clients();
            }
            closesocket(client_socket);
            return;
        }
        
        buffer[bytes_received] = '\0';
        std::string request(buffer);
        
        std::string response;
        if (request.find("AUTH:") == 0) {
            std::string serial = request.substr(5);
            std::cout << "\n=== [认证请求] ===" << std::endl;
            std::cout << "客户端IP: " << client_ip << std::endl;
            std::cout << "请求类型: 许可证认证" << std::endl;
            std::cout << "提交序列号: " << serial << std::endl;
            
            int max_users, current_users;
            if (validate_serial(serial, max_users, current_users)) {
                std::cout << "序列号验证: 通过" << std::endl;
                std::cout << "许可容量: " << current_users << "/" << max_users << std::endl;
                
                if (current_users < max_users) {
                    update_current_users(serial, 1);
                    
                    std::string client_id = serial + "_" + std::to_string(time(nullptr)) + "_" + std::to_string(rand());
                    
                    {
                        std::lock_guard<std::mutex> lock(clients_mutex);
                        clients[client_socket] = {serial, client_id, client_ip, time(nullptr)};
                        save_active_clients();
                    }
                    
                    std::cout << "生成客户端ID: " << client_id << std::endl;
                    std::cout << "认证结果: 成功" << std::endl;
                    response = "OK:" + client_id;
                } else {
                    std::cout << "认证结果: 失败 - 许可证已达最大使用人数" << std::endl;
                    response = "DENY:Max users exceeded";
                }
            } else {
                std::cout << "序列号验证: 失败 - 无效序列号" << std::endl;
                std::cout << "认证结果: 失败" << std::endl;
                response = "DENY:Invalid serial number";
            }
        } else if (request.find("HEARTBEAT:") == 0) {
            std::string client_id = request.substr(10);
            
            std::lock_guard<std::mutex> lock(clients_mutex);
            auto it = clients.find(client_socket);
            if (it != clients.end() && it->second.client_id == client_id) {
                it->second.last_heartbeat = time(nullptr);
                save_active_clients();
                response = "OK";
            } else {
                response = "DENY:Unauthorized client";
            }
        } else if (request.find("RELEASE:") == 0) {
            std::string client_id = request.substr(8);
            
            std::lock_guard<std::mutex> lock(clients_mutex);
            auto it = clients.find(client_socket);
            if (it != clients.end() && it->second.client_id == client_id) {
                update_current_users(it->second.serial_number, -1);
                clients.erase(it);
                save_active_clients();
                response = "OK";
            } else {
                response = "DENY:Invalid release request";
            }
            
            closesocket(client_socket);
            send(client_socket, response.c_str(), response.length(), 0);
            return;
        } else {
            response = "DENY:无效的请求格式";
        }
        
        send(client_socket, response.c_str(), response.length(), 0);
    }
}

int main() {
    // 设置控制台输出为UTF-8编码，解决中文乱码问题
    SetConsoleOutputCP(CP_UTF8);
    
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return 1;
    }
    
    load_licenses();
    
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }
    
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed: " << WSAGetLastError() << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }
    
    if (listen(server_socket, MAX_CLIENTS) == SOCKET_ERROR) {
        std::cerr << "Listen failed: " << WSAGetLastError() << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }
    
    std::cout << "License Server started successfully, listening on port " << PORT << std::endl;
    std::cout << "Loaded " << licenses.size() << " licenses" << std::endl;
    
    std::thread cleanup_thread(cleanup_dead_clients);
    cleanup_thread.detach();
    
    while (true) {
        sockaddr_in client_addr;
        int client_addr_size = sizeof(client_addr);
        SOCKET client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_addr_size);
        
        if (client_socket == INVALID_SOCKET) {
            std::cerr << "接受连接失败: " << WSAGetLastError() << std::endl;
            continue;
        }
        
        std::thread client_thread(handle_client, client_socket, client_addr);
        client_thread.detach();
    }
    
    closesocket(server_socket);
    WSACleanup();
    return 0;
}