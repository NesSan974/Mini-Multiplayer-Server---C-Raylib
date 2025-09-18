#ifndef __NET_PROTOCOL_H__
#define __NET_PROTOCOL_H__

#define PORT 22222
#define MAX_PLAYERS 16


size_t playerNetworkSize = sizeof(int) + sizeof(float) * 2 + sizeof(uint8_t);

typedef struct PlayerNetwork
{
    int socket_fd; // pour associer à fd_set, sert egalement d'id pour l'instant (vu que localhost pour l'instant c'est ok)
    float x;       // position
    float y;       // position
    uint8_t playerstate;
} PlayerNetwork;

/**
 * @enum Msg_Type
 * @brief Represents the type of message exchanged between client and server.
 *
 * Each value corresponds to the first byte in the protocol.
 */
enum Msg_Type
{
    HELLO = 'H',
    WELCOME = 'W',

    READY = 'R',
    GOPLAY = 'G',

    PLAYER_MOVE = 'M',

    UPDATE_ALL_PLAYERS = 'U',

    ERROR = 'E',

    BYE = 'B',
};

enum Player_State
{
    IDLE = 0,
    ALIVE = 1,
    MORT = 2,
    WAITING_LOBBY = 3,
};

#endif