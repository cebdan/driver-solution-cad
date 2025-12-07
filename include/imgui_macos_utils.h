#ifndef IMGUI_MACOS_UTILS_H
#define IMGUI_MACOS_UTILS_H

#include <SDL2/SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

void setup_macos_window_style(SDL_Window* window);
void minimize_macos_window(SDL_Window* window);

#ifdef __cplusplus
}
#endif

#endif // IMGUI_MACOS_UTILS_H


