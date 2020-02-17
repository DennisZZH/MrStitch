#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h> 


void sendImage(int sockfd) {
    FILE *thisImage;
    int size, read_size;
    char filename[] = "GrandCanyon/PIC_0042.JPG";

    thisImage = fopen(filename, "rb");
    if (thisImage == NULL) {
        std::cout << "Failed to open the image: " << filename << "\n";
        exit(0);
    }

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
    char read_buff[10240];

    // Create the socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("Socket creation failed.\n");
        exit(0);
    }

    char* server_ip = argv[1];
    int port = 8080;

    // Assign IP and port #
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(server_ip);
    server_address.sin_port = htons(port);

    if (connect(sockfd, (struct sockaddr*)&server_address, sizeof(server_address)) != 0) {
        printf("Connection with the server failed.\n");
        exit(0);
    }

    // // Select a folder
    // std::string folderpath;
    // std::cout << "Type the path to the folder: ";
    // std::cin >> folderpath;


    // Send the photos
    sendImage(sockfd);

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
    // outputImage = fopen("output.jpg", "w");
    // fwrite(read_buff, sizeof(char), sizeof(read_buff),outputImage);
    // fclose(outputImage);

    // Close the socket
    close(sockfd);
    return 0;
}