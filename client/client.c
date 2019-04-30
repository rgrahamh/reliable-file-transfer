#include "../shared/shared.c"
#define OUR_PORT "8080"
#include <error.h>

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
    printf("Server Port: %s\n", server_port);
    printf("Remote Path: %s\n", remote_path);
    printf("Local Path: %s\n", local_path);
    #endif

    //Stores the buffer
    char* buff = "This message should be sent to the server!";
    char* resp = malloc(MAX_IN_BUFF_SIZE);
    memset(resp, 0, MAX_IN_BUFF_SIZE);

    //Setting up our address information
    struct addrinfo hints, *client_info;

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, OUR_PORT, &hints, &client_info);

    //Initialize the socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    //Binding the socket to a port
    bind(sockfd, client_info->ai_addr, client_info->ai_addrlen);

    //Set it to timeout after eight seconds
    struct timeval tval;
    tval.tv_sec = 8;
    tval.tv_usec = 0;
    if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tval, sizeof tval) < 0){
        perror("Error");
    }

    //Initialize the server information
    struct sockaddr_in server_info;
    server_info.sin_family = AF_UNSPEC;
    server_info.sin_port = htons(atoi(server_port));
    server_info.sin_addr.s_addr = inet_addr(server_ip);

    //Send the built buffer to the server
    sendRTP(sockfd, buff, (size_t)strlen(buff), (struct sockaddr *)&server_info, resp, MAX_IN_BUFF_SIZE, client_info->ai_addr);

    //Free the address info
    freeaddrinfo(client_info);

    //Close the socket
    close(sockfd);

    return 0;
}
