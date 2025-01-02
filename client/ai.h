#ifndef AI_H
#define AI_H

#include "../shared/game.h"

void ai_next_move(const board_t* board, int* out_x, int* out_y, player_t player);

#endif
