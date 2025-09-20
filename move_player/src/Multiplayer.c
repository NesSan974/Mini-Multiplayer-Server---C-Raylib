#include <raylib.h>
#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>

#include "ProtocolNetwork.h"
#include "PlayerNetwork.h"

#include "Multiplayer.h"
#include "main.h"



int sockfd ;
int netPlayerId ;

PlayerNetwork players[MAX_PLAYERS]; // TODO : utiliser std_bs pour dynamic array (comme pour le server)

int createSocket()
{

    // Socket Initialization
    //--------------------------------------------------------------------------------------
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr = {
        .sin_family = AF_INET,  /* address family: AF_INET */
        .sin_port = htons(PORT) /* port in network byte order */
    };

    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    while (1)
    {

        if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
        {
            printf("impossible de joindre le serveur ! \n");
            sleep(3);
            continue;
        }

        break;
    }

    sendMessage(sockfd, HELLO, &(uint8_t){0}, sizeof(uint8_t));

    uint8_t type;
    recv(sockfd, &type, sizeof(type), 0);

    uint16_t length;
    recv(sockfd, &length, sizeof(length), 0);

    int n = recv(sockfd, &netPlayerId, length, 0);


    if (n <= 0)
    {
        return -2;
    }

    if (type != 'W')
    {
        return -666;
    }

    return 0;

}

void sendPlayerUpdate(Vector2 position)
{
    // NOTE : faudrait cast les float comme des uint32 pour les enoyés, puis les recast a float a l'arrrivé
    sendMessage(sockfd, PLAYER_MOVE,  &(PlayerNetwork){netPlayerId, position.x, position.y, (PlayerState_t)PLAYER_ACTIVE}, sizeof(PlayerNetwork) );
}

void checkNetworkMessage()
{
    Net_msgType_t type;
    uint16_t data_length;

    // faudrait calculer le pire cas pour créer un array de cette taille
    // actuellement : length players * sizeof(Playernetwork) est la data la plus grande
    uint8_t buffer[256]; // actuellemnt si on prend 8 joueurs * sizeof(Playernetwork) < 128 octets

    void *data = buffer;

    int n = recvMessage(sockfd, &type, data, &data_length, false);
    if (n <= 0) return;

    switch (type)
    {
    case WELCOME:

        netPlayerId = *(int *)data;
        printf("netPlayerId : %d\n", netPlayerId);
        break;

    case UPDATE_ALL_PLAYERS:
        //printf("UPDATE_ALL_PLAYERS\n");

        PlayerNetwork *recvPlayers = (PlayerNetwork *)data;

        for (size_t i = 0; i < data_length / sizeof(PlayerNetwork); i++)
        {
            players[i] = recvPlayers[i];
        }
        break;

    case GOPLAY:
        printf("game start ! \n");

        game_state = GS_GAME;
        break;

    default:
        printf("default case !!! unreachable ! \n");
        // On flush les les données du socket, car ils sont actuellement probablement illisible
        char flush[256];
        recv(sockfd, &flush, sizeof(flush), MSG_DONTWAIT);
        break;
    }
}

void sendReadyToPlay()
{
    sendMessage(sockfd, READY, &(uint8_t){true}, sizeof(uint8_t));
}
