#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include <signal.h>

#include <arpa/inet.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include <sys/select.h>
#include <fcntl.h>

#include "net_protocol.h"

#define TICK_RATE_MS 50 // 20 updates/sec


uint8_t last_playerid = 0;
fd_set readfds;



PlayerNetwork players[MAX_PLAYERS];

PlayerNetwork* getPlayer(int socket)
{
    for (size_t i = 0; i < MAX_PLAYERS; i++)
    {
        if (players[i].socket_fd == socket)
            return &players[i];
    }

    return NULL;
}

uint64_t current_time_ms()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)(tv.tv_sec) * 1000 + (tv.tv_usec / 1000);
}

void broadcast_all_player_goplay()
{
    printf("broadcast_all_player_goplay()\n");
    for (size_t i = 0; i < MAX_PLAYERS; i++)
    {

        if (players[i].socket_fd <= 0)
            continue;

        printf("sending 'G' to %d\n", players[i].socket_fd);
        send(players[i].socket_fd, &(char){GOPLAY}, sizeof(char), 0);
        send(players[i].socket_fd, &(uint16_t){0}, sizeof(uint16_t), 0);
        send(players[i].socket_fd, &(uint8_t){0}, sizeof(uint8_t), 0);

        players[i].playerstate = ALIVE;
    }
}

void broadcast_all_player_positions()
{
    // --- Étape 1 : Filtrer les joueurs connectés ---
    // On récupere tout les players actif dans le tableau "players_buffer"

    PlayerNetwork players_buffer[MAX_PLAYERS]; // Buffer temporaire
    uint8_t nb_active_players = 0;

    for (size_t i = 0; i < MAX_PLAYERS; i++)
    {

        PlayerNetwork *p = &players[i];

        if (p->socket_fd <= 0)
            continue;
        if (p->playerstate == IDLE || p->playerstate == WAITING_LOBBY)
            continue;

        players_buffer[nb_active_players++] = *p;
    }

    // --- Étape 2 : Copier les joueurs actifs dans un tableau à taille exacte ---
    // On créé un tableau (active_players) qui possede exactement la bonne taille, et on y copie les données de players_buffer

    PlayerNetwork active_players[nb_active_players];
    memcpy(active_players, players_buffer, nb_active_players * sizeof(PlayerNetwork));

    // --- Étape 3 : Debug (affiche les joueurs actifs) ---

    for (size_t i = 0; i < nb_active_players; i++)
        printf("act player : %d\n", active_players[i].socket_fd);

    // --- Étape 4 : Broadcast de tous les joueurs actifs à tous les autres ---

    for (size_t i = 0; i < nb_active_players; i++)
    {
        int socket = active_players[i].socket_fd;

        // Envois de type de message et de la taille des données à venir
        send(socket, &(char){UPDATE_ALL_PLAYERS}, sizeof(char), 0);
        send(socket, &(uint16_t){playerNetworkSize * nb_active_players}, sizeof(uint16_t), 0);

        for (size_t j = 0; j < nb_active_players; j++)
        {
            PlayerNetwork *otherPlayer = &players[j];

            send(socket, &otherPlayer->socket_fd, sizeof(int), 0);
            send(socket, &otherPlayer->x, sizeof(float), 0);
            send(socket, &otherPlayer->y, sizeof(float), 0);
            send(socket, &otherPlayer->playerstate, sizeof(uint8_t), 0);

        }
    }
}

/**
 * @brief Handles a message received from a client socket.
 *
 * This function receives and interprets a custom protocol message
 * from a client. The protocol follows the format:
 * [type (1 byte)][length (2 bytes)][data].
 *
 * The `type` field corresponds to a value of the `Msg_Type` enum.
 * Based on this type, the function handles the appropriate logic:
 *
 * - HELLO: Registers the player and sends a WELCOME response.
 * - READY: Marks the player as ready; if >= 2 players are ready, broadcasts GO.
 * - PLAYER_MOVE: Updates the player's position in the array "players".
 * - BYE: Cleans up and closes the player's socket.
 * - Unknown type: Logs an unreachable state.
 *
 * @param socket The file descriptor of the client socket.
 *
 * @note The function assumes a non-blocking or reliable connection.
 *       If `recv()` returns 0 (client disconnected), the socket is closed.
 *
 * @return void
 *
 * @see enum Msg_type
 */
void HandleMessage(int socket)
{
    // 'type' is the first field of the protocole, with 1 byte length
    uint8_t type;
    int n = recv(socket, &type, sizeof(type), 0);

    if (n == 0)
    {
        // Si la connexion à été fermé
        // Alors on reset le player

        PlayerNetwork *p = getPlayer(socket);
        p->socket_fd = -1;
        p->x = -1, p->y = -1;
        p->playerstate = IDLE;

        close(socket); // Je sais pas si on doit le mettre vu qu'en téhorie c'est déjà close ..
        return;
    }

    // 'length' is the second field of the protocole, with 2 byte length
    uint16_t length;
    n = recv(socket, &length, sizeof(length), 0);

    // if (n == 0)w
    // {
    //     close(socket);
    //     return;
    // }

    switch (type)
    {
    case HELLO:

        printf("HELLO RECEIVE fd : %d\n", socket);

        uint8_t nil_data;
        n = recv(socket, &nil_data, sizeof(nil_data), 0);

        players[last_playerid++] = (struct PlayerNetwork){
            .socket_fd = socket,
            .playerstate = IDLE,
            .x = -1,
            .y = -1,
        };
        printf("all connected players :\n");

        for (size_t i = 0; i < MAX_PLAYERS; i++)
        {
            if (players[i].socket_fd > 0)
            {
                printf("%d\n", players[i].socket_fd);
            }
        }
        printf("-------------\n");

        send(socket, &(char){WELCOME}, sizeof(char), 0);
        send(socket, &(uint16_t){sizeof(socket)}, sizeof(uint16_t), 0);
        send(socket, &socket, sizeof(socket), 0);

        break;

    case READY:
    {
        printf("READY RECEIVE\n");

        uint8_t data_id;
        recv(socket, &data_id, sizeof(data_id), 0);

        PlayerNetwork *p = getPlayer(socket);
        if (p->socket_fd == -1)
        {
            printf("clacrise wola \n");
            exit(-693); // TODO : on devrait pas exit, c'est pour voir  quand ca arrive ici
            send(socket, &(char){ERROR}, sizeof(char), 0);
            send(socket, &(uint16_t){0}, sizeof(uint16_t), 0);
            send(socket, &(uint8_t){0}, sizeof(uint8_t), 0);

            return;
        }

        p->playerstate = WAITING_LOBBY;

        int nbReady = 0;
        for (size_t i = 0; i < MAX_PLAYERS; i++)
        {
            if (players[i].socket_fd > 0)
            {
                nbReady += players[i].playerstate == WAITING_LOBBY ? 1 : 0;
            }
        }

        if (nbReady >= 2)
        {
            broadcast_all_player_goplay();
        }
    }
    break;

    case PLAYER_MOVE:
    {
        // printf("PLAYER_MOVE RECEIVE\n");

        float playerPositionX = 0.0f, playerPositionY = 0.0f;

        recv(socket, &playerPositionX, sizeof(playerPositionX), 0);
        recv(socket, &playerPositionY, sizeof(playerPositionY), 0);

        PlayerNetwork *p = getPlayer(socket);

        p->x = playerPositionX;
        p->y = playerPositionY;

        printf("[%d] %f, %f\n", socket, playerPositionX, playerPositionY);
    }
    break;

    case BYE:
    {
        printf("BYE RECEIVE\n");

        uint8_t nil_data;
        n = recv(socket, &nil_data, sizeof(nil_data), 0);

        close(socket);

        PlayerNetwork *p = getPlayer(socket);
        p->socket_fd = -1;
        p->playerstate = IDLE;
        p->x = -1.0f;
        p->y = -1.0f;
    }
    break;

    default:
        printf("UNREACHABLE\n");

        break;
    }
}

/**
 * @brief Handles messages from all currently connected clients.
 *
 * This function iterates over all connected client sockets,
 * adds them to a `fd_set` for reading, and uses `select()` to check
 * which ones have incoming data. For each client ready to communicate,
 * it delegates message handling to the `HandleMessage()` function.
 *
 * It does **not** handle new connections — only clients that have already been accepted
 * and are stored in the global `players[]` array.
 *
 * @note This function assumes that the global `PlayerNetwork players[]` array and `fd_set readfds` exist.
 *
 * @return int Returns:
 *  - `0` on success,
 *  - `-1` if there are no connected clients (`max_fd < 0`) or `select()` fails.
 *
 * @see HandleMessage
 */
int HandleConnectedClient()
{

    // Gestion des connexion deja etablie

    FD_ZERO(&readfds);

    int max_fd = -1;

    // On met dans "readfds" uniquement les clients qui ont deja un fd. donc les client "connecté"
    for (size_t i = 0; i < MAX_PLAYERS; i++)
    {
        int clt_fd = players[i].socket_fd;
        if (clt_fd > 0)
        {
            FD_SET(clt_fd, &readfds);
            max_fd = max_fd >= clt_fd ? max_fd : clt_fd;
        }
    }

    if (max_fd < 0)
        return -1;

    struct timeval tv = {0, 0};
    if (select(max_fd + 1, &readfds, NULL, NULL, &tv) < 0)
        return -1;

    // On boucle sur les joueur et on interperete les messages
    for (size_t i = 0; i < MAX_PLAYERS; i++)
    {
        if (FD_ISSET(players[i].socket_fd, &readfds))
        {
            // SI il a été mis a jour
            players[i].socket_fd;
            // printf("this sock will be updated %d \n", players[i].socket_fd);
            HandleMessage(players[i].socket_fd);
        }
    }
}

int main(void)
{

    // Ignore le signal SIGPIPE, arrive lorsqu'on ecrit dans un socket fermé, et clos le programme de manière innopiné
    signal(SIGPIPE, SIG_IGN);

    // Initialisation du tableau players
    for (size_t i = 0; i < MAX_PLAYERS; i++)
    {
        players[i] = (PlayerNetwork){
            .socket_fd = -1,
            .x = -1.0f,
            .y = -1.0f};
    }

    // ---------------------------- SOCKET

    // Creation socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
        return EXIT_FAILURE;

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
        return EXIT_FAILURE;

    // Listen
    if (listen(sockfd, 5) == -1)
        return EXIT_FAILURE;

    printf("listening on port %d ...\n", PORT);

    // ---------------------------- MAIN LOOP

    /*
     * La main loop commence par LA gestion du tickrate, afin d'envoyer de maniere regulirer la position de tout les joueurs
     * Ensuite,  on "accept()" en non bloquant, et si il y a un nouveau client, on le handle directement avec HandleMessage(int);
     * Sinon on gere les clients deja enregistrer avec 'HandleConnectedClient()'. on select() (man 2 select) et on HandleMessage(int).
     */

    uint64_t last_update = current_time_ms();

    while (1)
    {
        // TickRate
        uint64_t now = current_time_ms();
        if (now - last_update >= TICK_RATE_MS)
        {
            broadcast_all_player_positions();
            last_update = now;
        }

        // Socket
        int clientFd = -1;

        // Accept non bloquant
        struct sockaddr_in clientaddr;
        socklen_t addrlen = sizeof(clientaddr);
        clientFd = accept(sockfd, (struct sockaddr *)&clientaddr, &addrlen); // Non bloquant grace a fcntl

        if (clientFd >= 0)
        {
            // Si Nouvelle connexion
            // On affiche un message et on gere le message
            char ip_buf[16];
            inet_ntop(AF_INET, &clientaddr.sin_addr, ip_buf, sizeof(ip_buf));
            printf("new connection from %s:%d\n", ip_buf, ntohs(clientaddr.sin_port));

            HandleMessage(clientFd);
        }
        else
        {
            // Gestion player deja connecté

            // Si il ya eu une "vrai" erreur sur accept
            if (errno != EAGAIN)
            {
                perror("accept");
            }

            HandleConnectedClient();
        }
    }
}