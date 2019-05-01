#include "../shared/shared.c"
#include <error.h>

#define OUR_PORT "8080"

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

    //Stores the buffer
    char* buff = malloc(MAX_BUFF_SIZE);
    char* resp = malloc(MAX_BUFF_SIZE);
    memset(resp, 0, MAX_BUFF_SIZE);
    memset(resp, 0, MAX_BUFF_SIZE);

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

    //Send the initial handshake to the server
    char flags = HANDSHAKE_BIT;
    buff[0] = flags;

    //Tells us how many bytes were received by sendRTP
    int bytes_recv;

    //Initiate the handshake
    resp = sendRTP(sockfd, buff, 1, (struct sockaddr *)&server_info, resp, MAX_BUFF_SIZE, client_info->ai_addr, &bytes_recv, 1);
    if(resp == NULL){
        printf("Could not initiate handshake; exiting.\n");
        return 2;
    }

    //Send the request for the file
    strcat(buff, remote_path);
    flags = HANDSHAKE_BIT | ACK_BIT;
    buff[0] = flags;
    resp = sendRTP(sockfd, buff, strlen(buff), (struct sockaddr *)&server_info, resp, MAX_BUFF_SIZE, client_info->ai_addr, &bytes_recv, 1);
    if(resp == NULL){
        printf("Server timeout; exiting.\n");
        return 2;
    }
    
    //Start recieving the file
    if((resp[0] & HANDSHAKE_BIT) && (resp[0] & ACK_BIT) && (resp[0] & SEQ_BIT)){
        printf("Cannot find the file '%s' on the server.\n", remote_path);
        return 3;
    }
    int file_size = bytes_recv - 1;
    char* file_contents = malloc(file_size);
    int file_offset = 0;
    int seq = resp[0] | SEQ_BIT;
    flags = seq | ACK_BIT;
    while(!(resp[0] & LAST_BIT)){
        //Copy over file contents
        for(int i = 0; i < bytes_recv - 1; i++){
            file_contents[i+file_offset] = resp[i+1];
        }
        
        //Look for new packets while the sequence bit doesn't match the current packet in the sequence
        resp = sendRTP(sockfd, &flags, 1, (struct sockaddr *)&server_info, resp, MAX_BUFF_SIZE, client_info->ai_addr, &bytes_recv, 1);

        //Update values based upon the new response
        file_offset = file_size;
        file_size += bytes_recv - 1;
        realloc(file_contents, file_size);
        if(seq){
            seq = 0;
        } else {
            seq = SEQ_BIT;
        }
        flags = seq | ACK_BIT;
    }

    FILE* file = fopen(local_path, "w");
    if(!file){
        printf("Could not open the file for writing.\n");
        return 3;
    }

    for(int i = 0; i < file_size; i++){
        fprintf(file, "%c", file_contents[i]);
    }

    fclose(file);

    //Free the address info
    freeaddrinfo(client_info);

    //Close the socket
    close(sockfd);

    return 0;
}
