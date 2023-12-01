#pragma once

#ifndef DEBUG_COLLISIONS
#define DEBUG_COLLISIONS 0 // Configure in CMakeLists.txt
#endif

#define MAX_LINE_LENGTH 1024

#define MAX_SOUND_CHANNELS 8

#define ASPECT_RATIO ((float)WINDOW_WIDTH / (float)WINDOW_HEIGHT)

#define FONT_SIZE 16

#define ASCII_OFFSET 32
#define ASCII_MAX 126
#define ASCII_COUNT (ASCII_MAX - ASCII_OFFSET)

#ifndef GAME_NAME
#define GAME_NAME "Add Game Name to CMakeLists.txt"
#endif