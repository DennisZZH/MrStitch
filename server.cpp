//Author: Dennis
#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#define MAX 80 
#define PORT 8080 
#define SA struct sockaddr 

#include <cstdlib>
#include <iostream>

using namespace std;

int recv_images_from_client(int sock){
    int n = 0;

    cout<< "Reading image size" <<endl;
    char buf[50];
    int siz = 0;
    if ((n = recv(sock, buf, sizeof(buf), 0) <0)){
        perror("recv_size()");
        exit(errno);
    }
    siz = atoi(buf);
    cout << siz << endl; // 880 output

    char Rbuffer[siz];
    cout << "Reading image byte array" << endl;
    n = 0;
    if ((n = recv(sock, Rbuffer, sizeof(Rbuffer), 0)) < 0){
        perror("recv_size()");
        exit(errno);
    }

    cout << "Converting byte array to image" << endl;
    FILE *image;
    image = fopen("recu.jpg", "w");
    fwrite(Rbuffer, sizeof(char), siz, image);
    fclose(image);
    cout << "done" << endl;

    return 0;
}

int main(int argc, char const *argv[]) 
{ 
  int sockfd, connfd, len; 
    struct sockaddr_in servaddr, cli; 
  
    // socket create and verification 
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        printf("socket creation failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully created..\n"); 
    bzero(&servaddr, sizeof(servaddr)); 
  
    // assign IP, PORT 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(PORT); 
  
    // Binding newly created socket to given IP and verification 
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
        printf("socket bind failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully binded..\n"); 
  
    // Now server is ready to listen and verification 
    if ((listen(sockfd, 5)) != 0) { 
        printf("Listen failed...\n"); 
        exit(0); 
    } 
    else
        printf("Server listening..\n"); 
    len = sizeof(cli); 
  
    // Accept the data packet from client and verification 
    connfd = accept(sockfd, (SA*)&cli, &len); 
    if (connfd < 0) { 
        printf("server acccept failed...\n"); 
        exit(0); 
    } 
    else
        printf("server acccept the client...\n"); 
  
    // Function for chatting between client and server 
    recv_images_from_client(connfd); 
  
    // After chatting close the socket 
    close(sockfd); 
    return 0; 
} 