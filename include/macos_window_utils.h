#ifndef MACOS_WINDOW_UTILS_H
#define MACOS_WINDOW_UTILS_H

#ifdef __APPLE__
// Function to hide close button on all application windows
extern "C" void hide_close_button_all_windows();

// Function to hide close button on macOS window by window title
extern "C" void hide_close_button_by_title(const char* window_title);

// Function to force hide close buttons on all visible windows
extern "C" void force_hide_close_buttons();

// Function to set window style immediately when window is created (before showing)
extern "C" void setup_window_style_no_close_button(void* window_handle);

// Function to setup style for Solution manager window only (not for all windows)
extern "C" void setup_solution_manager_window_style();
#endif

#endif // MACOS_WINDOW_UTILS_H

