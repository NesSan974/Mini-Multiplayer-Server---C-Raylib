
#include <stddef.h> // => size_t
#include <stdint.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdbool.h>

#include "stb_ds.h"
#include "ProtocolNetwork.h"
#include "PlayerNetwork.h"
#include "Player.h"
#include "Globals.h"

void broadcast_all_player_goplay()
{

    #ifdef DEBUG
    printf("broadcast_all_player_goplay()\n");
    #endif
    for (size_t i = 0; i < arrlen(da_players); i++)
    {
        #ifdef DEBUG
        printf("sending 'G' to %d\n", da_players[i].socket_fd);
        #endif

        sendMessage(da_players[i].socket_fd, GOPLAY, &(uint8_t){0}, sizeof(uint8_t)); // send a nil data

        da_players[i].playerstate = PLAYER_ACTIVE;
    }
}

void broadcast_all_player_positions()
{
    int nb_players = arrlen(da_players);

    for (size_t i = 0; i < nb_players; i++)
    {

        if (da_players[i].playerstate != PLAYER_ACTIVE)
            continue;

        sendMessage(da_players[i].socket_fd, UPDATE_ALL_PLAYERS, da_players, nb_players * sizeof(PlayerNetwork));
    }
}

int getPlayerIndex(int socket)
{
    for (size_t i = 0; i < arrlen(da_players); i++)
    {
        if (da_players[i].socket_fd == socket)
            return i;
    }

    return -1;
}