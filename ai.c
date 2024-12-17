#include <assert.h>
#include <stdlib.h>
#include <limits.h>

#include "ai.h"

typedef struct
{
    int cost;
    int x;
    int y;
} ai_minimax_result_t;

static ai_minimax_result_t ai_next_move_minimax(const board_t* b, player_t curr_player, player_t max_player);

void ai_next_move(const board_t* b, int* out_x, int* out_y, player_t player)
{
    assert(out_x != NULL);
    assert(out_y != NULL);
    
    ai_minimax_result_t r = ai_next_move_minimax(b, player, player);
    *out_x = r.x;
    *out_y = r.y;
}

static ai_minimax_result_t ai_next_move_minimax(const board_t* b, player_t curr_player, player_t max_player)
{
    player_t winner;

    if(board_is_final(b, &winner))
    {
        ai_minimax_result_t r = {0};
        r.x = -1;
        r.y = -1;

        if(winner == NO_PLAYER)
        {
            r.cost = 0;
            return r;
        }

        r.cost = winner == max_player ? 1 : -1;
        return r; 
    }

    bool is_maximizing = curr_player == max_player;

    ai_minimax_result_t best_r = {0};
    best_r.cost = is_maximizing ? INT_MIN : INT_MAX;

    player_t opponent = curr_player == PLAYER_1 ?  PLAYER_2 : PLAYER_1;
    board_t new_b = board_copy(b);

    for(int y = 0; y < b->size; y++)
    {   
        for(int x = 0; x < b->size; x++)
        {
            if(board_get_cell(b, x, y) != NO_PLAYER) continue;

            board_set_cell(&new_b, x, y, curr_player);
            ai_minimax_result_t r = ai_next_move_minimax(&new_b, opponent, max_player);
            board_set_cell(&new_b, x, y, NO_PLAYER);

            if((is_maximizing && r.cost > best_r.cost) || (!is_maximizing && r.cost < best_r.cost))
            {
                best_r.cost = r.cost;
                best_r.x = x;
                best_r.y = y;
            }
        }
    }

    board_free(&new_b);
    return best_r;
}
