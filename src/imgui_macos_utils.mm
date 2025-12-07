#ifdef __APPLE__
#import <Cocoa/Cocoa.h>
#import <SDL2/SDL_syswm.h>

extern "C" void setup_macos_window_style(SDL_Window* window) {
    if (!window) return;
    
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(window, &wmInfo);
    
    if (wmInfo.subsystem == SDL_SYSWM_COCOA) {
        NSWindow* nsWindow = wmInfo.info.cocoa.window;
        if (nsWindow) {
            NSUInteger styleMask = [nsWindow styleMask];
            // Remove close button (red) and resizable (green), keep minimize (yellow)
            styleMask &= ~NSWindowStyleMaskClosable;
            styleMask &= ~NSWindowStyleMaskResizable;
            styleMask |= NSWindowStyleMaskMiniaturizable;
            [nsWindow setStyleMask:styleMask];
            [nsWindow display];
        }
    }
}

extern "C" void minimize_macos_window(SDL_Window* window) {
    if (!window) return;
    
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(window, &wmInfo);
    
    if (wmInfo.subsystem == SDL_SYSWM_COCOA) {
        NSWindow* nsWindow = wmInfo.info.cocoa.window;
        if (nsWindow) {
            [nsWindow miniaturize:nil];
        }
    }
}
#else
#include <SDL2/SDL.h>

extern "C" void setup_macos_window_style(SDL_Window* window) {
    (void)window; // Unused on non-macOS
}

extern "C" void minimize_macos_window(SDL_Window* window) {
    if (window) {
        SDL_MinimizeWindow(window);
    }
}
#endif

