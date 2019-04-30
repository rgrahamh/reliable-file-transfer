#ifndef RTP_SHARED
#define RTP_SHARED
#include <arpa/inet.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define MAX_IN_BUFF_SIZE 4096

char* sendRTP(int sockfd, char* msg, size_t msg_len, struct sockaddr * to_addr, char* resp, size_t resp_len, struct sockaddr * from_addr){
    int tryCount = 0;
    int bytes_recv = 0;
    clock_t last_packet_sent = clock();
    socklen_t from_addr_len = sizeof *from_addr;
    for(int tryCount = 0; tryCount < 8 && bytes_recv <= 0; tryCount++){
        sendto(sockfd, msg, msg_len, 0, to_addr, sizeof *to_addr);
        bytes_recv = recvfrom(sockfd, resp, resp_len, 0, from_addr, &from_addr_len);
    }
    return resp;
}
#endif
