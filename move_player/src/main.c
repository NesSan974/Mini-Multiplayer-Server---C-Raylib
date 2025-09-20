#include <raylib.h>
#include "raymath.h"

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

#include <stdint.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#include "ProtocolNetwork.h"
#include "PlayerNetwork.h"

#include  "Multiplayer.h"
#include "main.h"

enum Game_State game_state = GS_LOBBY;

const int screenWidth = 1200;
const int screenHeight = 720;

static void lobby_update()
{

    Rectangle button = {200.0f, 200.0f, 300.0f, 50.0f};
    Color btn_color = BLUE;

    bool btn_isActive = true;

    char *titleText = "Lobby, Click when ready";

    while (!WindowShouldClose() && game_state == GS_LOBBY)
    {
        //----------------------------------------------------------------------------------
        // Update
        //----------------------------------------------------------------------------------

        Vector2 mousePoint = GetMousePosition();

        checkNetworkMessage();

        if (btn_isActive)
        {
            btn_color = BLUE;

            // Check button state
            if (CheckCollisionPointRec(mousePoint, button))
            {
                btn_color = RED;

                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                {
                    sendReadyToPlay();

                    btn_color = ORANGE;
                    titleText = "Lobby, U R READY !!";
                    btn_isActive = false;
                }
            }
        }

        //----------------------------------------------------------------------------------
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(RAYWHITE);

        DrawText(titleText, 10, 10, 20, DARKGRAY);
        if (btn_isActive)
        {
            DrawRectangle(button.x, button.y, button.width, button.height, btn_color);
            DrawText("Click when ready ", button.x, button.y, 30, BLACK);
        }
        EndDrawing();
        //----------------------------------------------------------------------------------
    }
}

static void game_update()
{

    Vector2 playerPosition = (Vector2){(float)screenWidth / 2, (float)screenHeight / 2};

    float playerSpeed = 10.0f;

    uint64_t now = current_time_ms();
    uint64_t last_update = now;

    while (!WindowShouldClose() && game_state == GS_GAME)
    {
        //----------------------------------------------------------------------------------
        // Update
        //----------------------------------------------------------------------------------


        if (IsKeyDown(KEY_UP))
            playerPosition.y -= playerSpeed;

        if (IsKeyDown(KEY_DOWN))
            playerPosition.y += playerSpeed;

        if (IsKeyDown(KEY_LEFT))
            playerPosition.x -= playerSpeed;

        if (IsKeyDown(KEY_RIGHT))
            playerPosition.x += playerSpeed;

        // TickRate
        now = current_time_ms();
        if (now - last_update >= TICK_RATE_MS)
        {
            last_update = now;
            sendPlayerUpdate(playerPosition);
            checkNetworkMessage();
        }

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
            if (netPlayerId != players[i].socket_fd)
                DrawCircle(players[i].x, players[i].y, 50, MAGENTA);

        }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }
}

int main(void)
{
    createSocket();

    // Raylib Initialization
    //--------------------------------------------------------------------------------------

    InitWindow(screenWidth, screenHeight, "raylib multiplayer socket");

    SetTargetFPS(60);

    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {

        switch (game_state)
        {
        case GS_MENU:
            // game_update_menu();
            break;

        case GS_LOBBY:
            lobby_update();
            break;

        case GS_GAME:
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

    sendMessage(sockfd, BYE, &(uint8_t){0}, sizeof(uint8_t));
    close(sockfd);
    //--------------------------------------------------------------------------------------
}
