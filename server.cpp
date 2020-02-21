//Author: Dennis
#include <unistd.h> 
#include <stdio.h>
#include <sys/types.h>  
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#define PORT 8222

#include <cstdlib>
#include <iostream>
#include "stitching.cpp"
#include <pthread.h>

static char jobname[50];
static  int num = 0;

int recv_imgs_from_client(int sock, int num){

    printf("Reading image size!\n");
    int siz = 0;
    char buf[50];
    if ((recv(sock, buf, sizeof(buf), 0) <0)){
        perror("recv_size()");
        exit(errno);
    }
    siz = atoi(buf);
   printf("size = %d", siz);

    char Rbuffer[1024];
    printf("Reading image byte array!\n");
    printf("Converting byte array to image!\n");
    int n = 0;
    int acc = siz;

    FILE *image;
    std::string ext = ".jpg";
    std::string imgname = std::string(jobname) + std::to_string(num) + ext;
    image = std::fopen(imgname.c_str(), "wb");

    while(acc > 0){
        if ((n = recv(sock, Rbuffer, 1024, 0)) < 0){
            perror("recv_size()");
            exit(errno);
        }
        fwrite(Rbuffer, sizeof(char), n, image);
        acc -= n;
        printf("buffer size = %d\n", n); 
    }

    fclose(image);
    printf("done\n");

    return 0;
}

int send_imgs_to_client(int sock){
    FILE *thisImage;
    int size, read_size;

    std::string ext = ".jpg";
    std::string filename = std::string(jobname) + "_result" + ext;

    // Open the file
    thisImage = fopen(filename.c_str(), "rb");
    if (thisImage == NULL) {
        std::cout << "Failed to open the image: " << filename << "\n";
        exit(0);
    }

    // Get the size of the file
    fseek(thisImage, 0, SEEK_END);
    size = ftell(thisImage);
    std::cout <<"result size = "<< size << std::endl;
    fseek(thisImage, 0, SEEK_SET);
    
    // Copy the value to buff
    char buff[50];
    sprintf(buff, "%d", size);

    // Send image size
    send(sock, buff, sizeof(buff), 0);

    char buffer[1024];
    int acc = 0;
    while (!feof(thisImage)) {
        // Read from the file
        read_size = fread(buffer, 1, sizeof(buffer), thisImage);
        std::cout <<"result buffer size = "<<read_size << std::endl;
        // Send data through the socket
        if (read_size > 0) {
            if (send(sock, buffer, read_size, 0) < 0) {
                std::cout << "Failed to send the data!\n";
                exit(0);
            }
        }
        acc+=read_size;
        // Empty the buffer
        bzero(buffer, sizeof(buffer));
    }
    printf("send size = %d\n", acc);
    fclose(thisImage);
    return 0;
}

void *socket_thread_func(void *num){
    int seq = *((int*)num);
    int thread_fd, thread_socket, valread; 
    struct sockaddr_in address; 
    int opt = 1; 
    int addrlen = sizeof(address);
    char ACK[2] = "1"; 
    int portnum = PORT + seq + 1;
    // Creating socket file descriptor 
    printf("thread%d portnum = %d\n", seq, portnum);
    if ((thread_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    { 
        perror("socket failed!\n"); 
        exit(EXIT_FAILURE); 
    }    
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons(portnum); 
       
    if(bind(thread_fd, (struct sockaddr *)&address, sizeof(address)) != 0){
        perror("bind failed!\n"); 
        exit(EXIT_FAILURE); 
    }
    if (listen(thread_fd, 3) < 0) 
    { 
        perror("listen failed!\n"); 
        exit(EXIT_FAILURE); 
    } 
    if ((thread_socket = accept(thread_fd, (struct sockaddr *)&address,  
                       (socklen_t*)&addrlen))<0) 
    { 
        perror("accept failed!\n"); 
        exit(EXIT_FAILURE); 
    }
    // Receive image
    recv_imgs_from_client(thread_socket, seq);
    // Send ACK 
    if(send(thread_socket, ACK, sizeof(ACK), 0) < 0){
        std::cout << "Failed to send the ACK!\n";
        exit(errno);
    }
    // After chatting close the socket 
    close(thread_fd);
    return NULL;
}

int main(int argc, char const *argv[]) 
{ 
  int server_fd, main_socket, valread; 
    struct sockaddr_in address; 
    int opt = 1; 
    int addrlen = sizeof(address);
    char ACK[2] = "1"; 
       
    // Creating socket file descriptor 
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    { 
        perror("socket failed!\n"); 
        exit(EXIT_FAILURE); 
    } 
       
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons( PORT ); 
       
    if(bind(server_fd, (struct sockaddr *)&address, sizeof(address)) != 0){
        perror("bind failed!\n"); 
        exit(EXIT_FAILURE); 
    }

    if (listen(server_fd, 3) < 0) 
    { 
        perror("listen failed!\n"); 
        exit(EXIT_FAILURE); 
    } 
    if ((main_socket = accept(server_fd, (struct sockaddr *)&address,  
                       (socklen_t*)&addrlen))<0) 
    { 
        perror("accept failed!\n"); 
        exit(EXIT_FAILURE); 
    } 

    // Job name
    printf("Reading job name!\n");
    if ((recv(main_socket, jobname, sizeof(jobname), 0) <0)){
        perror("Error reading job name!\n");
        exit(errno);
    }
    printf("jobname = %s\n", jobname);

    // How many numbers of imgs to stitch
    printf("Reading image number!\n");
    char buf[50];
    if ((recv(main_socket, buf, sizeof(buf), 0) <0)){
        perror("Error reading image num!\n");
        exit(errno);
    }
    num = atoi(buf);
    printf("img num = %d\n",num);

  
    // Function for receiving image data from client
    pthread_t thread_ids[num];
    int seq[num];
    for(int i = 0; i < num; i++){
        seq[i] = i;
        if(pthread_create((thread_ids + i), NULL, socket_thread_func, (void*) (seq+i)) < 0){
            perror("could not create thread");
            exit(errno);
        }
        // Send ACK 
        if(send(main_socket, ACK, sizeof(ACK), 0) < 0){
            std::cout << "Failed to send the ACK!\n";
            exit(errno);
        }
    }

    for(int i = 0; i < num; i++){
        pthread_join(thread_ids[i], NULL);
    }

    stitch_imgs(num, jobname);
    std::cout<<"finish stitching!"<<std::endl;
    
    send_imgs_to_client(main_socket);
    std::cout<<"finish sending back result!"<<std::endl;

    // Wait to receive ACK
    std::cout<<"waiting ACK of result!"<<std::endl;
    if ((recv(main_socket, ACK, sizeof(ACK), 0) <0)){
        perror("Error Receiving ACK!\n");
        exit(errno);
    }
    // After chatting close the socket 
    close(server_fd); 
    return 0; 
}