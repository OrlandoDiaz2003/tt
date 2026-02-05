 #include <ncursesw/curses.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>

#define GRID_COLUMN 10
#define GRID_ROW 20
#define GRID_SIZE GRID_COLUMN * GRID_ROW

#define TETRIMINO_SIZE 4

#define CHAR_OFFSET 2
#define OFFSET_BORDER(limit) (GRID_COLUMN + CHAR_OFFSET) + limit
#define OFFSET_IN(limit) (CHAR_OFFSET + (limit * CHAR_OFFSET))

#define CENTER_Y ((LINES - GRID_ROW)/2)
#define CENTER_X ((COLS - GRID_COLUMN)/2) - GRID_COLUMN

#define INITIAL_POS_X (GRID_COLUMN - TETRIMINO_SIZE)/2

int grid [GRID_ROW * GRID_COLUMN] = {0};

//TETRIMINOS
typedef enum {
    TETRI_O,
    TETRI_I,
    TETRI_S,
    TETRI_Z,
    TETRI_L,
    TETRI_J,
    TETRI_T
} Type_Tetrimino;

typedef enum {
    COLLISION,
    FLOOR_COLLISION,
    NO_COLLISION,
    CEILING_COLLISION
} Collision_Type;

typedef struct {
    int positions[TETRIMINO_SIZE];
    int shape[TETRIMINO_SIZE][TETRIMINO_SIZE];
    int pos_x;
    int pos_y;
    Type_Tetrimino type;
} Actual_Tetrimino;

int tetri_o [TETRIMINO_SIZE][TETRIMINO_SIZE] = {
    {0,1,1,0},
    {0,1,1,0},
    {0,0,0,0},
    {0,0,0,0}
};

int tetri_i [TETRIMINO_SIZE][TETRIMINO_SIZE] = {
    {0,0,0,0},
    {1,1,1,1},
    {0,0,0,0},
    {0,0,0,0}
};

int tetri_s [TETRIMINO_SIZE][TETRIMINO_SIZE] = {
    {0,1,1,0},
    {1,1,0,0},
    {0,0,0,0},
    {0,0,0,0}
};

int tetri_z [TETRIMINO_SIZE][TETRIMINO_SIZE] = {
    {1,1,0,0},
    {0,1,1,0},
    {0,0,0,0},
    {0,0,0,0}
};

int tetri_j [TETRIMINO_SIZE][TETRIMINO_SIZE] = {
    {0,0,1,0},
    {1,1,1,0},
    {0,0,0,0},
    {0,0,0,0}
};

int tetri_l [TETRIMINO_SIZE][TETRIMINO_SIZE] = {
    {1,0,0,0},
    {1,1,1,0},
    {0,0,0,0},
    {0,0,0,0}
};

int tetri_t [TETRIMINO_SIZE][TETRIMINO_SIZE] = {
    {0,1,0,0},
    {1,1,1,0},
    {0,0,0,0},
    {0,0,0,0}
};

void draw_grid(void)
{
    int limit = 0;
    int line = 0;
    for (int i = 0; i < (GRID_ROW * GRID_COLUMN); i ++){
        if(limit == GRID_COLUMN) {
            mvprintw(CENTER_Y + line,CENTER_X + OFFSET_BORDER(limit), "!>");
            limit = 0;
            line ++;
        }
        if (limit == 0) mvprintw(CENTER_Y + line, CENTER_X + limit, "<!");

        if (grid[i] > 0) {
            mvprintw(CENTER_Y + line, CENTER_X + OFFSET_IN(limit), "[]");
        } else {
            mvprintw(CENTER_Y + line,CENTER_X + OFFSET_IN(limit), ". ");
        }
        limit ++;
    }
    mvprintw(CENTER_Y + line, CENTER_X + OFFSET_BORDER(limit), "!>");
    mvprintw(CENTER_Y + line + 1, CENTER_X, "<!====================!>");
}

void get_tetro_pos(Actual_Tetrimino *at)
{
    int index = 0;
    for (int y = 0; y < TETRIMINO_SIZE; y ++) {
        for (int x = 0; x < TETRIMINO_SIZE; x ++) {
            if(at->shape[y][x] == 0) continue;
            int tetro_pos_x = (INITIAL_POS_X + x + at->pos_x);
            int tetro_pos_y = (y + at->pos_y) * GRID_COLUMN;
            if(at->type == TETRI_I) tetro_pos_y -= GRID_COLUMN;

            at->positions[index++] = tetro_pos_x + tetro_pos_y;
        }
    }
}

void render_tetrimino(Actual_Tetrimino *at)
{
    for(int i = 0; i < TETRIMINO_SIZE; i++){
        int render_pos_x = at->positions[i] % GRID_COLUMN;
        int render_pos_y = at->positions[i] / GRID_COLUMN;

       mvprintw(CENTER_Y + render_pos_y,  CENTER_X + OFFSET_IN(render_pos_x),"[]");
    }
}

Collision_Type get_collision(Actual_Tetrimino * at,int temp_x, int temp_y)
{
    for(int i = 0; i < TETRIMINO_SIZE; i ++){
        int pos_x = (at->positions[i] % GRID_COLUMN) + temp_x;
        int pos_y = (at->positions[i] / GRID_COLUMN) + temp_y;

        int grid_position = (pos_y * GRID_COLUMN) + pos_x;

        if(pos_x >= GRID_COLUMN)                    return COLLISION;
        if(pos_x < 0)                               return COLLISION;
        if(pos_y >= GRID_ROW)                       return FLOOR_COLLISION;
        if(pos_y < 0)                               return CEILING_COLLISION;
        if(grid[grid_position] != 0) return (temp_y != 0) ? FLOOR_COLLISION: COLLISION;
    }
    return NO_COLLISION;
}

bool is_valid_position(Actual_Tetrimino *at, int rotated_shape[TETRIMINO_SIZE][TETRIMINO_SIZE], int temp_x)
{
    bool is_valid = true;
    for (int y = 0; y < TETRIMINO_SIZE; y ++) {
        for (int x = 0; x < TETRIMINO_SIZE; x ++) {
            if(rotated_shape[y][x] == 0) continue;

            int tetro_pos_x = (INITIAL_POS_X + x + temp_x);
            int tetro_pos_y = (y + at->pos_y) * GRID_COLUMN;
            if(tetro_pos_y > 0 && at->type == TETRI_I) tetro_pos_y -= GRID_COLUMN;

            if(tetro_pos_x < 0 || tetro_pos_x  > GRID_COLUMN - 1) return false;
            if(grid[tetro_pos_y + tetro_pos_x] > 0)               return false;
        }
    }
    return is_valid;
}

void get_rotate_tetrimino(Actual_Tetrimino *at, int rotated_shape[TETRIMINO_SIZE][TETRIMINO_SIZE])
{
    int size = (at->type == TETRI_I) ? 4 : 3;
    for(int y = 0; y < size; y ++){
        for(int x = 0; x < size; x ++){
            rotated_shape[y][size - x - 1] = at->shape[x][y];
        }
    }
}

void apply_rotation(Actual_Tetrimino *at, int rotated_shape[TETRIMINO_SIZE][TETRIMINO_SIZE])
{
    int size = (at->type == TETRI_I) ? 4 : 3;
    for(int y = 0; y < size; y ++){
        for(int x = 0; x < size; x ++){
            at->shape[y][x] = rotated_shape[y][x];
        }
    }
}

int wall_kick(Actual_Tetrimino *at, int rotated_shape[TETRIMINO_SIZE][TETRIMINO_SIZE]){
    int temp_x = at->pos_x;
    for(int i = 0; i < TETRIMINO_SIZE; i ++){
        if(is_valid_position(at, rotated_shape, temp_x - i)){
            at->pos_x -= i;
            apply_rotation(at, rotated_shape);
            return 0;
        }
        if(is_valid_position(at, rotated_shape, temp_x + i)){
            at->pos_x += i;
            apply_rotation(at, rotated_shape);
            return 0;
        }
    }
    return 1;
}

void lock_in(Actual_Tetrimino * at)
{
    for(int i = 0; i < TETRIMINO_SIZE; i++){
        grid[at->positions[i]] = 1;
    }
}

Collision_Type move_tetris(Actual_Tetrimino *at)
{
    int rotated_shape[TETRIMINO_SIZE][TETRIMINO_SIZE] = {0};
    int temp_x = 0;
    int temp_y = 0;

    int key = getch();
    switch(key)
    {
        case KEY_LEFT:
            temp_x -= 1;
            break;
        case KEY_RIGHT:
            temp_x += 1;
            break;
        case KEY_DOWN:
            temp_y += 1;
            break;
        case KEY_UP:
            temp_y -= 1;
            break;
        case ' ':
            if(at->type == TETRI_O) break;
            get_rotate_tetrimino(at,rotated_shape);
            wall_kick(at,rotated_shape);
            break;
    }
    Collision_Type collision = get_collision(at,temp_x, temp_y);

    if(collision == NO_COLLISION){
        at->pos_x += temp_x;
        at->pos_y += temp_y;
    }
    return collision;
}

void reset_actual_tetrimino(Actual_Tetrimino *at)
{
    at->pos_y = 0;
    at->pos_x = 0;
    at->type = rand() % 7;
    memset(at->positions, 0,sizeof(at->positions));
}

bool is_fill(int line){
    for(int i = 0; i < GRID_COLUMN; i ++){
        if(grid[line * GRID_COLUMN + i] == 0) return false;
    }
    return true;
}

void reset_grid(int fill_line)
{
    for(int current_line = fill_line; current_line > 0; current_line --){
        for(int column = 0; column < GRID_COLUMN; column ++){
            grid[current_line * GRID_COLUMN + column] = grid[(current_line -  1) * GRID_COLUMN + column];
        }
    }
}

void clean_fill_line(void)
{
    for(int lines = 0; lines < GRID_ROW; lines ++){
        if(is_fill(lines) == false){
            continue;
        }
        if(is_fill(lines)){
            for(int i = 0; i < GRID_COLUMN; i ++){
                grid[lines * GRID_COLUMN + i] = 0;
            }
            reset_grid(lines);
        }
    }
}

void get_tetrimino_type(Actual_Tetrimino *at)
{
    switch(at->type)
    {
        case TETRI_O:
            memcpy(at->shape, tetri_o,sizeof(tetri_o));
            break;
        case TETRI_I:
            memcpy(at->shape, tetri_i,sizeof(tetri_i));
            break;
        case TETRI_S:
            memcpy(at->shape, tetri_s, sizeof(tetri_s));
            break;
        case TETRI_Z:
            memcpy(at->shape, tetri_z,sizeof(tetri_z));
            break;
        case TETRI_J:
            memcpy(at->shape, tetri_j,sizeof(tetri_j));
            break;
        case TETRI_L:
            memcpy(at->shape, tetri_l,sizeof(tetri_l));
            break;
        case TETRI_T:
            memcpy(at->shape, tetri_t,sizeof(tetri_t));
            break;
    }
}

int main(void)
{
    initscr();
    curs_set(0);
    noecho();
    cbreak();
    setlocale(LC_ALL, "");
    keypad(stdscr, TRUE);
    Actual_Tetrimino at= {0};
    at.type = TETRI_S;
    get_tetrimino_type(&at);
    for(;;) {
        clear();
        clean_fill_line();
        draw_grid();
        get_tetro_pos(&at);
        render_tetrimino(&at);
        if(move_tetris(&at) == FLOOR_COLLISION) {
            lock_in(&at);
            reset_actual_tetrimino(&at);
            get_tetrimino_type(&at);
            continue;
        }
    }
    endwin();
    return 0;
}
