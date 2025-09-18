#include "raylib.h"
#include "raymath.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "../../server/net_protocol.h"



int sockfd;
uint8_t id;
Vector2 playerPosition;
float playerSpeed = 10.0f;
uint8_t game_state = 1;

PlayerNetwork players[MAX_PLAYERS];

void sendPlayerPosition(Vector2 position)
{
    send(sockfd, &(char){PLAYER_MOVE}, sizeof(char), 0);
    send(sockfd, &(uint16_t){sizeof(position.x) + sizeof(position.y)}, sizeof(uint16_t), 0);

    // NOTE : Verifier les endianess htons/htonl si ca pose pas de probleme

    send(sockfd, &position.x, sizeof(position.x), 0);
    send(sockfd, &position.y, sizeof(position.y), 0);
}

void pullFromServer()
{

    uint8_t type;
    int n = recv(sockfd, &type, sizeof(type), MSG_DONTWAIT);

    if (n <= 0)
    {
        return;
    }

    uint16_t length;
    n = recv(sockfd, &length, sizeof(length), MSG_WAITALL);

    switch (type)
    {
    case WELCOME:

        recv(sockfd, &id, sizeof(id), MSG_WAITALL);
        break;

    case UPDATE_ALL_PLAYERS:

        // NOTE : verifier endianess si vrai lan cross plateform
        for (size_t i = 1; i <= length / playerNetworkSize; i++)
        {
            n = recv(sockfd, &players[i], playerNetworkSize, 0);
        }

        break;

    case GOPLAY:
        printf("game start ! \n");
        uint8_t nil;
        recv(sockfd, &nil, sizeof(nil), MSG_WAITALL);

        game_state++;
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
    send(sockfd, &(char){READY}, sizeof(char), 0);
    send(sockfd, &(uint16_t){sizeof(id)}, sizeof(uint16_t), 0);
    send(sockfd, &id, sizeof(id), 0);
}

void lobby_update()
{

    //----------------------------------------------------------------------------------
    // Update
    //----------------------------------------------------------------------------------
    Vector2 mousePoint = GetMousePosition();

    Rectangle button = {200.0f, 200.0f, 300.0f, 50.0f};

    pullFromServer();

    // Check button state
    if (CheckCollisionPointRec(mousePoint, button) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        sendReadyToPlay();
    }

    //----------------------------------------------------------------------------------
    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

    ClearBackground(RAYWHITE);

    DrawText("Lobby : Waiting to start ", 10, 10, 20, DARKGRAY);

    DrawRectangle(button.x, button.y, button.width, button.height, GREEN);

    EndDrawing();
    //----------------------------------------------------------------------------------
}

void game_update()
{

    //----------------------------------------------------------------------------------
    // Update
    //----------------------------------------------------------------------------------

    pullFromServer();

    if (IsKeyDown(KEY_UP))
        playerPosition.y -= playerSpeed;

    if (IsKeyDown(KEY_DOWN))
        playerPosition.y += playerSpeed;

    if (IsKeyDown(KEY_LEFT))
        playerPosition.x -= playerSpeed;

    if (IsKeyDown(KEY_RIGHT))
        playerPosition.x += playerSpeed;

    // Deja caper avec setTargetFPS()
    sendPlayerPosition(playerPosition);

    //----------------------------------------------------------------------------------
    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

    ClearBackground(RAYWHITE);

    DrawText("GAME", 10, 10, 20, DARKGRAY);

    DrawCircleV(playerPosition, 50, RED); // Main Player

    // Other player
    for (size_t i = 0; i < MAX_PLAYERS; i++)
    {
        if (players[i].playerstate != 1 || id == players[i].socket_fd)
        {
            continue;
        }

        DrawCircle(players[i].x, players[i].y, 50, MAGENTA);
    }

    EndDrawing();
    //----------------------------------------------------------------------------------
}

int main(void)
{

    // Socket Initialization
    //--------------------------------------------------------------------------------------
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr = {
        .sin_family = AF_INET,  /* address family: AF_INET */
        .sin_port = htons(PORT) /* port in network byte order */
    };

    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        printf("impossible de joindre le serveur ! \n");

        return 1;
    }

    // Envois du hello, conformement au protocol.

    send(sockfd, &(char){HELLO}, sizeof(char), 0);
    send(sockfd, &(uint16_t){0}, sizeof(uint16_t), 0);
    send(sockfd, &(uint8_t){0}, sizeof(uint8_t), 0);

    // Reception du Welcome, conformement au protocol.

    uint8_t type;
    recv(sockfd, &type, sizeof(type), 0);

    uint16_t length;
    recv(sockfd, &length, sizeof(length), 0);

    int n = recv(sockfd, &id, sizeof(id), 0);

    if (n <= 0)
    {
        return 2;
    }

    if (type != 'W')
    {
        return 1;
    }

    // Raylib Initialization
    //--------------------------------------------------------------------------------------

    const int screenWidth = 1200;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "raylib multiplayer socket");

    playerPosition = (Vector2){(float)screenWidth / 2, (float)screenHeight / 2};

    SetTargetFPS(60);

    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {

        switch (game_state)
        {
        case 0:
            // game_update_menu();
            break;

        case 1:
            lobby_update();
            break;

        case 2:
            game_update();
            break;

        default:
            break;
        }
    }

    // De-Initialization
    //--------------------------Close window and OpenGL context-----------------------------
    CloseWindow();
    //--------------------------------------------------------------------------------------

    //---------------------Closing connection and Socket------------------------------------
    // Send Bye
    send(sockfd, &(char){BYE}, sizeof(char), 0);
    send(sockfd, &(uint16_t){sizeof(uint8_t)}, sizeof(uint16_t), 0);
    send(sockfd, &(uint8_t){0}, sizeof(uint8_t), MSG_WAITALL);
    close(sockfd);
    //--------------------------------------------------------------------------------------
}
