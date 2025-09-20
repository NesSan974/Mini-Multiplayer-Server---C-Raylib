#include <stdint.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <stdbool.h>

#include "ProtocolNetwork.h"

uint64_t current_time_ms()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)(tv.tv_sec) * 1000 + (tv.tv_usec / 1000);
}

int sendMessage(int sockfd, Net_msgType_t type, const void *buffer, size_t size)
{
    int total_read = 0;

    total_read += send(sockfd, &type, sizeof(Net_msgType_t), 0);
    total_read += send(sockfd, (Net_length_t*)&size, sizeof(Net_length_t), 0);
    // avec gestion endianess :
    // total_read += send(sockfd, &(uint16_t){htons(size)}, sizeof(uint16_t), 0); 
    total_read += send(sockfd, buffer, size, 0);

    return total_read;
}

int recvMessage(int sockfd, Net_msgType_t *type, void *data, Net_length_t *payload_size_received, bool shouldWait)
{
    
    int total_read = 0; 
    total_read += recv(sockfd, type, sizeof(Net_msgType_t), shouldWait ? 0 : MSG_DONTWAIT );

    if (total_read <= 0)
    {
        return total_read;
    }

    Net_length_t length_network;
    total_read += recv(sockfd, &length_network, sizeof(Net_length_t), MSG_WAITALL);
    
    *payload_size_received = length_network; 
    // gestion endianess :
    // *payload_size_received = ntohs(length_network);

    total_read += recv(sockfd, data, *payload_size_received, MSG_WAITALL);    

    // Faudra penser a :
    // - re-cast vers le bon type
    // - endian (faire les ntohs/ntohl)

    return total_read;
}
