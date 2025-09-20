#ifndef __MAIN_H__
#define __MAIN_H__


enum Game_State
{
    GS_MENU = 0,
    GS_LOBBY = 1,
    GS_GAME = 2,
};

extern enum Game_State game_state;

extern const int screenWidth;
extern const int screenHeight;


#endif // __MAIN_H__