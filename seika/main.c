#include "seika.h"

int main() {
    if (!ska_init_all("test", 800, 600, 800, 600)) {
        return -1;
    }

    while (ska_is_running()) {
        ska_update();
    }

    ska_shutdown_all();

    return 0;
}
