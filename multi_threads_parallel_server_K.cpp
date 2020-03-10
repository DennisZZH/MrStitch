//Author: Dennis
#include <unistd.h> 
#include <stdio.h>
#include <sys/types.h>  
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#define PORT 8200
#define NUM_MULTI_THREAD 3
#include <cstdlib>
#include <iostream>
#include "stitching_multi_threads_parallel.cpp"
#include <pthread.h>
#include <sys/time.h>	// for gettimeofday()

static char jobname[50];
static int nums[4];

int recv_imgs_from_client(int sock, int num){
    
    printf("Reading image size!\n");
    printf("Receiving image%d!\n", num);
   
    int siz = 0;
    char buf[50];
    if ((recv(sock, buf, sizeof(buf), 0) <0)){
        perror("recv_size()");
        exit(errno);
    }
    siz = atoi(buf);
   printf("size = %d\n", siz);

    char Rbuffer[1024];
    int n = 0;
    int acc = siz;

    FILE *image;
    std::string ext = ".jpg";
    std::string imgname = std::string(jobname) + std::to_string(num) + ext;
    image = std::fopen(imgname.c_str(), "wb");

    printf("Reading image byte array!\n");
    printf("Converting byte array to image!\n");
    while(acc > 0){
       
        if ((n = recv(sock, Rbuffer, 1024, 0)) < 0){
            perror("recv_size()");
            exit(errno);
        }
        fwrite(Rbuffer, sizeof(char), n, image);
        acc -= n;
        //printf("buffer size = %d\n", n); 
    }

    fclose(image);
    //printf("Done!\n");

    return 0;
}

int send_imgs_to_client(int sock){
    FILE *thisImage;
    int size, read_size;
    char ACK[2];

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
        //std::cout <<"result buffer size = "<<read_size << std::endl;
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
        // Wait for ACK
        if((recv(sock, ACK, 2, 0)) < 0){
            perror("Fail to receive ACK!\n");
            exit(errno);
        }else{
            //printf("Receive ACK for result\n");
        }
    }
    //printf("send size = %d\n", acc);
    fclose(thisImage);
    return 0;
}

void *socket_thread_func(void *args){
    std::string strbuf = *((std::string*) args);
    char* charbuf = strdup(strbuf.c_str());
    int para[4];
    int thread_fd, thread_socket; 
    struct sockaddr_in address; 
    int addrlen = sizeof(address);
    char ACK[2] = "1"; 

    // get args
    int acc = 0;
    char * pch;
    pch = strtok (charbuf, ";");
    while (pch != NULL){
        para[acc] = atoi(pch);
        acc++;
        pch = strtok (NULL, ";");
    }

    // printf("This is client %d\n", para[0]);
    // printf("This is thread %d!\n", para[1]);
    // for(int i = 0; i < 3; i++){
    //     printf("client%d thread%d args = %d ", para[0], para[1], para[i]);
    // }
    // printf("\n");

    int portnum = PORT + para[0] * 4 + para[1] + 1;
    // Creating socket file descriptor 
    //printf("thread%d portnum = %d\n", para[1], portnum);
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
    for(int i = para[2]; i < para[2] + para[3]; i++){
        recv_imgs_from_client(thread_socket, i);
        // Send ACK 
        if(send(thread_socket, ACK, sizeof(ACK), 0) < 0){
            std::cout << "Failed to send the ACK!\n";
            exit(errno);
        }
    }
    // // Send ACK 
    // if(send(thread_socket, ACK, sizeof(ACK), 0) < 0){
    //     std::cout << "Failed to send the ACK!\n";
    //     exit(errno);
    // }

    // After chatting close the socket 
    close(thread_fd);
    return NULL;
}


void* server_setup(void* args) 
{ 
    int client_num = *((int*)args);
    int portnum = PORT + client_num * 4;
    int server_fd, main_socket; 
    struct sockaddr_in address; 
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
    address.sin_port = htons( portnum ); 
       
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

    // Start timing
    struct timeval start, end;
	gettimeofday(&start, NULL);

    // Job name
    printf("Reading job name!\n");
    if ((recv(main_socket, jobname, sizeof(jobname), 0) <0)){
        perror("Error reading job name!\n");
        exit(errno);
    }
    //printf("jobname = %s\n", jobname);

    // Receive start num of each thread
    printf("Reading image number!\n");
    char buf[50];
    if ((recv(main_socket, buf, sizeof(buf), 0) <0)){
        perror("Error reading image num!\n");
        exit(errno);
    }
    
    char * pch;
    int acc = 0;
    pch = strtok (buf,";");
    while (pch != NULL){
        nums[acc] = atoi(pch);
        acc++;
        pch = strtok (NULL, ";");
    }
//    for(int i = 0; i < 4; i++){
//        printf("image start num%d = %d\n", i, nums[i]);
//    }
  
    // Function for receiving image data from client
    pthread_t thread_ids[3];
    std::string socket_args[3];
    for(int i = 0; i < 3; i++){
        // args = client num, socket num, start img num, img count
        socket_args[i] = std::to_string(client_num) + ";" + std::to_string(i) + ";" + std::to_string(nums[i]) + ";" + std::to_string(nums[i+1] - nums[i]);
        //std::cout<<"main socket_args"<<i<<" = "<<socket_args[i]<<std::endl;
        if(pthread_create(&(thread_ids[i]), NULL, socket_thread_func, (void*) &(socket_args[i])) < 0){
            perror("could not create thread");
            exit(errno);
        }
        // Send ACK 
        if(send(main_socket, ACK, sizeof(ACK), 0) < 0){
            std::cout << "Failed to send the ACK!\n";
            exit(errno);
        }
    }

    for(int i = 0; i < 3; i++){
        pthread_join(thread_ids[i], NULL);
    }

    stitch_imgs(client_num, nums[3], jobname);
    std::cout<<"finish stitching!"<<std::endl;

    send_imgs_to_client(main_socket);
    std::cout<<"finish sending back result!"<<std::endl;

    // Wait to receive ACK
    //std::cout<<"waiting ACK of result!"<<std::endl;
    if ((recv(main_socket, ACK, sizeof(ACK), 0) <0)){
        perror("Error Receiving ACK!\n");
        exit(errno);
    }
    // After chatting close the socket 
    close(server_fd); 

    // Finish timing
    gettimeofday(&end, NULL);

	long seconds = (end.tv_sec - start.tv_sec);
	long micros = ((seconds * 1000000) + end.tv_usec) - (start.tv_usec);

	printf("****************Client%d time elpased is %ld micro second*****************\n", client_num, micros);

    return NULL;

}



int main(){
    pthread_t tids[3];
    int clients_num[3];
    for(int i = 0; i < NUM_MULTI_THREAD; i++){
        clients_num[i] = i;
        pthread_create(&(tids[i]), NULL, server_setup, &(clients_num[i]));
    }
    for(int i = 0; i < NUM_MULTI_THREAD; i++){
        pthread_join(tids[i], NULL);
    }
    return 0;
}