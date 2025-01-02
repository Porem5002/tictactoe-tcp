#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "game.h"

static bool board_check_line(const board_t* b, int y, player_t* winner)
{
    player_t p = board_get_cell(b, 0, y);
    if(p == NO_PLAYER) return false;

    for(int x = 1; x < b->size; x++)
        if(p != board_get_cell(b, x, y))
            return false;
        
    if(winner != NULL) *winner = p;
    return true;
}

static bool board_check_column(const board_t* b, int x, player_t* winner)
{
    player_t p = board_get_cell(b, x, 0);
    if(p == NO_PLAYER) return false;

    for(int y = 1; y < b->size; y++)
        if(p != board_get_cell(b, x, y))
            return false;
    
    if(winner != NULL) *winner = p;
    return true;
}

static bool board_check_diagonal(const board_t* b, bool is_main, player_t* winner)
{
    player_t p = is_main ? board_get_cell(b, 0, 0) : board_get_cell(b, 0, b->size-1);
    if(p == NO_PLAYER) return false;

    for(int i = 1; i < b->size; i++)
    {
        int x = i;
        int y = is_main ? i : b->size - i - 1;

        if(p != board_get_cell(b, x, y))
            return false;
    }

    if(winner != NULL) *winner = p;
    return true;
}

board_t board_make(int size)
{
    board_t b;
    b.size = size;

    b.cells = malloc(sizeof(player_t) * size * size);
    if(b.cells == NULL)
    {
        fprintf(stderr, "ERROR: Ran out of memory\n");
        exit(EXIT_FAILURE);
    }

    board_clear(&b);
    return b;
}

board_t board_copy(const board_t* b)
{
    board_t new_b = board_make(b->size);
    memcpy(new_b.cells, b->cells, sizeof(player_t) * b->size * b->size);
    return new_b;
}

void board_clear(board_t* b)
{
    for(int i = 0; i < b->size * b->size; i++)
        b->cells[i] = NO_PLAYER;
}

player_t board_get_cell(const board_t* b, int x, int y)
{
    assert(x >= 0 && x < b->size);
    assert(y >= 0 && y < b->size);
    return b->cells[x + y * b->size]; 
}

void board_set_cell(board_t* b, int x, int y, player_t value)
{
    assert(x >= 0 && x < b->size);
    assert(y >= 0 && y < b->size);
    b->cells[x + y * b->size] = value; 
}

bool board_is_final(const board_t* b, player_t* winner)
{
    // Check lines
    for(int y = 0; y < b->size; y++)
        if(board_check_line(b, y, winner))
            return true;

    // Check columns
    for(int x = 0; x < b->size; x++)
        if(board_check_column(b, x, winner))
            return true;

    // Check diagonals
    if(board_check_diagonal(b, true, winner))
        return true;

    if(board_check_diagonal(b, false, winner))
        return true;

    for(int i = 0; i < b->size * b->size; i++)
        if(b->cells[i] == NO_PLAYER)
            return false;

    if(winner != NULL) *winner = NO_PLAYER;
    return true;
}

void board_free(board_t* b)
{
    b->size = 0;
    free(b->cells);
    b->cells = NULL;
}

game_t game_make(int board_size, player_t starting_player)
{
    game_t game = {
        .running = true,
        .starting_player = starting_player,
        .curr_player = starting_player,
        .board = board_make(board_size)
    };

    return game;
}

void game_restart(game_t* game)
{
    game->running = true;
    game->curr_player = game->starting_player;
    board_clear(&game->board);
}

bool game_do_move(game_t* game, int x, int y)
{
    if(game->running && board_get_cell(&game->board, x, y) == NO_PLAYER)
    {
        board_set_cell(&game->board, x, y, game->curr_player);
        game->curr_player = game->curr_player == PLAYER_1 ? PLAYER_2 : PLAYER_1;
        
        if(board_is_final(&game->board, NULL))
            game->running = false;
        
        return true;
    }

    return false;
}

void game_free(game_t* game)
{
    game->running = false;
    game->starting_player = NO_PLAYER;
    game->curr_player = NO_PLAYER;
    board_free(&game->board);
}
