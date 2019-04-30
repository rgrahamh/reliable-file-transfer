#include "../shared/shared.c"

int handleRequest(int sockfd, char* resp, int bytes_recv, struct sockaddr * serv_addr, struct sockaddr * client_addr, socklen_t* client_addr_len) {
    if((unsigned char)resp[0] != HANDSHAKE_BIT){
        printf("Invalid handshake request.\n");
        #ifdef TESTING
        printf("Flags: %d\n", resp[0]);
        #endif
        return 1;
    }

    //Initialize/clear buffers
    char* buff = malloc(MAX_BUFF_SIZE);
    memset(buff, 0, MAX_BUFF_SIZE);
    memset(resp, 0, MAX_BUFF_SIZE);

    //Set flags
    char flags = HANDSHAKE_BIT;
    buff[0] = flags;

    //Send response to handshake
    resp = sendRTP(sockfd, buff, 1, client_addr, resp, MAX_BUFF_SIZE, serv_addr);
    if(resp == NULL){
        printf("Server timeout; exiting");
    }
    
    printf("Path: %s\n", buff);
    return 0;
}

int main(int argc, char** argv){
    //Defining a variable to hold the command line argument
    char* listening_port;

    //Catch improper argument number
    if(argc != 2){
        printf("Unrecognized argument. Please use the following format:\n./server <listening_port>\n");
        return 1;
    }

    //Allocate space for the command line argument
    listening_port = (char*)malloc(strlen(argv[1]) + 1);

    //Copy the command line argument into a variable
    strcpy(listening_port, argv[1]);

    #ifdef TESTING
    printf("Listening on port %s\n", listening_port);
    #endif

    struct addrinfo hints, *serv_info;

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, listening_port, &hints, &serv_info);

    //Store buffers (on recieving socket)
    char buff[MAX_BUFF_SIZE];
    
    //Clear buffer
    memset(buff, 0, MAX_BUFF_SIZE);

    //Initialize the socket
    int sockfd = socket(serv_info->ai_family, serv_info->ai_socktype, serv_info->ai_protocol);

    //Bind the socket to a port
    bind(sockfd, serv_info->ai_addr, serv_info->ai_addrlen);


    int bytes_recv = 0;
    struct sockaddr from_addr;
    socklen_t from_addr_len = sizeof from_addr;

    //Listening for the initial 
    while(1){
        bytes_recv = recvfrom(sockfd, (void*)buff, MAX_BUFF_SIZE, 0, &from_addr, &from_addr_len);
        if(bytes_recv){
            handleRequest(sockfd, buff, bytes_recv, serv_info->ai_addr, &from_addr, &from_addr_len);
        
            //Clear the buffer
            memset(buff, 0, MAX_BUFF_SIZE);
        }
    }

    //Free the server info
    freeaddrinfo(serv_info);

    close(sockfd);

    return 0;
}
