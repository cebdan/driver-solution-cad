#import <Cocoa/Cocoa.h>
#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

// Window delegate for specific window (Solution manager)
@interface SolutionManagerWindowDelegate : NSObject <NSWindowDelegate>
@end

@implementation SolutionManagerWindowDelegate

- (void)windowDidBecomeKey:(NSNotification *)notification {
    NSWindow* window = [notification object];
    if (window && [[window title] isEqualToString:@"Solution manager"]) {
        NSUInteger styleMask = [window styleMask];
        // Remove close button (red), keep minimize button (yellow)
        styleMask &= ~NSWindowStyleMaskClosable;
        styleMask |= NSWindowStyleMaskMiniaturizable;
        [window setStyleMask:styleMask];
        [window display];
    }
}

- (void)windowDidUpdate:(NSNotification *)notification {
    NSWindow* window = [notification object];
    if (window && [[window title] isEqualToString:@"Solution manager"]) {
        NSUInteger styleMask = [window styleMask];
        if (styleMask & NSWindowStyleMaskClosable) {
            styleMask &= ~NSWindowStyleMaskClosable;
            styleMask |= NSWindowStyleMaskMiniaturizable;
            [window setStyleMask:styleMask];
        }
    }
}

@end

static SolutionManagerWindowDelegate* g_solution_manager_delegate = nil;

// Function to hide close and maximize buttons on all application windows, but keep minimize button
extern "C" void hide_close_button_all_windows() {
    @autoreleasepool {
        NSArray* windows = [[NSApplication sharedApplication] windows];
        for (NSWindow* window in windows) {
            NSUInteger styleMask = [window styleMask];
            // Remove close button (red)
            styleMask &= ~NSWindowStyleMaskClosable;
            // Ensure minimize button is enabled (yellow)
            styleMask |= NSWindowStyleMaskMiniaturizable;
            [window setStyleMask:styleMask];
            [window display];
        }
    }
}

// Function to hide close button on macOS window by window title
extern "C" void hide_close_button_by_title(const char* window_title) {
    @autoreleasepool {
        // Get all windows of the application
        NSArray* windows = [[NSApplication sharedApplication] windows];
        
        NSString* targetTitle = [NSString stringWithUTF8String:window_title];
        
        // Try to find window by exact title match
        BOOL found = NO;
        for (NSWindow* window in windows) {
            NSString* title = [window title];
            if (title && [title isEqualToString:targetTitle]) {
                // Remove close button (red), keep minimize button (yellow)
                NSUInteger styleMask = [window styleMask];
                // Remove close button
                styleMask &= ~NSWindowStyleMaskClosable;
                // Ensure minimize button is enabled
                styleMask |= NSWindowStyleMaskMiniaturizable;
                [window setStyleMask:styleMask];
                // Force window to update
                [window display];
                found = YES;
            }
        }
        
        // If not found by exact title, try to find main window or key window
        if (!found) {
            NSWindow* keyWindow = [[NSApplication sharedApplication] keyWindow];
            if (keyWindow) {
                NSUInteger styleMask = [keyWindow styleMask];
                styleMask &= ~NSWindowStyleMaskClosable;
                styleMask |= NSWindowStyleMaskMiniaturizable;
                [keyWindow setStyleMask:styleMask];
                [keyWindow display];
            }
            
            // Also try main window
            NSWindow* mainWindow = [[NSApplication sharedApplication] mainWindow];
            if (mainWindow && mainWindow != keyWindow) {
                NSUInteger styleMask = [mainWindow styleMask];
                styleMask &= ~NSWindowStyleMaskClosable;
                styleMask |= NSWindowStyleMaskMiniaturizable;
                [mainWindow setStyleMask:styleMask];
                [mainWindow display];
            }
        }
        
        // Also try to hide on all windows as fallback
        hide_close_button_all_windows();
    }
}

// Function to continuously hide close button - call this periodically
extern "C" void force_hide_close_buttons() {
    @autoreleasepool {
        // Get all windows and force hide close button
        NSArray* windows = [[NSApplication sharedApplication] windows];
        for (NSWindow* window in windows) {
            // Check if window is visible and has a title
            if ([window isVisible] && [[window title] length] > 0) {
                NSUInteger styleMask = [window styleMask];
                // Force remove close button
                if (styleMask & NSWindowStyleMaskClosable) {
                    styleMask &= ~NSWindowStyleMaskClosable;
                    styleMask |= NSWindowStyleMaskMiniaturizable; // Ensure minimize is enabled
                    [window setStyleMask:styleMask];
                    [window display];
                }
            }
        }
    }
}

// Function to set window style immediately when window is created (before showing)
// This should be called right after creating the form but before showing it
extern "C" void setup_window_style_no_close_button(void* window_handle) {
    @autoreleasepool {
        if (window_handle) {
            NSWindow* window = (__bridge NSWindow*)window_handle;
            if (window) {
                NSUInteger styleMask = [window styleMask];
                // Remove close button (red), keep minimize button (yellow)
                styleMask &= ~NSWindowStyleMaskClosable;
                styleMask |= NSWindowStyleMaskMiniaturizable;
                [window setStyleMask:styleMask];
            }
        }
    }
}

// Function to setup style for Solution manager window only
extern "C" void setup_solution_manager_window_style() {
    @autoreleasepool {
        if (!g_solution_manager_delegate) {
            g_solution_manager_delegate = [[SolutionManagerWindowDelegate alloc] init];
        }
        
        // Find and setup only "Solution manager" window
        NSArray* windows = [[NSApplication sharedApplication] windows];
        NSString* targetTitle = @"Solution manager";
        
        BOOL found = NO;
        for (NSWindow* window in windows) {
            NSString* windowTitle = [window title];
            if (windowTitle && [windowTitle isEqualToString:targetTitle]) {
                // Set delegate only for this specific window
                [window setDelegate:g_solution_manager_delegate];
                // Set style immediately - force it
                NSUInteger styleMask = [window styleMask];
                // Remove close button (red) and resizable (green), keep minimize (yellow)
                styleMask &= ~NSWindowStyleMaskClosable;
                styleMask &= ~NSWindowStyleMaskResizable;  // This removes green button
                styleMask |= NSWindowStyleMaskMiniaturizable;
                [window setStyleMask:styleMask];
                [window display];
                // Force update
                [window invalidateShadow];
                found = YES;
            }
        }
        
        // If not found, try key window or main window
        if (!found) {
            NSWindow* keyWindow = [[NSApplication sharedApplication] keyWindow];
            if (keyWindow) {
                [keyWindow setDelegate:g_solution_manager_delegate];
                NSUInteger styleMask = [keyWindow styleMask];
                styleMask &= ~NSWindowStyleMaskClosable;
                styleMask &= ~NSWindowStyleMaskResizable;
                styleMask |= NSWindowStyleMaskMiniaturizable;
                [keyWindow setStyleMask:styleMask];
                [keyWindow display];
                [keyWindow invalidateShadow];
            }
        }
    }
}

// Function to minimize window by title
extern "C" void minimize_window_by_title(const char* window_title) {
    @autoreleasepool {
        NSArray* windows = [[NSApplication sharedApplication] windows];
        NSString* targetTitle = [NSString stringWithUTF8String:window_title];
        
        for (NSWindow* window in windows) {
            NSString* title = [window title];
            if (title && [title isEqualToString:targetTitle]) {
                [window miniaturize:nil];
                return;
            }
        }
        
        // If not found, try key window and main window
        NSWindow* keyWindow = [[NSApplication sharedApplication] keyWindow];
        if (keyWindow && [[keyWindow title] isEqualToString:targetTitle]) {
            [keyWindow miniaturize:nil];
            return;
        }
        
        NSWindow* mainWindow = [[NSApplication sharedApplication] mainWindow];
        if (mainWindow && mainWindow != keyWindow && [[mainWindow title] isEqualToString:targetTitle]) {
            [mainWindow miniaturize:nil];
        }
    }
}
