#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>

#include <stddef.h>
#include <stdint.h>

#include "stb_ds.h"
#include "ProtocolHandler.h"
#include "ProtocolNetwork.h"
#include "PlayerNetwork.h"
#include "Player.h"
#include "Globals.h"


void HandleMessage(int socket)
{
    char type;
    uint16_t length;
    uint8_t buf[256];
    void* data = buf;

    int n = recvMessage(socket, &type, data, &length, true);

    if (n < 0) {return; } // TODO : faudrait verifier si c'est un EAGAIN avant de return 

    if (n == 0)
    {
        // Si la connexion à été fermé
        // Alors on reset le player

        int index = getPlayerIndex(socket);
        if (index < 0)
            return;
        
        arrdel(da_players, index);

        //close(socket);
        return;
    }
        
    switch (type)
    {
    case HELLO:

        printf("HELLO RECEIVE fd : %d\n", socket);

        PlayerNetwork p = {
            .socket_fd = socket,
            .playerstate = PLAYER_IDLE,
            .x = -1,
            .y = -1,
        };

        arrpush(da_players, p);

        printf("all connected players :\n");

        for (size_t i = 0; i < arrlen(da_players); i++)
        {
            printf("%d\n", da_players[i].socket_fd);
        }
        printf("-------------\n");

        sendMessage(socket, WELCOME, &socket, sizeof(socket));

        break;

    case READY:
    {
        printf("READY RECEIVE\n");

        int index = getPlayerIndex(socket);
        if (index < 0)
            return;

        PlayerNetwork *p = &da_players[index];

        p->playerstate = PLAYER_READY;

        int nbPlayerWaitingReady = 0;

        for (size_t i = 0; i < arrlen(da_players); i++)
        {
            nbPlayerWaitingReady += da_players[i].playerstate == PLAYER_READY ? 1 : 0;
        }

        if (nbPlayerWaitingReady >= arrlen(da_players))
        {
            broadcast_all_player_goplay();
        }
    }
    break;

    case PLAYER_MOVE:
    {
        // printf("PLAYER_MOVE RECEIVE\n");
        int index = getPlayerIndex(socket);
        if (index < 0)
            return;

        da_players[index] = *(PlayerNetwork*)data;
        
        printf("[%d]%f, %f\n",da_players[index].socket_fd, da_players[index].x, da_players[index].y);

    }
    break;

    case BYE:
    {
        printf("BYE RECEIVE\n");

        close(socket);

        int index = getPlayerIndex(socket);
        if (index < 0)
            return;

        arrdel(da_players, index);
    }
    break;

    default:
        printf("UNREACHABLE\n");

        break;
    }
}

int HandleConnectedPlayers()
{

    fd_set readfds;

    // Gestion des connexion deja etablie

    FD_ZERO(&readfds);

    int max_fd = -1;

    // On met dans "readfds" les players connecté
    for (size_t i = 0; i < arrlen(da_players); i++)
    {
        int sock = da_players[i].socket_fd;
        FD_SET(sock, &readfds);
        max_fd = max_fd >= sock ? max_fd : sock;
    }

    if (max_fd < 0)
        return -1;

    // select non bloquant avec un timeval a 0
    struct timeval tv = {0, 0};
    if (select(max_fd + 1, &readfds, NULL, NULL, &tv) < 0)
        return -1;

    // On boucle sur les players et ceux qui ont envoyé des messages on les interperete
    for (size_t i = 0; i < arrlen(da_players); i++)
    {
        if (FD_ISSET(da_players[i].socket_fd, &readfds))
        {
            // si on peut lire le fd / si message recu
            HandleMessage(da_players[i].socket_fd);
        }
    }
}