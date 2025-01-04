#ifndef GAME_H
#define GAME_H

#define GRID_WIDTH 20
#define GRID_HEIGHT 10

typedef struct {
    int x, y;
} Position;

typedef struct {
    Position body[100];
    int length;
    Position direction;
} Snake;

typedef struct {
    Snake snake;
    Position fruit;
    int score;
    int is_running;
} GameState;

// Funkcie na manipuláciu s hrou
void initialize_game(GameState *state);
void update_game(GameState *state);
int check_collision(const GameState *state);
void move_snake(GameState *state);
void spawn_fruit(GameState *state);

#endif

