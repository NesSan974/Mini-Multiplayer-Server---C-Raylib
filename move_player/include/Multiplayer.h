#ifndef __MULTIPLAYER_H__
#define __MULTIPLAYER_H__

#define MAX_PLAYERS 8

extern int sockfd;
extern int netPlayerId;
extern PlayerNetwork players[];

int createSocket();
void sendPlayerUpdate(Vector2 position);
void checkNetworkMessage();
void sendReadyToPlay();

#endif // __MULTIPLAYER_H__