#include <arpa/inet.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char** argv){
    //Defining variables to hold command line arguments
    char *server_ip, *server_port, *remote_path, *local_path;

    //Catch improper argument number
    if(argc != 5){
        printf("Unrecognized arguments. Please use the following format:\n./client <server_ip> <server_port> <remote_path> <local_path>\n");
        return 1;
    }

    //Allocate space for command line arguments
    server_ip = (char*)malloc(strlen(argv[1]) + 1);
    server_port = (char*)malloc(strlen(argv[2]) + 1);
    remote_path = (char*)malloc(strlen(argv[3]) + 1);
    local_path = (char*)malloc(strlen(argv[4]) + 1);

    //Copy command line arguments into variables
    strcpy(server_ip, argv[1]);
    strcpy(server_port, argv[2]);
    strcpy(remote_path, argv[3]);
    strcpy(local_path, argv[4]);

    //Printing command line arguments for proof
    #ifdef TESTING
    printf("Server IP: %s\n", server_ip);
    printf("Server Path: %s\n", server_port);
    printf("Remote Path: %s\n", remote_path);
    printf("Local Path: %s\n", local_path);
    #endif

    //Stores the buffer
    char* buff = "This message should be sent to the server!";

    //Initialize the socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    //Initialize the server information
    struct sockaddr_in server_info;
    server_info.sin_family = AF_UNSPEC;
    server_info.sin_port = htons(atoi(server_port));
    server_info.sin_addr.s_addr = inet_addr(server_ip);

    //Send the built buffer to the server
    sendto(sockfd, buff, (size_t)strlen(buff), 0, (const struct sockaddr *)&server_info, (socklen_t)sizeof server_info);

    //Close the socket
    close(sockfd);

    return 0;
}
