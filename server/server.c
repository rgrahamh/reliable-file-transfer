#include "../shared/shared.c"

void resetBuffers(char* buff, char* resp){
    memset(buff, 0, MAX_BUFF_SIZE);
    memset(resp, 0, MAX_BUFF_SIZE);
}

int handleRequest(int sockfd, char* resp, struct sockaddr * serv_addr, struct sockaddr * client_addr, socklen_t* client_addr_len) {
    if((unsigned char)resp[0] != HANDSHAKE_BIT){
        printf("Invalid handshake request.\n");
        return 1;
    }

    //Initialize/clear buffers
    char* buff = malloc(MAX_BUFF_SIZE);
    resetBuffers(buff, resp);

    //Keeps track of how many bytes have been received (address of this int is passed to sendRTP)
    int bytes_recv = 0;

    //Set flags
    char flags = HANDSHAKE_BIT;
    buff[0] = flags;

    //Send response to handshake
    resp = sendRTP(sockfd, buff, 1, client_addr, resp, MAX_BUFF_SIZE, serv_addr, &bytes_recv, 1);

    //Until we recieve an ACK flag, we don't know that they've given us the file path.
    while(!(resp[0] | ACK_BIT)){
        resp = sendRTP(sockfd, buff, 1, client_addr, resp, MAX_BUFF_SIZE, serv_addr, &bytes_recv, 1);
    }
    if(resp == NULL){
        printf("Server timeout; exiting");
        return 1;
    }
    
    #ifdef TESTING
    printf("Path: %s\n", resp);
    #endif

    FILE* file = fopen(&(resp[1]), "r");
    if(!file){
        printf("Error opening file.\n");
        flags = HANDSHAKE_BIT | ACK_BIT | SEQ_BIT;
        buff[0] = flags;
        sendRTP(sockfd, buff, 1, client_addr, resp, MAX_BUFF_SIZE, serv_addr, &bytes_recv, 0);
        return 2;
    }

    //Get the content size and read the file into the out_buff
    int content_size;

    fseek(file, 0, SEEK_END);
    content_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* file_contents = (char*)malloc(content_size);
    fread(file_contents, content_size, 1, file);

    fclose(file);

    char seq = SEQ_BIT;
    flags = seq;
    buff[0] = flags;
    for(int i = 0; i < (content_size / (MAX_BUFF_SIZE - 1)) + !!(content_size % (MAX_BUFF_SIZE - 1)); i++){
        int content_offset = i * (MAX_BUFF_SIZE - 1);
        int j;
        for(j = 1; j < MAX_BUFF_SIZE && content_offset + j < content_size; j++){
            buff[j] = file_contents[content_offset + j - 1];
        }
        resp = sendRTP(sockfd, buff, j, client_addr, resp, MAX_BUFF_SIZE, serv_addr, &bytes_recv, 1);
        while(!(resp[0] & ACK_BIT) || seq != (resp[0] & SEQ_BIT)){
            resp = sendRTP(sockfd, buff, j, client_addr, resp, MAX_BUFF_SIZE, serv_addr, &bytes_recv, 1);
            #ifdef TESTING
            printf("Failed to recieve the proper packet.\nACK Flag: %d\nSEQ Flag:%d\nseq:%d\n\n", resp[0] & ACK_BIT, resp[0] & SEQ_BIT, seq);
            #endif
        }

        //Alternate between setting and unsetting the sequence bit
        seq = (seq)? 0 : SEQ_BIT;

        flags = seq;
        buff[0] = flags;
    }

    //Terminate the connection
    flags |= LAST_BIT;
    buff[0] = flags;
    sendRTP(sockfd, &flags, 1, client_addr, resp, MAX_BUFF_SIZE, serv_addr, &bytes_recv, 0);

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

    //Set it to timeout after eight seconds
    struct timeval tval;
    tval.tv_sec = 8;
    tval.tv_usec = 0;
    if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tval, sizeof tval) < 0){
        perror("Error");
    }

    int bytes_recv = 0;
    struct sockaddr from_addr;
    socklen_t from_addr_len = sizeof from_addr;

    //Listening for the initial request
    while(1){
        bytes_recv = recvfrom(sockfd, (void*)buff, MAX_BUFF_SIZE, 0, &from_addr, &from_addr_len);
        if(bytes_recv > 0){
            handleRequest(sockfd, buff, serv_info->ai_addr, &from_addr, &from_addr_len);
        
            //Clear the buffer
            memset(buff, 0, MAX_BUFF_SIZE);
        }
    }

    //Free the server info
    freeaddrinfo(serv_info);

    close(sockfd);

    return 0;
}
