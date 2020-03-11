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
#include <sys/time.h>

char* server_ip = "192.168.1.10";
bool divide = true;

struct triple
{
    int startNum;
    int num;
    int numOfThread;
    int port;
    int firstFile;
    std::string folder;
};


void sendImage(int sockfd, std::string folder, int startnum, int i) {
    FILE *thisImage;
    int size, read_size;

    // Assemble the file path of the file
    int numofimg = startnum + i;
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
    triple args = *(triple *)arg;

    std::cout << "startNum: " << args.startNum << "\nnum: " << args.num << "\nnumOfThread: " << args.numOfThread << "\n";

    // Create the local socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("Local socket creation failed.\n");
        exit(0);
    }

    // Assign IP and port #
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(server_ip);
    server_address.sin_port = htons(args.port + args.numOfThread + 1);
    std::cout << args.numOfThread << ") Port #: " << args.port + args.numOfThread + 1 << "\n";

    if (connect(sockfd, (struct sockaddr*)&server_address, sizeof(server_address)) != 0) {
        printf("Error number: %d\n", errno);
        printf("The error message is %s\n", strerror(errno));
        printf("Local socket connection with the server failed.\n");
        exit(0);
    }
    std::cout << "Socket " << args.numOfThread << " connected!\n";

    // Send the picture
    for (int j = args.startNum; j < args.num + args.startNum; j++) {
        std::cout << args.numOfThread << ") Sending the " << j << " image\n";
        sendImage(sockfd, args.folder, args.firstFile,j);

        // Check the ack
        if (recv(sockfd, ack, sizeof(ack), 0)) {
            continue;
        }
    }
    
    // Close the socket and return
    close(sockfd);
    return NULL;
}

int main(int argc, char** argv) {
    timeval start, end;
    gettimeofday(&start, NULL);
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

    int port;
    std::cout << "You are connecting to " << server_ip << std::endl;
    std::cout << "Type the port number: ";
    std::cin >> port;
    std::cout << "\n";

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
    std::string jobname = "";
    std::cout << "Type a name for this job: ";
    std::cin >> jobname;
    std::cout << "\n";

    strcpy(buff, jobname.c_str());
    send(sockfd, buff, sizeof(buff), 0);
    
    // Select a folder
    std::string folderpath = "";
    std::cout << "Type the path to the folder: ";
    std::cin >> folderpath;
    std::cout << "\n";

    int firstFile;
    std::cout << "Type the sequence number of the first file: ";
    std::cin >> firstFile;
    std::cout << "\n";

    // Input the number of files
    int filenum;
    std::cout << "Type the number of images: ";
    std::cin >> filenum;
    
    // Assign the number of files for each thread
    int numOfImages = filenum / 3;
    if (filenum % 3 != 0) {
        divide = false;
    }
    int num[3];
    if (!divide) {
        num[0] = numOfImages + 1;
        num[1] = numOfImages;
        num[2] = filenum - 2 * numOfImages - 1;
    }
    else {
        num[0] = numOfImages;
        num[1] = numOfImages;
        num[2] = numOfImages;
    }

    // Send the numbers
    bzero(buff, sizeof(buff));
    std::string snum = "0;" + std::to_string(num[0]) + ";" + std::to_string(num[1] + num[0]) + ";" + std::to_string(filenum);
    strcpy(buff, snum.c_str());
    send(sockfd, buff, sizeof(buff), 0);
    bzero(buff, sizeof(buff));

    // Assemble the arg
    triple args[3];
    for (int i = 0; i < 3; i++) {
        args[i].numOfThread = i;
        args[i].num = num[i];
        if (i == 0) {
            args[i].startNum = 0;
        }
        else {
            args[i].startNum = args[i-1].startNum + args[i-1].num;
        }
        args[i].port = port;
        args[i].folder = folderpath;
        args[i].firstFile = firstFile;
    }

    // Send the photos
    pthread_t tid[3];
    for (int i = 0; i < 3; i++) {
        if (recv(sockfd, ack, sizeof(ack), 0)) {
            if ((pthread_create(&tid[i], NULL, clientThread, (void*)(args + i)) < 0)) {
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
    gettimeofday(&end, NULL);

    long seconds = (end.tv_sec - start.tv_sec);
    long micro = ((seconds * 1000000) + end.tv_usec) - (start.tv_usec);
    std::cout << "The program is done.\n The time used is " << seconds <<"s and " << micro << " microseconds.\n";
    return 0;
}