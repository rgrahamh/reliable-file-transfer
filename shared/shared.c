#ifndef RTP_SHARED
#define RTP_SHARED
#include <arpa/inet.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define MAX_BUFF_SIZE 512

#define HANDSHAKE_BIT 0x80
#define ACK_BIT 0x40
#define SEQ_BIT 0x20
#define LAST_BIT 0x10

char* sendRTP(int sockfd, char* msg, size_t msg_len, struct sockaddr * to_addr, char* resp, size_t resp_len, struct sockaddr * from_addr, int* bytes_recv, char expect_reply){
    *bytes_recv = 0;
    socklen_t from_addr_len = sizeof *from_addr;
    if(!expect_reply){
        sendto(sockfd, msg, msg_len, 0, to_addr, sizeof *to_addr);
        return NULL;
    }
    int tryCount = 0;
    for(; tryCount < 8 && *bytes_recv <= 0; tryCount++){
        sendto(sockfd, msg, msg_len, 0, to_addr, sizeof *to_addr);
        *bytes_recv = recvfrom(sockfd, resp, resp_len, 0, from_addr, &from_addr_len);
    }
    if(tryCount == 8){
        printf("Timeout occurred, exiting.");
        exit(2);
    }
    return resp;
}
#endif
