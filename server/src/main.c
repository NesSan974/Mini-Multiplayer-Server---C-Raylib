#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <unistd.h>

#include <errno.h>
#include <signal.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>

#include <fcntl.h>

#include <sys/time.h>

#include "ProtocolNetwork.h"
#include "PlayerNetwork.h"
#include "Player.h"
#include "ProtocolHandler.h"
#include "Globals.h"

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#define MAX_LISTEN_QUEUE 5

static int createSocket()
{

    // Creation socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
        return -1;

    // Rendre le port réutilisable (utile surtout pour eviter d'attendre)
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    // Rendre le socket non bloquant (le "accept()")
    fcntl(sockfd, F_SETFL, O_NONBLOCK);

    // Binding
    struct sockaddr_in addr = {
        .sin_family = AF_INET,   /* address family: AF_INET */
        .sin_addr = INADDR_ANY,  /* Any internet address */
        .sin_port = htons(PORT), /* port in network byte order */
    };

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
        return -1;

    // Listen
    if (listen(sockfd, MAX_LISTEN_QUEUE) == -1)
        return -1;

    return sockfd;
}

int main(void)
{
    int sockfd = createSocket();

    if (sockfd < 0)
        return EXIT_FAILURE;

    // Ignore le signal SIGPIPE, arrive lorsqu'on ecrit dans un socket fermé, et clos le programme innopinément
    signal(SIGPIPE, SIG_IGN);

    printf("listening on port %d ...\n", PORT);

    // ---------------------------- MAIN LOOP

    uint64_t now = current_time_ms();
    uint64_t last_update = now;

    // Socket
    int clientFd = -1;

    struct sockaddr_in clientaddr;
    socklen_t addrlen = sizeof(clientaddr);


    // NOTE / TODO : mettre le socket server dans le select et handle tout les socket de la meme facon.

    while (1)
    {
        // TickRate
        now = current_time_ms();
        if (now - last_update >= TICK_RATE_MS)
        {
            
            broadcast_all_player_positions();
            last_update = now;

        }
        // Gestion player deja connecté
        HandleConnectedPlayers();
        
        // Accept non bloquant
        clientFd = accept(sockfd, (struct sockaddr *)&clientaddr, &addrlen); // Non bloquant grace a fcntl
        
        if (clientFd < 0)
            continue;

        // Si Nouvelle connexion
        // On affiche un message et on gere le message
        char ip_buf[16];
        inet_ntop(AF_INET, &clientaddr.sin_addr, ip_buf, sizeof(ip_buf));
        printf("new connection from %s:%d\n", ip_buf, ntohs(clientaddr.sin_port));

        HandleMessage(clientFd);
    }
}