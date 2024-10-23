#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>

int PORT = 6666;
char SERVER_IP[] = "192.168.64.1";

// Функция для приема сообщений от сервера
void receiveMessages(int clientSocket) {
    char buffer[1024] = {0};
    while (true) {
        int valread = read(clientSocket, buffer, 1024);
        if (valread > 0) {
            std::cout << "\nMessage from another client: " << buffer << std::endl;
            memset(buffer, 0, sizeof(buffer)); // Очистка буфера после получения сообщения
        }
    }
}

int main() {
    int clientSocket;
    struct sockaddr_in serverAddress;
    char message[1024] = {0};

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

    // Запуск отдельного потока для приема сообщений от сервера
    std::thread receiveThread(receiveMessages, clientSocket);
    receiveThread.detach(); // Отделяем поток для независимого выполнения

    // Цикл отправки сообщений
    while (true) {
        std::cout << "Enter message: ";
        std::cin.getline(message, 1024);
        send(clientSocket, message, strlen(message), 0);
        std::cout << "Message sent\n";
    }

    return 0;
}
