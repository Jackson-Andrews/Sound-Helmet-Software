
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>

// Link with the Winsock library
#pragma comment(lib, "Ws2_32.lib")

int main() {
    // 1. Initialize Winsock
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed with error: " << result << std::endl;
        return 1;
    }

    // 2. Create a socket
    SOCKET connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (connectSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed with error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // 3. Connect to the ESP32 server
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(10000); // Port must match the ESP32 server
    serverAddr.sin_addr.s_addr = inet_addr("172.20.10.12");

    std::cout << "Connecting to ESP32 server at " << inet_ntoa(serverAddr.sin_addr) << "..." << std::endl;
    result = connect(connectSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if (result == SOCKET_ERROR) {
        std::cerr << "Unable to connect to server. Error: " << WSAGetLastError() << std::endl;
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }
    std::cout << "Successfully connected to ESP32!" << std::endl;
    std::cout << "Enter '1' to turn the LED ON." << std::endl;
    std::cout << "Enter '0' to turn the LED OFF." << std::endl;
    std::cout << "Enter 'q' to quit." << std::endl;


    // 4. Interactive loop to send commands
    char inputChar;
    char recvbuf[512]; // Buffer to receive confirmation message
    int recvbuflen = 512;

    do {
        std::cout << "\n> ";
        std::cin >> inputChar;
        std::cin.ignore(10000, '\n'); // Clear the input buffer

        if (inputChar != '1' && inputChar != '0' && inputChar != 'q') {
            std::cout << "Invalid input. Please enter '1', '0', or 'q'." << std::endl;
            continue;
        }

        if (inputChar == 'q') {
            break; // Exit loop if user types 'q'
        }

        // Send the single character command
        int bytesSent = send(connectSocket, &inputChar, 1, 0);

        if (bytesSent == SOCKET_ERROR) {
            std::cerr << "Send failed with error: " << WSAGetLastError() << std::endl;
            break;
        }

        // Wait for a confirmation message from the ESP32
        int bytesReceived = recv(connectSocket, recvbuf, recvbuflen, 0);
        if (bytesReceived > 0) {
            recvbuf[bytesReceived] = '\0'; // Null-terminate the string
            std::cout << "ESP32 Response: " << recvbuf;
        } else if (bytesReceived == 0) {
            std::cout << "Connection closed by server." << std::endl;
            break;
        } else {
            std::cerr << "Recv failed with error: " << WSAGetLastError() << std::endl;
            break;
        }

    } while (inputChar != 'q');

    // 5. Cleanup
    std::cout << "\nClosing connection." << std::endl;
    closesocket(connectSocket);
    WSACleanup();

    return 0;
}