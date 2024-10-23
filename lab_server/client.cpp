#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>

int PORT = 6666;
char SERVER_IP[] = "127.0.0.1";
std::string username;

// Функция для приема сообщений от сервера
void receiveMessages(int clientSocket) {
    char buffer[1024] = {0};
    while (true) {
        int valread = read(clientSocket, buffer, 1024);
        if (valread > 0) {
            std::string message(buffer);
            // Выделяем личные сообщения
            if (message.find('@') == 0) {
                // Личное сообщение
                std::cout << "\033[1;32m" << message << "\033[0m\n";  // Зеленый цвет
            } else {
                // Обычное сообщение
                std::cout << message << std::endl;
            }
            memset(buffer, 0, sizeof(buffer)); // Очистка буфера
        }
    }
}

int main() {
    int clientSocket;
    struct sockaddr_in serverAddress;
    std::string message;

    // Создание сокета
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation failed\n";
        return -1;
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);

    // Преобразование IP-адреса
    if (inet_pton(AF_INET, SERVER_IP, &serverAddress.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported\n";
        return -1;
    }

    // Подключение к серверу
    if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        std::cerr << "Connection failed\n";
        return -1;
    }

    std::cout << "Connected to server\n";

    // Ввод имени пользователя перед подключением
    std::cout << "Enter your name: ";
    std::getline(std::cin, username);
    send(clientSocket, username.c_str(), username.length(), 0);

    // Запуск отдельного потока для приема сообщений от сервера
    std::thread receiveThread(receiveMessages, clientSocket);
    receiveThread.detach(); // Отделяем поток для независимого выполнения

    // Цикл отправки сообщений
    while (true) {
        std::getline(std::cin, message);

        // Проверка на специальные команды
        if (message.find("#NewName") == 0) {
            // Команда для смены имени
            send(clientSocket, message.c_str(), message.length(), 0);
        } else if (message.find("#users") == 0) {
            // Команда для запроса списка пользователей
            send(clientSocket, message.c_str(), message.length(), 0);
        } else if (message.find("#ToUser") == 0) {
            // Команда для отправки личного сообщения
            send(clientSocket, message.c_str(), message.length(), 0);
        } else {
            // Отправка обычного сообщения
            send(clientSocket, message.c_str(), message.length(), 0);
        }

        std::cout << "Message sent\n";
    }

    return 0;
}
