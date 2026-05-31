// TCP C++ client for Windows which sends a data structure to an ESP32 server
// Compiled with g++ (MinGW) on Windows: g++ your_file_name.cpp -o client.exe -lws2_32

#define WIN32_LEAN_AND_MEAN // Excludes lesser-used APIs from windows.h

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <cstdint> // For fixed-width integers like int16_t, int32_t

// Link with the Winsock library
#pragma comment(lib, "Ws2_32.lib")

// The data structure to be sent.
// It MUST EXACTLY match the structure on the ESP32.
// __attribute__((packed)) is crucial to prevent compiler padding.
#pragma pack(push, 1)
struct __attribute__((packed)) Data {
  int16_t seq;      // sequence number
  int32_t distance;
  float   voltage;
  char    text[50];
};
#pragma pack(pop)

void delay(unsigned int milliseconds) {
    Sleep(milliseconds); // Use the Windows Sleep function for better performance
}

int main() {
    // 1. Initialize Winsock
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed with error: " << result << std::endl;
        return 1;
    }
    std::cout << "Winsock initialized." << std::endl;

    // 2. Create a socket
    SOCKET connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (connectSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed with error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }
    std::cout << "Socket created." << std::endl;

    // 3. Connect to the ESP32 server
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(10000); // Port must match the ESP32 server
    // Set the ESP32's IP address. Find this from the ESP32's Serial Monitor output.
    // **FIX**: Replaced inet_pton with the more compatible inet_addr for MinGW/g++.
    serverAddr.sin_addr.s_addr = inet_addr("172.20.10.12");

    std::cout << "Connecting to server..." << std::endl;
    result = connect(connectSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if (result == SOCKET_ERROR) {
        std::cerr << "Unable to connect to server. Error: " << WSAGetLastError() << std::endl;
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }
    std::cout << "Successfully connected to server!" << std::endl;

    // 4. Initialize data and start sending
    Data dataToSend = { 0, 56, 3.14159, "hello from PC" };

    while (true) {
        std::cout << "Sending packet -> seq: " << dataToSend.seq
                  << ", dist: " << dataToSend.distance
                  << ", volt: " << dataToSend.voltage << ", text: " << dataToSend.text << std::endl;

        // Send the raw bytes of the struct
        int bytesSent = send(connectSocket, (const char*)&dataToSend, sizeof(Data), 0);

        if (bytesSent == SOCKET_ERROR) {
            std::cerr << "Send failed with error: " << WSAGetLastError() << std::endl;
            break; // Exit loop on error
        } else if (bytesSent != sizeof(Data)) {
            std::cerr << "Warning: not all bytes were sent. Sent " << bytesSent << " of " << sizeof(Data) << std::endl;
        }

        // Update data for the next packet
        dataToSend.seq++;
        dataToSend.distance += 10;
        dataToSend.voltage += 0.1f;        
        snprintf(dataToSend.text, sizeof(dataToSend.text), "hello from PC #%d", dataToSend.seq);        

        delay(2000); // Wait 2 seconds
    }



    // 5. Cleanup
    std::cout << "Closing connection." << std::endl;
    closesocket(connectSocket);
    WSACleanup();

    return 0;
}
