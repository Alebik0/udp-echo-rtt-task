#include <iostream>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>

const int MAX_STRING_LENGTH = 1024;

int main(int argc, char *argv[]) {
    if (argc == 1) {
        std::cout << "No port provided\n";
        std::exit(-1);
    }
    
    int port = std::atoi(argv[1]);
    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd == -1) {
        std::cerr << "socket creation failed\n"; 
        std::exit(1); 
    }
    
    struct sockaddr_in server, client;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = INADDR_ANY;

    int bind_code = bind(sock_fd, (struct sockaddr*) &server, (socklen_t) sizeof(server));
    if (bind_code == -1) {
        std::cerr << "bind failed\n"; 
        close(sock_fd);
        exit(-1);
    }


    std::cout << "Listening from port " << port << std::endl;
    char message_buffer[MAX_STRING_LENGTH + 1];
    int counter = 0;

    while (true) {
        int len = sizeof(client);
        ssize_t recvfrom_code = recvfrom(sock_fd, (void*) message_buffer, (size_t) (MAX_STRING_LENGTH + 1), MSG_WAITALL, (struct sockaddr*) &client, (socklen_t*) &len);
        if (recvfrom_code == -1) {
            std::cerr << "recvfrom error\n"; 
            continue;
        } 

        ssize_t sendto_code = sendto(sock_fd, (void*) message_buffer, (size_t) strlen(message_buffer) + 1, 0, (struct sockaddr*) &client, (socklen_t) len);
        if (sendto_code == -1) {
            std::cerr << "sendto error\n"; 
            continue;
        }

        std::cout << counter++ << ") Recieved message from " << inet_ntoa(client.sin_addr) << ": " << message_buffer << std::endl;
    }

    close(sock_fd);
    exit(0);
}
