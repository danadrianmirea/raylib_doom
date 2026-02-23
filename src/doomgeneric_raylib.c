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
    // Return one keypress event at a time. Return 0 when no events available.
    int k = GetKeyPressed();
    if (k == 0) return 0;

    // By convention return only key-down events (pressed = 1)
    *pressed = 1;

    // Map raylib key codes to simple ASCII or doom-specific codes
    if (k >= KEY_A && k <= KEY_Z)
    {
        *key = (unsigned char)tolower((int)k);
        return 1;
    }

    if (k >= KEY_ZERO && k <= KEY_NINE)
    {
        // KEY_ZERO..KEY_NINE are ASCII '0'..'9' in raylib
        *key = (unsigned char)k;
        return 1;
    }

    switch (k)
    {
        case KEY_SPACE: *key = ' '; return 1;
        case KEY_ENTER: *key = 13; return 1;        /* Doom KEY_ENTER */
        case KEY_TAB: *key = 9; return 1;           /* Doom KEY_TAB */
        case KEY_BACKSPACE: *key = 0x7f; return 1; /* Doom KEY_BACKSPACE */
        case KEY_ESCAPE: *key = 27; return 1;      /* Doom KEY_ESCAPE */
        case KEY_RIGHT: *key = 0xae; return 1;     /* Doom KEY_RIGHTARROW */
        case KEY_LEFT: *key = 0xac; return 1;      /* Doom KEY_LEFTARROW */
        case KEY_UP: *key = 0xad; return 1;        /* Doom KEY_UPARROW */
        case KEY_DOWN: *key = 0xaf; return 1;      /* Doom KEY_DOWNARROW */
        default:
            // Unknown key: return as zero so caller ignores it
            *key = 0;
            return 1;
    }
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