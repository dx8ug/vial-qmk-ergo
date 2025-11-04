#include "display.h"
#include "ergohaven.h"
#include "ergohaven_display.h"

void housekeeping_task_user(void) {
    display_housekeeping_task();
}

void keyboard_post_init_user(void) {
    display_init_kb();
}
