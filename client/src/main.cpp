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
const auto global_start = std::chrono::high_resolution_clock::now();

int sock_fd;
struct sockaddr_in server;
int64_t interval_duration;
int attempts;

void* reciever(void*) {
    std::cout << "interval,rtt\n";

    char serverReply[MAX_STRING_LENGTH + 1];
    int len = 0;

    for (int i = 0; i < attempts; i++) {
        ssize_t recvfrom_code = recvfrom(sock_fd, (void*) serverReply, (size_t) (MAX_STRING_LENGTH + 1), MSG_WAITALL, (struct sockaddr *) &server, (socklen_t*) &len);
        if (recvfrom_code == -1)
        {
            std::cerr << "recvfrom error\n"; 
            continue;
        }

        auto finish = std::chrono::high_resolution_clock::now();
        auto duration_finish = std::chrono::duration_cast<std::chrono::nanoseconds>(finish - global_start).count();
        auto duration_start = strtoll(serverReply, nullptr, 10);
        auto duration = duration_finish - duration_start;
        std::cout<< interval_duration << "," << duration << "\n";
    }

    close(sock_fd);
    exit(0);
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        std::cerr << "not enough arguments\n";
        exit(-1);
    }
  
    int ip = std::atoi(argv[1]);
    int port = std::atoi(argv[2]);
    if (argc == 3) {
        interval_duration = -1;
        attempts = 1;
    } else if (argc == 4) {
        interval_duration = strtoll(argv[3], nullptr, 10);
        attempts = 1000000000;
    } else if (argc == 5) {
        interval_duration = strtoll(argv[3], nullptr, 10);
        attempts = std::atoi(argv[4]);
    } else if (argc > 5) {
        std::cerr << "socket creation failed\n"; 
        std::exit(1); 
    }

    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd == -1) {
        std::cerr << "socket creation failed\n"; 
        std::exit(1); 
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = INADDR_ANY;
  
    int len = 0;

    int tid=0;
    if (pthread_create((pthread_t*) &tid, NULL, reciever, NULL)==-1)
    {
        perror("pthread_create error");
        close(sock_fd);
        exit(-1);
    }

    for (int i = 0; i < attempts; i++) {
        auto start = std::chrono::high_resolution_clock::now();
        auto duration_start = std::chrono::duration_cast<std::chrono::nanoseconds>(start - global_start).count();
        auto messageBuffer = std::to_string(duration_start).c_str();

        ssize_t sendto_code = sendto(sock_fd, (void*) messageBuffer, (size_t) strlen(messageBuffer) + 1, 0, (struct sockaddr *) &server, sizeof(server));
        if (sendto_code == -1)
        {
            std::cerr << "sendto error\n"; 
            close(sock_fd);
            exit(1);
        } 

        if (interval_duration > 0) {
            std::this_thread::sleep_for(std::chrono::nanoseconds(interval_duration));
        }
    }

    while (true) {}
}
