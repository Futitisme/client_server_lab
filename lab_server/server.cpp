#include <iostream>
#include <thread>
#include <vector>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <map>
#include <algorithm>

int PORT = 6666;
int MAX_CLIENTS = 10;
std::map<int, std::string> clients; // Клиенты и их имена

void handleClient(int clientSocket) {
    char buffer[1024] = {0};
    std::string username;

    // Запрашиваем у клиента имя
    send(clientSocket, "Enter your name: ", 17, 0);
    int valread = read(clientSocket, buffer, 1024);
    username = std::string(buffer).substr(0, valread);
    username.erase(std::remove(username.begin(), username.end(), '\n'), username.end());

    // Проверка уникальности имени
    bool nameExists = false;
    for (const auto& client : clients) {
        if (client.second == username) {
            nameExists = true;
            break;
        }
    }

    if (nameExists) {
        send(clientSocket, "Name already taken, try again: ", 32, 0);
        return handleClient(clientSocket); // Снова запросить имя
    } else {
        clients[clientSocket] = username;
        std::cout << username << " has joined the chat." << std::endl;
    }

    // Основной цикл обработки сообщений
    while (true) {
        valread = read(clientSocket, buffer, 1024);
        if (valread == 0) {
            std::cout << username << " disconnected\n";
            clients.erase(clientSocket);
            break;
        }

        std::string message(buffer);
        if (message.find("#NewName") == 0) {
            // Смена имени
            std::string newName = message.substr(9);
            if (newName.empty()) continue;

            bool newNameExists = false;
            for (const auto& client : clients) {
                if (client.second == newName) {
                    newNameExists = true;
                    break;
                }
            }

            if (newNameExists) {
                send(clientSocket, "Name already taken\n", 19, 0);
            } else {
                std::cout << username << " changed name to " << newName << std::endl;
                clients[clientSocket] = newName;
                username = newName;
            }
        } else if (message.find("#users") == 0) {
            // Отправка списка пользователей
            std::string userList = "Connected users:\n";
            for (const auto& client : clients) {
                userList += client.second + "\n";
            }
            send(clientSocket, userList.c_str(), userList.length(), 0);
        } else if (message.find("#ToUser") == 0) {
            // Отправка личного сообщения
            size_t spacePos = message.find(' ', 8);
            std::string targetUser = message.substr(8, spacePos - 8);
            std::string privateMsg = message.substr(spacePos + 1);

            bool userFound = false;
            for (const auto& client : clients) {
                if (client.second == targetUser) {
                    std::string formattedMsg = "@" + username + ": " + privateMsg;
                    send(client.first, formattedMsg.c_str(), formattedMsg.length(), 0);
                    userFound = true;
                    break;
                }
            }

            if (!userFound) {
                send(clientSocket, "User not found\n", 15, 0);
            }
        } else {
            // Обычная рассылка сообщения
            std::string formattedMsg = username + ": " + message;
            for (const auto& client : clients) {
                if (client.first != clientSocket) {
                    send(client.first, formattedMsg.c_str(), formattedMsg.length(), 0);
                }
            }
        }

        memset(buffer, 0, sizeof(buffer));
    }

    close(clientSocket);
}

int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Создание сокета
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "Socket creation failed\n";
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Привязка сокета к порту
    if (bind(serverSocket, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed\n";
        return -1;
    }

    // Ожидание подключений
    if (listen(serverSocket, MAX_CLIENTS) < 0) {
        std::cerr << "Listen failed\n";
        return -1;
    }

    std::cout << "Server listening on port " << PORT << std::endl;

    // Основной цикл сервера
    while (true) {
        if ((clientSocket = accept(serverSocket, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            std::cerr << "Accept failed\n";
            return -1;
        }
        std::cout << "New client connected\n";
        std::thread clientThread(&handleClient, clientSocket);
        clientThread.detach(); // Отделяем поток для независимой работы
    }

    return 0;
}
