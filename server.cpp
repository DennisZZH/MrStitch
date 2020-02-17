//Author: Dennis
#include <unistd.h> 
#include <stdio.h>
#include <sys/types.h>  
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#define PORT 8080 

#include <cstdlib>
#include <iostream>
#include "stitching.cpp"

using namespace std;

int recv_imgs_from_client(int sock, char* jobname, int num){

    cout<< "Reading image size" <<endl;
    int siz = 0;
    char buf[50];
    if ((recv(sock, buf, sizeof(buf), 0) <0)){
        perror("recv_size()");
        exit(errno);
    }
    siz = atoi(buf);
    cout << "size = "<<siz << endl;

    char Rbuffer[1024];
    cout << "Reading image byte array" << endl;
    cout << "Converting byte array to image" << endl;
    int n = 0;
    int acc = siz;

    FILE *image;
    string imgname = string(jobname) + to_string(num);
    image = std::fopen(imgname.c_str(), "wb");

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

int send_imgs_to_client(int sock){
    //STUB
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
       
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons( PORT ); 
       
    // Forcefully attaching socket to the port 8080
    // if ( bind(server_fd, (struct sockaddr *)&address, sizeof(address)) != 0 )
    // { 
    //     perror("bind failed"); 
    //     exit(EXIT_FAILURE); 
    // } 
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));

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

    // Job name
    cout<< "Reading job name" <<endl;
    char jobname[50];
    if ((recv(new_socket, jobname, sizeof(jobname), 0) <0)){
        perror("Error reading job name");
        exit(errno);
    }
    cout << "jobname = "<<jobname<< endl;

    // How many numbers of imgs to stitch
    cout<< "Reading image number" <<endl;
    int num = 0;
    char buf[50];
    if ((recv(new_socket, buf, sizeof(buf), 0) <0)){
        perror("Error reading image num");
        exit(errno);
    }
    num = atoi(buf);
    cout << "img num = "<<num<< endl;

  
    // Function for receiving image data from client
    for(int i = 0; i < num; i++){
        recv_imgs_from_client(new_socket, jobname, i); 
    }

    stitch_imgs(jobname, num);

    send_imgs_to_client(new_socket);

    // After chatting close the socket 
    close(server_fd); 
    return 0; 
} 