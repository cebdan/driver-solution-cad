#include "../include/close_app.h"
#include <xtd/xtd>
#include <cstdlib>

using namespace xtd;
using namespace xtd::forms;

void close_app::close() {
    // Force application to exit immediately
    // This is the most reliable way to close the application
    std::exit(0);
}

