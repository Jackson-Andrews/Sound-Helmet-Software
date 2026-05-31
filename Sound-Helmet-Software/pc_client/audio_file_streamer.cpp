#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

// Platform-specific includes
#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")  // Link Winsock library
typedef SOCKET socket_t;
#define close_socket closesocket
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
typedef int socket_t;
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define close_socket close
#endif

const int FILE_BUFFER_SIZE = 4096;       // chunk size for reading mp3
const char* ESP32_IP = "10.168.157.30";  // IP of ESP32
const int ESP32_PORT = 10000;            // Port to connect to

int main() {
#if defined(_WIN32)
    // Initialize Windows sockets.
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed.\n";
        return 1;
    }
#endif

    // open mp3 file in binary mode
    std::ifstream mp3File("music.mp3", std::ios::binary);
    if (!mp3File.is_open()) {
        std::cerr << "Error: Could not open music.mp3.\n";
#if defined(_WIN32)
        WSACleanup();
#endif
        return 1;
    }

    // create a TCP socket
    socket_t sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed.\n";
        mp3File.close();
#if defined(_WIN32)
        WSACleanup();
#endif
        return 1;
    }

    // fill in server (ESP32) address info
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;          // use IPv4
    serverAddr.sin_port = htons(ESP32_PORT);  // convert port to network order
    inet_pton(AF_INET, ESP32_IP,
              &serverAddr.sin_addr);  // convert IP string to binary

    // try to connect to ESP32
    std::cout << "Connecting to ESP32...\n";
    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) ==
        SOCKET_ERROR) {
        std::cerr << "Connection failed.\n";
        close_socket(sock);
        mp3File.close();
#if defined(_WIN32)
        WSACleanup();
#endif
        return 1;
    }

    std::cout << "Connected to ESP32!\n";

    std::vector<char> buffer(FILE_BUFFER_SIZE);  // create buffer for chunks
    char ackBuffer[10];                          // buffer to hold ESP32's "OK"

    // read file and send to ESP32 in chunks
    while (!mp3File.eof()) {
        std::cout << "Sending chunk...\n";
        mp3File.read(buffer.data(), buffer.size());  // read next chunk
        std::streamsize bytesRead =
            mp3File.gcount();  // how many bytes were read

        if (bytesRead > 0) {
            // send chunk size first
            uint32_t netSize = htonl(static_cast<uint32_t>(
                bytesRead));  // convert to network byte order
            if (send(sock, reinterpret_cast<char*>(&netSize), sizeof(netSize),
                     0) != sizeof(netSize)) {
                std::cerr << "Failed to send chunk size.\n";
                break;
            }

            // send actual chunk data
            if (send(sock, buffer.data(), bytesRead, 0) != bytesRead) {
                std::cerr << "Failed to send data chunk.\n";
                break;
            }

            // wait for ESP32 to say "OK"
            int bytesReceived = recv(sock, ackBuffer, sizeof(ackBuffer) - 1, 0);
            if (bytesReceived > 0) {
                ackBuffer[bytesReceived] = '\0';  // null-terminate it
                if (strcmp(ackBuffer, "OK") != 0) {
                    std::cerr << "Unexpected ACK: " << ackBuffer << "\n";
                    break;
                }
            } else {
                std::cerr << "No ACK received or connection closed.\n";
                break;
            }
        }
    }

    std::cout << "Finished streaming audio.\n";

    // clean up
    mp3File.close();
    close_socket(sock);
#if defined(_WIN32)
    WSACleanup();
#endif

    std::cout << "Connection closed.\n";
    return 0;
}
