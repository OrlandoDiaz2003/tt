#include <ncursesw/curses.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>

#define GRID_COLUMN 10
#define GRID_ROW 20
#define GRID_SIZE (GRID_COLUMN * GRID_ROW)

#define TTM_SIZE 4

#define CHAR_OFFSET 2
#define OFFSET_BORDER(limit) (GRID_COLUMN + CHAR_OFFSET) + limit
#define OFFSET_IN(limit) (CHAR_OFFSET + (limit * CHAR_OFFSET))

#define CENTER_Y ((LINES - GRID_ROW)/2)
#define CENTER_X ((COLS - GRID_COLUMN)/2) - GRID_COLUMN

#define TOTAL_TTM 7
#define INITIAL_POS_X (GRID_COLUMN - TTM_SIZE)/2

bool term_colors = false;

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
    DEFAULT,
    RED,
    MAGENTA,
    GREEN,
    YELLOW,
    ORANGE,
    BLUE,
    CYAN,
    BORDERS
} Color;

typedef enum {
    LEFT_COLLISION,
    RIGHT_COLLISION,
    FLOOR_COLLISION,
    NO_COLLISION,
    COLLISION,
    CEILING_COLLISION
} Collision_Type;

typedef struct{
    int matrix[TTM_SIZE][TTM_SIZE];
    Type_Tetrimino type;
    Color color;
} Tetrimino;

typedef struct {
    int positions[TTM_SIZE];
    Tetrimino ttm;
    int pos_x;
    int pos_y;
} Actual_Tetrimino;

const Tetrimino tetriminos[TOTAL_TTM] =
{
    {
        .type = TETRI_I,
        .color = CYAN,
        .matrix = {
            {0,0,0,0},
            {1,1,1,1},
            {0,0,0,0},
            {0,0,0,0}
        }
    },
    {
        .type = TETRI_O,
        .color = YELLOW,
        .matrix = {
            {0,1,1,0},
            {0,1,1,0},
            {0,0,0,0},
            {0,0,0,0}
        }
    },
    {
        .type = TETRI_J,
        .color = BLUE,
        .matrix = {
            {1,0,0,0},
            {1,1,1,0},
            {0,0,0,0},
            {0,0,0,0}
        }
    },
    {
        .type= TETRI_L,
        .color = ORANGE,
        .matrix = {
            {0,0,1,0},
            {1,1,1,0},
            {0,0,0,0},
            {0,0,0,0}
        }
    },
    {
        .type = TETRI_Z,
        .color = RED,
        .matrix = {
            {1,1,0,0},
            {0,1,1,0},
            {0,0,0,0},
            {0,0,0,0}
        }
    },
    {
        .type = TETRI_S,
        .color = GREEN,
        .matrix = {
            {0,1,1,0},
            {1,1,0,0},
            {0,0,0,0},
            {0,0,0,0}
        }
    },
    {
        .type = TETRI_T,
        .color = MAGENTA,
        .matrix ={
            {0,1,0,0},
            {1,1,1,0},
            {0,0,0,0},
            {0,0,0,0}
        }
    }
};
void color_on(Color color)
{
    if(term_colors) attron(COLOR_PAIR(color));
}
void color_off(Color color)
{
    if(term_colors)attroff(COLOR_PAIR(color));
}

void draw_grid(void)
{
    int limit = 0;
    int line = 0;
    for (int i = 0; i < (GRID_ROW * GRID_COLUMN); i ++){
        color_on(BORDERS);
        if(limit == GRID_COLUMN) {

            mvprintw(CENTER_Y + line,CENTER_X + OFFSET_BORDER(limit), "!>");
            limit = 0;
            line ++;
            color_on(BORDERS);
        }
        if (limit == 0) {

            mvprintw(CENTER_Y + line, CENTER_X + limit, "<!");

        }

        if (grid[i] > 0) {
            color_on(grid[i]);
            mvprintw(CENTER_Y + line, CENTER_X + OFFSET_IN(limit), "[]");
            color_off(BORDERS);
        } else {
            color_off(BORDERS);
            mvprintw(CENTER_Y + line,CENTER_X + OFFSET_IN(limit), ". ");
        }
        color_on(BORDERS);
        limit ++;
    }

    mvprintw(CENTER_Y + line, CENTER_X + OFFSET_BORDER(limit), "!>");
    mvprintw(CENTER_Y + line + 1, CENTER_X, "<!====================!>");
    color_off(BORDERS);
}

void get_tetro_pos(Actual_Tetrimino *at)
{
    int index = 0;
    for (int y = 0; y < TTM_SIZE; y ++) {
        for (int x = 0; x < TTM_SIZE; x ++) {
            if(at->ttm.matrix[y][x] == 0) continue;
            int tetro_pos_x = (INITIAL_POS_X + x + at->pos_x);
            int tetro_pos_y = (y + at->pos_y) * GRID_COLUMN;
            if(at->ttm.type == TETRI_I && tetro_pos_y > 0) tetro_pos_y -= GRID_COLUMN;

            at->positions[index++] = tetro_pos_x + tetro_pos_y;
        }
    }
}

void render_tetrimino(int positions[TTM_SIZE],Color color)
{
    for(int i = 0; i < TTM_SIZE; i++){
        int render_pos_x = positions[i] % GRID_COLUMN;
        int render_pos_y = positions[i] / GRID_COLUMN;
        color_on(color);
        mvprintw(CENTER_Y + render_pos_y,  CENTER_X + OFFSET_IN(render_pos_x),"[]");
        color_off(color);

    }
}

Collision_Type get_collision(int positions[TTM_SIZE],int temp_x, int temp_y)
{
    for(int i = 0; i < TTM_SIZE; i ++){
        int pos_x = (positions[i] % GRID_COLUMN) + temp_x;
        int pos_y = (positions[i] / GRID_COLUMN) + temp_y;

        int grid_position = (pos_y * GRID_COLUMN) + pos_x;

        if(pos_x >= GRID_COLUMN)                    return RIGHT_COLLISION;
        if(pos_x < 0)                               return LEFT_COLLISION;
        if(pos_y >= GRID_ROW)                       return FLOOR_COLLISION;
        if(pos_y < 0)                               return CEILING_COLLISION;
        if(grid[grid_position] != 0) return (temp_y != 0) ? FLOOR_COLLISION: COLLISION;
    }
    return NO_COLLISION;
}

Collision_Type is_valid_position(int rotated_shape[TTM_SIZE][TTM_SIZE], int temp_x, int temp_y)
{
    int positions_rotated[TTM_SIZE] = {0};
    int index = 0;
    for (int y = 0; y < TTM_SIZE; y ++) {
        for (int x = 0; x < TTM_SIZE; x ++) {
            if(rotated_shape[y][x] == 0) continue;

            int tetro_pos_x = INITIAL_POS_X + x;
            int tetro_pos_y = y * GRID_COLUMN;
            positions_rotated[index++] = tetro_pos_x + tetro_pos_y;
        }
    }
    Collision_Type collision = get_collision(positions_rotated, temp_x, temp_y);
    return collision;
}

void get_rotate_tetrimino(Actual_Tetrimino *at, int rotated_shape[TTM_SIZE][TTM_SIZE])
{
    int size = (at->ttm.type == TETRI_I) ? 4 : 3;
    for(int y = 0; y < size; y ++){
        for(int x = 0; x < size; x ++){
            rotated_shape[y][size - x - 1] = at->ttm.matrix[x][y];
        }
    }
}

void apply_rotation(Actual_Tetrimino *at, int rotated_shape[TTM_SIZE][TTM_SIZE])
{
    int size = (at->ttm.type == TETRI_I) ? 4 : 3;
    for(int y = 0; y < size; y ++){
        for(int x = 0; x < size; x ++){
            at->ttm.matrix[y][x] = rotated_shape[y][x];
        }
    }
}

int wall_kick(Actual_Tetrimino *at, int rotated_shape[TTM_SIZE][TTM_SIZE]){
    int temp_x = at->pos_x;
    int temp_y = at->pos_y;
    for(int i = 0; i < 5; i ++){
        if(is_valid_position(rotated_shape,temp_x,temp_y) == NO_COLLISION){
            at->pos_x = temp_x;
            at->pos_y = temp_y;
            apply_rotation(at,rotated_shape);
            return 0;
        }
        temp_x = at->pos_x;
        temp_y = at->pos_y;
        if(at->ttm.type == TETRI_I){
            switch(i)
            {
                case 1:
                    temp_x -= 2;
                    break;
                case 2:
                    temp_x += 1;
                    break;
                case 3:
                    temp_x -= 1;
                    temp_y -= 2;
                    break;
                case 4:
                    temp_x += 1;
                    temp_y += 2;
                    break;
            }
        }else{
            switch(i)
            {
                case 1:
                    temp_x -= 1;
                    break;
                case 2:
                    temp_x -= 1;
                    temp_y += 1;
                    break;
                case 3:
                    temp_y -= 2;
                    break;
                case 4:
                    temp_x -= 1;
                    temp_y -= 2;
                    break;
                }
            }
        }
    return 1;
}

int hard_drop(Actual_Tetrimino * at)
{
    int temp_y = 0;
    Collision_Type hard_drop_col = NO_COLLISION;
    while(hard_drop_col != FLOOR_COLLISION){
        temp_y += 1;
        hard_drop_col = get_collision(at->positions, 0, temp_y + 1);
    }
    while(get_collision(at->positions, 0, temp_y) != NO_COLLISION){
        temp_y -=1;
    }
    return temp_y;
}

void lock_in(Actual_Tetrimino * at)
{
    for(int i = 0; i < TTM_SIZE; i++){
        grid[at->positions[i]] = at->ttm.color;
    }
}

Collision_Type move_tetris(Actual_Tetrimino *at)
{
    int rotated_shape[TTM_SIZE][TTM_SIZE] = {0};
    int temp_x = 0;
    int temp_y = 0;
    bool hard = false;
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
            if(at->ttm.type == TETRI_O) break;
            get_rotate_tetrimino(at,rotated_shape);
            wall_kick(at,rotated_shape);
            break;
        case 'h':
            temp_y = hard_drop(at);
            hard = true;
            break;
    }
    Collision_Type collision = get_collision(at->positions,temp_x, temp_y);
    if(hard){
        at->pos_x += temp_x;
        at->pos_y += temp_y;
        get_tetro_pos(at);
        return FLOOR_COLLISION;
    }
    if(collision == NO_COLLISION){
        at->pos_x += temp_x;
        at->pos_y += temp_y;
    }
    return collision;
}

void reset_actual_tetrimino(Actual_Tetrimino *at)
{
    int index = rand() % 7;
    at->pos_y = 0;
    at->pos_x = 0;
    at->ttm = tetriminos[index];
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
void terminal_has_colors(void)
{
    if(has_colors()){
        start_color();
        term_colors = true;
        if(can_change_color()){
            // CYAN BG / FG
            init_color(COLOR_CYAN, 0, 1000, 1000);
            init_color(10, 0, 400, 400);

            // BLUE BG / FG
            init_color(COLOR_BLUE, 0, 0, 1000);
            init_color(11, 0, 0, 400);

            // ORANGE BG / FG
            init_color(12, 1000, 500, 0);
            init_color(13, 400, 200, 0);

            // YELLOW BG / FG
            init_color(COLOR_YELLOW, 1000, 1000, 0);
            init_color(14, 400, 400, 0);

            // GREEN BG / FG
            init_color(COLOR_GREEN, 0, 1000, 0);
            init_color(15, 0, 400, 0);

            // PURPLE BG / FG
            init_color(COLOR_MAGENTA, 700, 0, 1000);
            init_color(16, 300, 0, 400);

            // RED BG / FG
            init_color(COLOR_RED, 1000, 0, 0);
            init_color(17, 400, 0, 0);

            init_pair(RED, 17, COLOR_RED);
            init_pair(GREEN, 15,COLOR_GREEN);
            init_pair(YELLOW, 14 , COLOR_YELLOW);
            init_pair(BLUE, 11,COLOR_BLUE);
            init_pair(CYAN, 10, COLOR_CYAN);
            init_pair(MAGENTA, 16,COLOR_MAGENTA);
            init_pair(ORANGE, 13, 12);
            init_pair(BORDERS, COLOR_BLACK,COLOR_WHITE);
        }else{
            init_pair(RED, COLOR_RED,COLOR_RED);
            init_pair(GREEN, COLOR_GREEN,COLOR_GREEN);
            init_pair(YELLOW, COLOR_YELLOW, COLOR_YELLOW);
            init_pair(BLUE,COLOR_BLUE,COLOR_BLUE);
            init_pair(CYAN, COLOR_CYAN, COLOR_CYAN);
            init_pair(MAGENTA, COLOR_MAGENTA,COLOR_MAGENTA);
            init_pair(ORANGE, COLOR_MAGENTA,COLOR_MAGENTA);
            init_pair(BORDERS, COLOR_BLACK,COLOR_WHITE);
        }
    }
}
int main(void)
{
    initscr();
    terminal_has_colors();
    curs_set(0);
    noecho();
    cbreak();
    nodelay(stdscr, TRUE);
    setlocale(LC_ALL, "");
    keypad(stdscr, TRUE);
    Actual_Tetrimino at= {0};
    at.ttm = tetriminos[0];
    for(;;) {
        refresh();
        clean_fill_line();
        draw_grid();
        get_tetro_pos(&at);
        render_tetrimino(at.positions,at.ttm.color);
        if(move_tetris(&at) == FLOOR_COLLISION) {
            lock_in(&at);
            reset_actual_tetrimino(&at);
            continue;
        }
    }
    endwin();
    return 0;
}
