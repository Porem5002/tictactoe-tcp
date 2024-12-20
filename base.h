#ifndef BASE_H
#define BASE_H 

#include <stdbool.h>

typedef enum
{
    NO_PLAYER,
    PLAYER_1,
    PLAYER_2,
} player_t;

typedef struct
{
    int size;
    player_t* cells;
} board_t;

typedef struct
{
    bool running;
    player_t starting_player;

    player_t curr_player;
    board_t board;
} game_t;

board_t board_make(int size);
board_t board_copy(const board_t* b);
void board_clear(board_t* b);
player_t board_get_cell(const board_t* b, int x, int y);
void board_set_cell(board_t* b, int x, int y, player_t value);
bool board_is_final(const board_t* b, player_t* winner);
void board_free(board_t* b);

game_t game_make(int board_size, player_t starting_player);
void game_restart(game_t* game);
bool game_do_move(game_t* game, int x, int y);
void game_free(game_t* game);

#endif
