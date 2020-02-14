#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h> 


void sendPhotos(int sockfd) {

}

int main(int argc, char** argv) {

    struct sockaddr_in server_address;

    // Create the socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("Socket creation failed.\n");
        exit(0);
    }

    char* server_ip = argv[1];
    int port = atoi(argv[2]);

    // Assign IP and port #
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(server_ip);
    server_address.sin_port = htons(port);

    if (connect(sockfd, (struct sockaddr*)&server_address, sizeof(server_address)) != 0) {
        printf("Connection with the server failed.\n");
        exit(0);
    }

    // Send the photos
    sendPhotos(sockfd);

    // Receive the panorama
    read(sockfd, buff, sizeof(buff));

    // Close the socket
    close(sockfd);
}