#include <iostream>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include <chrono>
#include <thread>

const int MAX_STRING_LENGTH = 1024;

int main(int argc, char *argv[])
{
    if (argc < 3) {
        std::cerr << "not enough arguments\n";
        exit(-1);
    }
  
    int ip = std::atoi(argv[1]);
    int port = std::atoi(argv[2]);
    int64_t interval_duration = -1;
    int attempts = 1e9;
    if (argc == 4) {
        interval_duration = strtoll(argv[3], nullptr, 10);
    } else if (argc == 5) {
        interval_duration = strtoll(argv[3], nullptr, 10);
        attempts = std::atoi(argv[4]);
    } else if (argc > 5) {
        std::cerr << "socket creation failed\n"; 
        std::exit(1); 
    }

    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd == -1) {
        std::cerr << "socket creation failed\n"; 
        std::exit(1); 
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = INADDR_ANY;
  
    char messageBuffer[MAX_STRING_LENGTH + 1] = "Message from client;";
    char serverReply[MAX_STRING_LENGTH + 1] = "";
    int len = 0;

    std::cout << "interval,rtt\n";

    for (int i = 0; i < attempts; i++) {
        auto start = std::chrono::high_resolution_clock::now();

        ssize_t sendto_code = sendto(sock_fd, (void*) messageBuffer, (size_t) strlen(messageBuffer) + 1, 0, (struct sockaddr *) &server, sizeof(server));
        if (sendto_code == -1)
        {
            std::cerr << "sendto error\n"; 
            close(sock_fd);
            exit(1);
        } 

        ssize_t recvfrom_code = recvfrom(sock_fd, (void*) serverReply, (size_t) (MAX_STRING_LENGTH + 1), MSG_WAITALL, (struct sockaddr *) &server, (socklen_t*) &len);
        if (recvfrom_code == -1)
        {
            std::cerr << "recvfrom error\n"; 
            close(sock_fd);
            exit(1);
        }

        auto finish = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(finish-start).count();
        std::cout<< interval_duration << "," << duration << "\n";
        
        if (interval_duration > 0) {
            std::this_thread::sleep_for(std::chrono::nanoseconds(interval_duration));
        } else {
            break;
        }
    }
    
    close(sock_fd);
    exit(0);
}
