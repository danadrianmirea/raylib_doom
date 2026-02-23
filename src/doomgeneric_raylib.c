// Raylib-based platform implementation for doomgeneric
#include "doomgeneric/doomgeneric.h"
#include <raylib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
// Avoid including <windows.h> to prevent symbol conflicts with raylib
// Declare Sleep manually (kernel32) to sleep for milliseconds.
void Sleep(unsigned long ms);
#else
#include <unistd.h>
#endif

static Texture2D dg_texture = {0};
static unsigned char *dg_pixels = NULL; // RGBA pixel buffer for raylib
static int dg_window_created = 0;

void DG_Init()
{
    if (dg_window_created) return;
    
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(DOOMGENERIC_RESX, DOOMGENERIC_RESY, "DOOM (raylib)");
    MaximizeWindow();
    SetTargetFPS(60);

    
    // Create an empty texture we will update each frame
    Image img = GenImageColor(DOOMGENERIC_RESX, DOOMGENERIC_RESY, BLACK);
    dg_texture = LoadTextureFromImage(img);
    UnloadImage(img);

    dg_pixels = (unsigned char*)malloc(DOOMGENERIC_RESX * DOOMGENERIC_RESY * 4);
    dg_window_created = 1;
}

void DG_DrawFrame()
{
    if (!dg_window_created) return;

    if (DG_ScreenBuffer == NULL) return;

    int total = DOOMGENERIC_RESX * DOOMGENERIC_RESY;
    for (int i = 0; i < total; i++)
    {
        pixel_t p = DG_ScreenBuffer[i];
        dg_pixels[i * 4 + 0] = (p >> 16) & 0xFF; // R
        dg_pixels[i * 4 + 1] = (p >> 8) & 0xFF;  // G
        dg_pixels[i * 4 + 2] = p & 0xFF;         // B
        dg_pixels[i * 4 + 3] = 255;              // A
    }

    UpdateTexture(dg_texture, dg_pixels);

    BeginDrawing();
    ClearBackground(BLACK);

    // Draw the texture scaled to the current window size
    Rectangle src = { 0.0f, 0.0f, (float)DOOMGENERIC_RESX, (float)DOOMGENERIC_RESY };
    Rectangle dst = { 0.0f, 0.0f, (float)GetScreenWidth(), (float)GetScreenHeight() };
    Vector2 origin = { 0.0f, 0.0f };
    DrawTexturePro(dg_texture, src, dst, origin, 0.0f, WHITE);

    EndDrawing();
}

void DG_SleepMs(uint32_t ms)
{
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
}

uint32_t DG_GetTicksMs()
{
    double t = GetTime(); // seconds
    return (uint32_t)(t * 1000.0);
}

int DG_GetKey(int* pressed, unsigned char* key)
{
    // Track only the relevant keys for Doom
    static struct {
        int raylib_code;
        unsigned char doom_code;
        int state; // 1 = currently pressed, 0 = not pressed
    } keys[] = {
        {KEY_A, 'a', 0}, {KEY_B, 'b', 0}, {KEY_C, 'c', 0}, {KEY_D, 'd', 0},
        {KEY_E, 'e', 0}, {KEY_F, 'f', 0}, {KEY_G, 'g', 0}, {KEY_H, 'h', 0},
        {KEY_I, 'i', 0}, {KEY_J, 'j', 0}, {KEY_K, 'k', 0}, {KEY_L, 'l', 0},
        {KEY_M, 'm', 0}, {KEY_N, 'n', 0}, {KEY_O, 'o', 0}, {KEY_P, 'p', 0},
        {KEY_Q, 'q', 0}, {KEY_R, 'r', 0}, {KEY_S, 's', 0}, {KEY_T, 't', 0},
        {KEY_U, 'u', 0}, {KEY_V, 'v', 0}, {KEY_W, 'w', 0}, {KEY_X, 'x', 0},
        {KEY_Y, 'y', 0}, {KEY_Z, 'z', 0},
        {KEY_ZERO, '0', 0}, {KEY_ONE, '1', 0}, {KEY_TWO, '2', 0}, {KEY_THREE, '3', 0},
        {KEY_FOUR, '4', 0}, {KEY_FIVE, '5', 0}, {KEY_SIX, '6', 0}, {KEY_SEVEN, '7', 0},
        {KEY_EIGHT, '8', 0}, {KEY_NINE, '9', 0},
        {KEY_SPACE, 0xa2, 0}, {KEY_ENTER, 13, 0}, {KEY_TAB, 9, 0},
        {KEY_BACKSPACE, 0x7f, 0}, {KEY_ESCAPE, 27, 0},
        {KEY_UP, 0xad, 0}, {KEY_DOWN, 0xaf, 0}, {KEY_LEFT, 0xac, 0}, {KEY_RIGHT, 0xae, 0},
        {KEY_LEFT_SHIFT, 0x80 + 0x36, 0}, {KEY_RIGHT_SHIFT, 0x80 + 0x36, 0},
        {KEY_LEFT_CONTROL, 0xa3, 0}, {KEY_RIGHT_CONTROL, 0xa3, 0},
        {KEY_LEFT_ALT, 0x80 + 0x38, 0}, {KEY_RIGHT_ALT, 0x80 + 0x38, 0},
        {-1, 0, 0} // sentinel
    };

    // Check for state changes
    for (int i = 0; keys[i].raylib_code != -1; ++i) {
        int cur = IsKeyDown(keys[i].raylib_code) ? 1 : 0;
                
        if (cur != keys[i].state) {
            keys[i].state = cur;
            *pressed = cur;
            *key = keys[i].doom_code;
            return 1;
        }
    }

    return 0;
}

void DG_SetWindowTitle(const char * title)
{
    if (!dg_window_created) return;
    SetWindowTitle(title);
}

int main(int argc, char **argv)
{
    doomgeneric_Create(argc, argv);

    for (int i = 0; ; i++)
    {
        doomgeneric_Tick();
    }
    

    return 0;
}