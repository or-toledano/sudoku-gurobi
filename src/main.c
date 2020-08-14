#include <time.h>
#include <stdlib.h>
#include "game.h"
#include "main_aux.h"

int main() {
    game_t *game = game_ctor();
    srand((unsigned int) time(NULL));
    parse_and_dispatch_commands(game);
    return 0;
}
