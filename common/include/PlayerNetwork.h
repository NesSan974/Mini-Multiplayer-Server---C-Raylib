#ifndef __PLAYER_NETWORK_H__
#define __PLAYER_NETWORK_H__

#define PACKED_STRUCT(name) struct __attribute__((packed)) name // Evite le padding pour le transport reseaux

enum Player_State
{
    PLAYER_IDLE = 0,        // State par defaut
    PLAYER_READY,           // Dans le lobby, en etant prêt, attend que la partie démarre
    PLAYER_ACTIVE,          // Partie en cours, reçoit/envoie des données
    PLAYER_DISCONNECTED     // Déconnecté / timeout / kické ...      
};

typedef uint8_t PlayerState_t;  // Type réseau pour enum PlayerState

typedef PACKED_STRUCT( PlayerNetwork)
{
    int socket_fd;              // pour associer à fd_set, sert egalement d'id pour l'instant (pour l'instant c'est ok)
    float x;                    // position X
    float y;                    // position Y
    PlayerState_t playerstate;  // Enum 'Player_State'
} PlayerNetwork;


#endif //__PLAYER_NETWORK_H__