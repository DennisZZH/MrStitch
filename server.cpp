//Author: Dennis
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#define PORT 8080 

#include <cstdlib>
#include <iostream>

using namespace std;

int recv_images_from_client(int sock){

    cout<< "Reading image size" <<endl;
    int siz = 0;
    char buf[50];
    if ((recv(sock, buf, sizeof(buf), 0) <0)){
        perror("recv_size()");
        exit(errno);
    }
    siz = atoi(buf);
    // read(sock, &siz, sizeof(int));
    cout << "size = "<<siz << endl; // 880 output

    char Rbuffer[1024];
    cout << "Reading image byte array" << endl;
    cout << "Converting byte array to image" << endl;
    int n = 0;
    int acc = siz;

    FILE *image;
    image = fopen("recu.JPG", "wb");

    while(acc > 0){
        if ((n = recv(sock, Rbuffer, 1024, 0)) < 0){
            perror("recv_size()");
            exit(errno);
        }
        fwrite(Rbuffer, sizeof(char), n, image);
        acc -= n;  
    }

    fclose(image);
    cout << "done" << endl;

    return 0;
}

int main(int argc, char const *argv[]) 
{ 
  int server_fd, new_socket, valread; 
    struct sockaddr_in address; 
    int opt = 1; 
    int addrlen = sizeof(address); 
       
    // Creating socket file descriptor 
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    } 
       
    // // Forcefully attaching socket to the port 8080 
    // if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
    //                                               &opt, sizeof(opt))) 
    // { 
    //     perror("setsockopt"); 
    //     exit(EXIT_FAILURE); 
    // } 
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons( PORT ); 
       
    // Forcefully attaching socket to the port 8080 
    if (bind(server_fd, (struct sockaddr *)&address,  
                                 sizeof(address))<0) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    if (listen(server_fd, 3) < 0) 
    { 
        perror("listen"); 
        exit(EXIT_FAILURE); 
    } 
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,  
                       (socklen_t*)&addrlen))<0) 
    { 
        perror("accept"); 
        exit(EXIT_FAILURE); 
    } 
  
    // Function for chatting between client and server 
    recv_images_from_client(new_socket); 
  
    // After chatting close the socket 
    close(server_fd); 
    return 0; 
} 