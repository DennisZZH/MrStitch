// Author: Shiheng Wang
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h> 
#include <errno.h>

char* server_ip = "169.231.164.67";
int port = 8222;
std::string folder = "GrandCanyon";

void sendImage(int sockfd, std::string folder, int i) {
    FILE *thisImage;
    int size, read_size;

    // Assemble the file path of the file
    int numofimg = 42 + i;
    std::string filepath = folder + "/PIC_00" + std::to_string(numofimg) + ".JPG";

    // Copy the path to a c_string
    char filename[filepath.length() + 1];
    strcpy(filename, filepath.c_str());

    // Open the file
    thisImage = fopen(filename, "rb");
    if (thisImage == NULL) {
        std::cout << "Failed to open the image: " << filename << "\n";
        exit(0);
    }

    // Get the size of the file
    fseek(thisImage, 0, SEEK_END);
    size = ftell(thisImage);
    std::cout << "The sending size is " << size << std::endl;
    fseek(thisImage, 0, SEEK_SET);
    
    // Copy the value to buff
    char buff[50];
    sprintf(buff, "%d", size);

    // Send image size
    send(sockfd, buff, sizeof(buff), 0);

    char Sbuffer[1024];

    while (!feof(thisImage)) {
        // Read from the file
        read_size = fread(Sbuffer, 1, sizeof(Sbuffer), thisImage);
        // std::cout << read_size << "\n";
        // Send data through the socket
        if (read_size > 0) {
            if (send(sockfd, Sbuffer, read_size, 0) < 0) {
                std::cout << "Failed to send the data.\n";
                exit(0);
            }
        }

        // Empty the buffer
        bzero(Sbuffer, sizeof(Sbuffer));
    }
    fclose(thisImage);
}

void *clientThread(void* arg) {
    struct sockaddr_in server_address;
    char ack[2];
    int i = *(int *)arg;

    // Create the local socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("Local socket creation failed.\n");
        exit(0);
    }

    // Assign IP and port #
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(server_ip);
    server_address.sin_port = htons(port + i + 1);
    std::cout << "Port #: " << port + i + 1 << std::endl;

    if (connect(sockfd, (struct sockaddr*)&server_address, sizeof(server_address)) != 0) {
        printf("Error number: %d\n", errno);
        printf("The error message is %s\n", strerror(errno));
        printf("Local socket connection with the server failed.\n");
        exit(0);
    }

    // Send the picture
    sendImage(sockfd, folder, i);
    
    // Check the ack
    recv(sockfd, ack, sizeof(ack), 0);
    if (ack[0] == '1') {
        std::cout << "The server received " << i << " image.\n Closing the socket.\n";
        close(sockfd);
    }
    else {
        std::cout << "The server does not finish receiving " << i << " image.\n";
    }
    return NULL;
}

int main(int argc, char** argv) {

    struct sockaddr_in server_address;
    char buff[50];
    char read_buff[1024];
    char ack[2];

    // Create the main socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("Main socket creation failed.\n");
        exit(0);
    }

    std::cout << "You are connecting to " << server_ip << std::endl;

    // Assign IP and port #
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(server_ip);
    server_address.sin_port = htons(port);

    if (connect(sockfd, (struct sockaddr*)&server_address, sizeof(server_address)) != 0) {
        printf("Error number: %d\n", errno);
        printf("The error message is %s\n", strerror(errno));
        printf("Main socket connection with the server failed.\n");
        exit(0);
    }

    // Declare and send the job name
    std::string jobname = "GrandCanyon";
    // std::cout << "Type a name for this job: ";
    // std::cin >> jobname;
    strcpy(buff, jobname.c_str());
    send(sockfd, buff, sizeof(buff), 0);
    
    // Select a folder
    // std::string folderpath = "GrandCanyon";
    // std::cout << "Type the path to the folder: ";
    // std::cin >> folderpath;

    // Input the number of files
    int filenum = 2;
    // std::cout << "Type the number of images: ";
    // std::cin >> filenum;

    sprintf(buff, "%d", filenum);
    send(sockfd, buff, sizeof(buff), 0);
    bzero(buff, sizeof(buff));
    
    // Send the photos
    int sequence[filenum];
    pthread_t tid[filenum];
    for (int i = 0; i < filenum; i++) {
        sequence[i] = i;
        if (recv(sockfd, ack, sizeof(ack), 0)) {
            if ((pthread_create(&tid[i], NULL, clientThread, (void*)(sequence + i)) < 0)) {
                std::cout << "Failed to create the " << i << " thread\n";
                exit(0);
            }
        }
    }

    for (int i = 0; i < filenum; i++) {
        pthread_join(tid[i], NULL);
    }

    // Receive the panorama size
    int read_size = 0;
    if ((read_size = recv(sockfd, read_buff, sizeof(read_buff), 0)) < 0) {
        std::cout << "Failed to read the size.\n";
        exit(0);
    }
    size_t size = atoi(read_buff);
    std::cout << "The receiving size is " << size << "\n";
    bzero(read_buff, sizeof(read_buff));

    // Create the result file name
    std::string resultfile = jobname + ".jpg";
    char resultname[resultfile.length() + 1];
    strcpy(resultname, resultfile.c_str());

    // Open a file for the resualt
    FILE *outputImage;
    outputImage = fopen(resultname, "wb");

    ack[0] = '1';
    // Receive the panorama image and store it
    while (size > 0) {
        if ((read_size = recv(sockfd, read_buff, sizeof(read_buff), 0)) <= 0) {
            std::cout << "Failed to read the data.\n";
            exit(0);
        }
        send(sockfd, ack, sizeof(ack), 0);
        
        // std::cout << "Received " << read_size << "\n";
        fwrite(read_buff, sizeof(char), read_size, outputImage);
        size = size - read_size;
        // std::cout << "Left: " << size << "\n";
        bzero(read_buff, sizeof(read_buff));
    }
    // Send ACK back to the server
    send(sockfd, ack, sizeof(ack), 0);
    
    fclose(outputImage);

    // Close the socket
    close(sockfd);
    return 0;
}