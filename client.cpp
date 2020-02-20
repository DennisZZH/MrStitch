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
    std::cout << size << std::endl;
    fseek(thisImage, 0, SEEK_SET);
    
    // Copy the value to buff
    char buff[50];
    sprintf(buff, "%d", size);

    // Send image size
    send(sockfd, buff, sizeof(buff), 0);

    char buffer[1024];

    while (!feof(thisImage)) {
        // Read from the file
        read_size = fread(buffer, 1, sizeof(buffer), thisImage);
        std::cout << read_size << "\n";
        // Send data through the socket
        if (read_size > 0) {
            if (send(sockfd, buffer, read_size, 0) < 0) {
                std::cout << "Failed to send the data.\n";
                exit(0);
            }
        }

        // Empty the buffer
        bzero(buffer, sizeof(buffer));
    }
    fclose(thisImage);
}

int main(int argc, char** argv) {

    struct sockaddr_in server_address;
    char buff[50];
    char read_buff[10240];

    // Create the socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("Socket creation failed.\n");
        exit(0);
    }

    char* server_ip = argv[1];
    std::cout << server_ip << std::endl;
    int port = 10000;

    // Assign IP and port #
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(server_ip);
    server_address.sin_port = htons(port);

    if (connect(sockfd, (struct sockaddr*)&server_address, sizeof(server_address)) != 0) {
        printf("Error number: %d\n", errno);
        printf("The error message is %s\n", strerror(errno));
        printf("Connection with the server failed.\n");
        exit(0);
    }

    // Declare and send the job name
    std::string jobname;
    std::cout << "Type a name for this job: ";
    std::cin >> jobname;
    strcpy(buff, jobname.c_str());
    send(sockfd, buff, sizeof(buff), 0);
    
    // Select a folder
    std::string folderpath;
    std::cout << "Type the path to the folder: ";
    std::cin >> folderpath;

    // Input the number of files
    int filenum = 2;
    // std::cout << "Type the number of images: ";
    // std::cin >> filenum;

    sprintf(buff, "%d", filenum);
    send(sockfd, buff, sizeof(buff), 0);
    
    // Send the photos
    for (int i = 0; i < filenum; i ++) {
        sendImage(sockfd, folderpath, i);
    }

    // // Empty the buffer
    // bzero(read_buff, sizeof(read_buff));

    // // REceive the panorama size
    // int read_size = 0;
    // if ((read_size = read(sockfd, read_buff, sizeof(read_buff))) < 0) {

    // }
    // size_t size = atoi(read_buff);
    // bzero(read_buff, sizeof(read_buff));

    // // Receive the panorama image
    // if ((read_size = read(sockfd, read_buff, sizeof(read_buff))) < 0) {
    //     std::cout << "Error when receiving.\n";
    //     exit(0);
    // }

    // // Store the panorama
    // FILE *outputImage;
    // outputImage = fopen("output.jpg", "wb");
    // fwrite(read_buff, sizeof(char), sizeof(read_buff),outputImage);
    // fclose(outputImage);

    // Close the socket
    close(sockfd);
    return 0;
}