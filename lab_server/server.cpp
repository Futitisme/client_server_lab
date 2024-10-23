// Server.cpp
#include <iostream>
#include <thread>
#include <vector>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
int PORT = 6666;
int MAX_CLIENTS = 10;
std::vector<int> clients;
void handleClient(int clientSocket) {
    char buffer[1024] = {0};
    while (true) {
        int valread = read(clientSocket, buffer, 1024);
        if (valread == 0) {
            std::cout << "Client disconnected\n";
            break;
        }
        std::cout << "Client message: " << buffer << std::endl;
        // Broadcast message to all clients (excluding sender)
        for (int client : clients) {
            if (client != clientSocket) {
                send(client, buffer, strlen(buffer), 0);
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
    // Creating socket file descriptor
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "Socket creation failed\n";
        return -1;
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    // Binding socket to port
    if (bind(serverSocket, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed\n";
        return -1;
    }
    // Listening for incoming connections
    if (listen(serverSocket, MAX_CLIENTS) < 0) {
        std::cerr << "Listen failed\n";
        return -1;
    }
    std::cout << "Server listening on port " << PORT << std::endl;
    while (true) {
        if ((clientSocket = accept(serverSocket, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            std::cerr << "Accept failed\n";
            return -1;
        }
        std::cout << "New client connected\n";
        clients.push_back(clientSocket);
        std::thread clientThread(&handleClient, clientSocket);
        clientThread.detach(); // Detach the thread to run independently
    }
    return 0;
}