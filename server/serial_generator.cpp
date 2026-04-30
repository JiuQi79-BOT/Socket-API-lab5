#include <iostream>
#include <windows.h>
#include <string>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <sstream>
#include <map>

#define DB_FILE "database/licenses.txt"

struct License {
    std::string serial_number;
    std::string username;
    std::string password;
    int max_users;
    int current_users;
};

std::map<std::string, License> load_licenses() {
    std::map<std::string, License> licenses;
    std::ifstream file(DB_FILE);
    if (!file.is_open()) {
        return licenses;
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
    return licenses;
}

void save_license(const License& lic) {
    std::ofstream file(DB_FILE, std::ios::app);
    if (!file.is_open()) {
        std::cerr << "无法打开数据库文件" << std::endl;
        return;
    }
    
    file << lic.serial_number << "|" 
         << lic.username << "|" 
         << lic.password << "|" 
         << lic.max_users << "|" 
         << lic.current_users << std::endl;
    file.close();
}

std::string generate_serial() {
    std::string serial;
    for (int i = 0; i < 10; ++i) {
        serial += std::to_string(rand() % 10);
    }
    return serial;
}

int main() {
    srand(time(nullptr));
    
    std::string username, password;
    int max_users;
    
    std::cout << "=== License Generator ===" << std::endl;
    std::cout << "Enter username: ";
    std::cin >> username;
    std::cout << "Enter password: ";
    std::cin >> password;
    std::cout << "Enter max users (e.g., 10, 50): ";
    std::cin >> max_users;
    
    auto licenses = load_licenses();
    
    std::string serial;
    bool unique = false;
    while (!unique) {
        serial = generate_serial();
        if (licenses.find(serial) == licenses.end()) {
            unique = true;
        }
    }
    
    License new_lic;
    new_lic.serial_number = serial;
    new_lic.username = username;
    new_lic.password = password;
    new_lic.max_users = max_users;
    new_lic.current_users = 0;
    
    save_license(new_lic);
    
    std::cout << std::endl << "=== License Generated Successfully ===" << std::endl;
    std::cout << "Username: " << username << std::endl;
    std::cout << "Max Users: " << max_users << std::endl;
    std::cout << "Serial Number: " << serial << std::endl;
    std::cout << "Please keep this serial number safe!" << std::endl;
    
    return 0;
}