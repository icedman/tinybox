#include "menu.h"

static struct tinybox_menu menu = { 0 };

int main(int argc, const char **argv) {
    if (!menu_setup(&menu)) {
        return -1;
    }

    menu_run(&menu);
    return 0;
}
