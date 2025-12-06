#ifndef CLOSE_APP_H
#define CLOSE_APP_H

#include <xtd/xtd>

class close_app {
public:
    // Close application properly
    static void close();
    
private:
    close_app() = delete; // Prevent instantiation
    ~close_app() = delete;
};

#endif // CLOSE_APP_H

