#include <ncursesw/curses.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <unistd.h>
#include <time.h>

#define GRID_COLUMN 10
#define GRID_ROW 20
#define GRID_SIZE (GRID_COLUMN * GRID_ROW)

#define TTM_SIZE 4

#define TTM_WIDTH 2
#define OFFSET_BORDER(limit) (GRID_COLUMN + TTM_WIDTH) + limit
#define OFFSET_IN(limit) (TTM_WIDTH + (limit * TTM_WIDTH))

#define CENTER_Y ((LINES - GRID_ROW)/2)
#define CENTER_X ((COLS - GRID_COLUMN)/2) - GRID_COLUMN

#define NUM_TTM 7
#define INITIAL_POS_X (GRID_COLUMN - TTM_SIZE)/2

#define NUM_LEVELS 30

bool term_colors = false;

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

typedef enum {
    ACT_DOWN,
    ACT_LEFT,
    ACT_RIGHT,
    ACT_ROTATE_RIGHT,
    ACT_ROTATE_LEFT,
    ACT_HARD_DROP,
    ACT_RESIZE,
    ACT_NONE
} Action;

typedef enum {
    SPAWN,
    ROT_RIGHT,
    ROT_HALF,
    ROT_LEFT,
} Rotation_Position;

const int score_per_line[4] = {
    40,100,300,1200
};

const int frames_per_level[NUM_LEVELS] = {
    48, 43, 38, 33, 28, 23, 18, 13, 8, 6, 5, 5, 5, 4, 4, 4,
    3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1
};

const int of_wl[4][5][2] = {
    //SPAWN
    {{0,0},{0,0},{0,0},{0,0},{0,0}},
    //ROT_RIGHT
    {{0,0},{1,0},{1,1},{0,-2},{1,-2}},
    //ROT_HALF
    {{0,0},{0,0},{0,0},{0,0},{0,0}},
    //ROT_LEFT
    {{0,0},{-1,0},{-1,1},{0,-2},{-1,-2}}
};

const int of_wl_ttm_i[4][5][2] = {
    //SPAWN
    {{0,0},{-1,0},{+2,0},{-1,0},{+2,0}},
    //ROT_RIGHT
    {{-1,0},{0,0},{0,0},{0,-1},{0,2}},
    //ROT_HALF
    {{-1,-1},{1,-1},{-2,-1},{1,0},{-2,0}},
    //ROT_LEFT
    {{0,-1},{0,-1},{0,-1},{0,1},{0,-2}}
};

typedef enum {
    GAME,
    MENU,
    GAME_OVER,
    EXIT
} Game_State;

typedef struct{
    int matrix[TTM_SIZE][TTM_SIZE];
    Type_Tetrimino type;
    Color color;
} Tetrimino;

typedef struct {
    int positions[TTM_SIZE];
    Tetrimino ttm;
    Rotation_Position rot_pos;
    int pos_x;
    int pos_y;
} Actual_Tetrimino;

typedef struct {
    int grid [GRID_ROW * GRID_COLUMN];
    Game_State game_state;
    Actual_Tetrimino current_tetrimino;
    Tetrimino next_tetrimino;
    Tetrimino bag[NUM_TTM];
    int bag_index;
    int current_frame;
    int key;
    unsigned int score;
    unsigned int lines_clear;
    unsigned int level;
} State;

const Tetrimino tetriminos[NUM_TTM] =
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
};

void color_on(Color color)
{
    if(term_colors) attron(COLOR_PAIR(color));
}

void color_off(Color color)
{
    if(term_colors) attroff(COLOR_PAIR(color));
}

void draw_grid(int grid[GRID_SIZE])
{
    int column = 0;
    int line = 0;
    for (int i = 0; i < (GRID_SIZE); i ++){
        color_on(BORDERS);
        if(column == GRID_COLUMN) {
            mvprintw(CENTER_Y + line, CENTER_X + OFFSET_BORDER(column), "!>");
            column = 0;
            line ++;
            color_on(BORDERS);
        }
        if (column == 0) {
            mvprintw(CENTER_Y + line, CENTER_X + column, "<!");
        }
        if (grid[i] > 0) {
            color_on(grid[i]);
            mvprintw(CENTER_Y + line, CENTER_X + OFFSET_IN(column), "[]");
            color_off(BORDERS);
        } else {
            color_off(BORDERS);
            mvprintw(CENTER_Y + line,CENTER_X + OFFSET_IN(column), ". ");
        }
        color_on(BORDERS);
        column ++;
    }

    mvprintw(CENTER_Y + line, CENTER_X + OFFSET_BORDER(column), "!>");
    mvprintw(CENTER_Y + line + 1, CENTER_X, "<!====================!>");
    color_off(BORDERS);
}

void get_tetro_pos(int *positions, int shape[TTM_SIZE][TTM_SIZE],int pos_x, int pos_y)
{
    int index = 0;
    for (int y = 0; y < TTM_SIZE; y ++) {
        for (int x = 0; x < TTM_SIZE; x ++) {
            if(shape[y][x] == 0) continue;
            int tetro_pos_x = (INITIAL_POS_X + x + pos_x);
            int tetro_pos_y = (y + pos_y) * GRID_COLUMN;
            positions[index++] = tetro_pos_x + tetro_pos_y;
        }
    }
}

void render(int *positions,Color color,int offset_x, int offset_y)
{
    color_on(color);
    for(int i = 0; i < TTM_SIZE; i++){
        int render_pos_x = positions[i] % GRID_COLUMN;
        int render_pos_y = positions[i] / GRID_COLUMN;
        mvprintw(offset_y + render_pos_y,  offset_x + OFFSET_IN(render_pos_x),"[]");
    }
    color_off(color);
}

Collision_Type get_collision(int grid[GRID_SIZE], int positions[TTM_SIZE], int temp_x, int temp_y)
{
    for(int i = 0; i < TTM_SIZE; i ++){
        int pos_x = (positions[i] % GRID_COLUMN) + temp_x;
        int pos_y = (positions[i] / GRID_COLUMN) + temp_y;

        int grid_position = (pos_y * GRID_COLUMN) + pos_x;
        if(pos_x >= GRID_COLUMN)                    return RIGHT_COLLISION;
        if(pos_x < 0)                               return LEFT_COLLISION;
        if(pos_y >= GRID_ROW)                       return FLOOR_COLLISION;
        if(pos_y < 0)                               return CEILING_COLLISION;
        if(grid[grid_position] != 0)                return (temp_y != 0) ? FLOOR_COLLISION: CEILING_COLLISION;
    }
    return NO_COLLISION;
}

Collision_Type is_valid_position(int grid[GRID_SIZE], int shape[TTM_SIZE][TTM_SIZE], int temp_x, int temp_y)
{
    int positions[TTM_SIZE] = {0};
    int index = 0;
    for (int y = 0; y < TTM_SIZE; y ++) {
        for (int x = 0; x < TTM_SIZE; x ++) {
            if(shape[y][x] == 0) continue;
            int tetro_pos_x = INITIAL_POS_X + x;
            int tetro_pos_y = y * GRID_COLUMN;
            positions[index++] = tetro_pos_x + tetro_pos_y;
        }
    }
    Collision_Type collision = get_collision(grid,positions, temp_x, temp_y);
    return collision;
}

Rotation_Position get_rotate_positions(Actual_Tetrimino *at, int rotated_shape[TTM_SIZE][TTM_SIZE], Action rotation_type)
{
    Rotation_Position new_rot_pos = at->rot_pos;
    //clockwise rotation
    if(rotation_type == ACT_ROTATE_RIGHT) {
        new_rot_pos = (new_rot_pos + 1) % 4;
        int size = (at->ttm.type == TETRI_I) ? 4 : 3;
        for(int y = 0; y < size; y ++){
            for(int x = 0; x < size; x ++){
                rotated_shape[y][size - x - 1] = at->ttm.matrix[x][y];
            }
        }
        return new_rot_pos;
    }
    //counterclockwise rotation
    if(rotation_type == ACT_ROTATE_LEFT) {
        new_rot_pos = (new_rot_pos - 1) % 4;
        int size = (at->ttm.type == TETRI_I) ? 4 : 3;
        for(int y = 0; y < size; y ++){
            for(int x = 0; x < size; x ++){
                rotated_shape[size - x - 1][y] = at->ttm.matrix[y][x];
            }
        }
        return new_rot_pos;
    }

    return at->rot_pos;
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

void wall_kick(Actual_Tetrimino *at, int grid[GRID_SIZE], int rotated_shape[TTM_SIZE][TTM_SIZE], Rotation_Position new_rot_pos){
    int temp_x = at->pos_x;
    int temp_y = at->pos_y;
    for(int i = 0; i < 5; i ++){
        if(is_valid_position(grid,rotated_shape,temp_x,temp_y) == NO_COLLISION){
            at->pos_x = temp_x;
            at->pos_y = temp_y;
            at->rot_pos = new_rot_pos;
            apply_rotation(at,rotated_shape);
            break;
        }
        temp_x = at->pos_x;
        temp_y = at->pos_y;
        if(at->ttm.type == TETRI_I){
            temp_x += of_wl_ttm_i[at->rot_pos][i][0] - of_wl_ttm_i[new_rot_pos][i][0];
            temp_y += of_wl_ttm_i[at->rot_pos][i][1] - of_wl_ttm_i[new_rot_pos][i][1];
        }else{
            temp_x += of_wl[at->rot_pos][i][0] - of_wl[new_rot_pos][i][0];
            temp_y += of_wl[at->rot_pos][i][1] - of_wl[new_rot_pos][i][1];
        }
    }
}

void shuffle(State *state) {
    for (int i = NUM_TTM - 1; i > 0; i--) {
        int index = rand() % (i + 1);
        Tetrimino temp = state->bag[i];
        state->bag[i] = state->bag[index];
        state->bag[index] = temp;
    }
}

void reset_current_tetrimino(State *state)
{
    state->current_tetrimino.pos_y = 0;
    state->current_tetrimino.pos_x = 0;
    state->current_tetrimino.rot_pos = SPAWN;
    state->current_tetrimino.ttm = state->bag[state->bag_index];
    state->bag_index ++;
    if(state->bag_index > NUM_TTM - 1){
        state->bag_index = 0;
        shuffle(state);
    }
    state->next_tetrimino = state->bag[state->bag_index];
    memset(state->current_tetrimino.positions, 0,sizeof(state->current_tetrimino.positions));
}


bool spawn(State *state)
{
    bool can_spawn = true;
    reset_current_tetrimino(state);
    if(state->current_tetrimino.ttm.type == TETRI_I){
        can_spawn = is_valid_position(state->grid,state->current_tetrimino.ttm.matrix,0,-1) == NO_COLLISION;
        get_tetro_pos(state->current_tetrimino.positions, state->current_tetrimino.ttm.matrix,0,-1);
        state->current_tetrimino.pos_y -= 1;
    }else{
        can_spawn = is_valid_position(state->grid,state->current_tetrimino.ttm.matrix,0,0) == NO_COLLISION;
        get_tetro_pos(state->current_tetrimino.positions, state->current_tetrimino.ttm.matrix,0,0);
    }
    return can_spawn;
}

int hard_drop(Actual_Tetrimino * at, int grid[GRID_SIZE])
{
    int temp_y = 0;
    Collision_Type hard_drop_col = NO_COLLISION;
    while(hard_drop_col != FLOOR_COLLISION){
        temp_y += 1;
        hard_drop_col = get_collision(grid, at->positions, 0, temp_y + 1);
    }
    while(get_collision(grid, at->positions, 0, temp_y) != NO_COLLISION){
        temp_y -=1;
    }
    return temp_y;
}

void lock_in(Actual_Tetrimino * at, int grid[GRID_SIZE])
{
    for(int i = 0; i < TTM_SIZE; i++){
        grid[at->positions[i]] = at->ttm.color;
    }
}

void move_tetrimino(State *state, int temp_x, int temp_y, Action action)
{
    Collision_Type collision = get_collision(state->grid, state->current_tetrimino.positions, temp_x, temp_y);

    if(collision == NO_COLLISION ){
        state->current_tetrimino.pos_x += temp_x;
        state->current_tetrimino.pos_y += temp_y;
    }
    if(action == ACT_HARD_DROP) {
        get_tetro_pos(state->current_tetrimino.positions, state->current_tetrimino.ttm.matrix, state->current_tetrimino.pos_x, state->current_tetrimino.pos_y);
        collision = FLOOR_COLLISION;
    }

    if(collision == FLOOR_COLLISION){
        clear();
        lock_in(&state->current_tetrimino, state->grid);
        int can_spawn = spawn(state);
        if(!can_spawn) state->game_state = GAME_OVER;
    }
}

void rotate_tetrimino(State *state, Action rotation_type)
{
    Rotation_Position new_rot_pos = state->current_tetrimino.rot_pos;
    int new_positions[TTM_SIZE][TTM_SIZE]= {0};
    if(state->current_tetrimino.ttm.type == TETRI_O) return;
    new_rot_pos = get_rotate_positions(&state->current_tetrimino, new_positions,rotation_type);
    wall_kick(&state->current_tetrimino,state->grid,new_positions, new_rot_pos);
}


Action get_action(int key)
{
    switch(key)
    {
        case KEY_LEFT:   return ACT_LEFT;
        case KEY_RIGHT:  return ACT_RIGHT;
        case KEY_DOWN:   return ACT_DOWN;
        case KEY_UP:     return ACT_ROTATE_RIGHT;
        case 'd':        return ACT_ROTATE_LEFT;
        case 'r':        return ACT_ROTATE_RIGHT;
        case KEY_RESIZE: return ACT_RESIZE;
        case ' ':        return ACT_HARD_DROP;
    }
    return ACT_NONE;
}

void exec_action(Action action, State *state)
{
    int temp_x = 0;
    int temp_y = 0;
    switch(action){
    case ACT_NONE:
        break;
    case ACT_RIGHT:
        temp_x += 1;
        move_tetrimino(state, temp_x, 0, ACT_RIGHT);
        break;
    case ACT_LEFT:
        temp_x -= 1;
        move_tetrimino(state, temp_x, 0, ACT_LEFT);
        break;
    case ACT_DOWN:
        temp_y += 1;
        move_tetrimino(state, 0, temp_y, ACT_DOWN);
        break;
    case ACT_HARD_DROP:
        temp_y = hard_drop(&state->current_tetrimino, state->grid);
        move_tetrimino(state,0,temp_y,ACT_HARD_DROP);
        break;
    case ACT_ROTATE_RIGHT:
        rotate_tetrimino(state, action);
        break;
    case ACT_ROTATE_LEFT:
        rotate_tetrimino(state, action);
        break;
    case ACT_RESIZE:
        clear();
        break;
    }
}

bool is_fill(int line, int grid[GRID_SIZE])
{
    for(int i = 0; i < GRID_COLUMN; i ++){
        if(grid[line * GRID_COLUMN + i] == 0) return false;
    }
    return true;
}

void reset_grid(int fill_line, int grid[GRID_SIZE])
{
    memmove(grid + GRID_COLUMN, grid, (fill_line * GRID_COLUMN) * sizeof(int));
    memset(grid,0, GRID_COLUMN * sizeof(int));
}

int clean_fill_line(int grid[GRID_SIZE])
{
    int lines_clear = 0;
    for(int lines = 0; lines < GRID_ROW; lines ++){
        if(is_fill(lines, grid)){
            lines_clear += 1;
            reset_grid(lines,grid);
        }
    }
    return lines_clear;
}

void init_state(State *state)
{
    srand(time(NULL));
    memcpy(state->bag, tetriminos, sizeof(state->bag));
    shuffle(state);
    state->next_tetrimino = state->bag[1];
    state->game_state     = MENU;
    state->bag_index      = 0;
    state->score          = 0;
    state->lines_clear    = 0;
    state->level          = 0;
    memset(state->grid, 0,sizeof(state->grid));
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

            init_pair(RED,     17, COLOR_RED);
            init_pair(GREEN,   15, COLOR_GREEN);
            init_pair(YELLOW,  14, COLOR_YELLOW);
            init_pair(BLUE,    11, COLOR_BLUE);
            init_pair(CYAN,    10, COLOR_CYAN);
            init_pair(MAGENTA, 16, COLOR_MAGENTA);
            init_pair(ORANGE,  13, 12);
            init_pair(BORDERS, COLOR_BLACK, COLOR_WHITE);
        }else{
            init_pair(RED,     COLOR_BLACK, COLOR_RED);
            init_pair(GREEN,   COLOR_BLACK, COLOR_GREEN);
            init_pair(YELLOW,  COLOR_BLACK, COLOR_YELLOW);
            init_pair(BLUE,    COLOR_BLACK, COLOR_BLUE);
            init_pair(CYAN,    COLOR_BLACK, COLOR_CYAN);
            init_pair(MAGENTA, COLOR_BLACK, COLOR_MAGENTA);
            init_pair(ORANGE,  COLOR_BLACK, COLOR_MAGENTA);
            init_pair(BORDERS, COLOR_BLACK, COLOR_WHITE);
        }
    }
}

void render_next_tetrimino(Tetrimino next_tetrimino)
{
    int next_ttm_position[TTM_SIZE] = {0};
    get_tetro_pos(next_ttm_position, next_tetrimino.matrix, 0, 0);
    mvprintw((LINES/2) - (GRID_ROW/2), CENTER_X - (GRID_COLUMN * 2) + 6, "Next Piece");
    render(next_ttm_position, next_tetrimino.color, CENTER_X - (GRID_COLUMN * 2), (LINES/2) - (GRID_ROW/2) + 3);
}

void render_game_stats(State *state)
{
    refresh();
    mvprintw((LINES/2) - (GRID_ROW/2) + 0, CENTER_X + (GRID_COLUMN * 2) + 6,  "Level");
    mvprintw((LINES/2) - (GRID_ROW/2) + 1, CENTER_X + (GRID_COLUMN * 2) + 10, "%d", state->level);
    mvprintw((LINES/2) - (GRID_ROW/2) + 2, CENTER_X + (GRID_COLUMN * 2) + 6,  "Score");
    mvprintw((LINES/2) - (GRID_ROW/2) + 3, CENTER_X + (GRID_COLUMN * 2) + 10, "%d", state->score);
    mvprintw((LINES/2) - (GRID_ROW/2) + 4, CENTER_X + (GRID_COLUMN * 2) + 6,  "Lines");
    mvprintw((LINES/2) - (GRID_ROW/2) + 5, CENTER_X + (GRID_COLUMN * 2) + 10, "%d", state->lines_clear);
}

void render_ghost_piece(State *state)
{
    int pos_y = hard_drop(&state->current_tetrimino, state->grid);
    render(state->current_tetrimino.positions, 0, CENTER_X, CENTER_Y + pos_y);
}

void update_render(State *state)
{
    refresh();
    draw_grid(state->grid);
    render_ghost_piece(state);
    render(state->current_tetrimino.positions, state->current_tetrimino.ttm.color, CENTER_X, CENTER_Y);
    render_next_tetrimino(state->next_tetrimino);
    render_game_stats(state);
}

void update_score(State *state, int lines_clear)
{
    int total_lines_clear = state->lines_clear + lines_clear;
    int next_level = (total_lines_clear / 10);
    int current_level = (state->lines_clear / 10);
    if(current_level < next_level && state->level < (NUM_LEVELS - 1)){
        state->level += 1;
    }
    state->lines_clear = total_lines_clear;
    if(lines_clear > 0 && lines_clear < 5) state->score += score_per_line[lines_clear - 1];
}

void start_menu(State *state)
{
    int positions[TTM_SIZE] = {0};
    int shape[TTM_SIZE][TTM_SIZE] = {0};

    //Show all tetriminos
    for(int i = 0; i < NUM_TTM; i ++){
        int tetri_offset = (i * TTM_WIDTH + TTM_SIZE * i);
        memcpy(shape, tetriminos[i].matrix, sizeof(shape));
        get_tetro_pos(positions, shape, 0,0);
        if(i <= 2){
            if(tetriminos[i].type == TETRI_L) get_tetro_pos(positions, shape, -1, 0);
            render(positions, tetriminos[i].color, (tetri_offset + (i * 4)) + COLS - CENTER_X - 28, LINES/2.0 - LINES * 0.3);
        }else{
            render(positions, tetriminos[i].color, (tetri_offset + (i * 2)) + COLS - CENTER_X - 28, LINES/2.0 - LINES * 0.3);
        }
    }
    color_on(BLUE);
    mvprintw(LINES/2.0 - LINES * 0.3 + 4  , COLS - (CENTER_X) - 10*2,  "== TETRIS TIME ==");
    color_off(BLUE);
    mvprintw(LINES/2.0 - LINES * 0.3 + 6  , COLS - (CENTER_X) - 9*2,  "== CONTROLS ==");
    mvprintw(LINES/2.0 - LINES * 0.3 + 7  , COLS - (CENTER_X) - 11*2, "[arrow_right]:  move right");
    mvprintw(LINES/2.0 - LINES * 0.3 + 8  , COLS - (CENTER_X) - 11*2, "[arrow_left]:   move left");
    mvprintw(LINES/2.0 - LINES * 0.3 + 9  , COLS - (CENTER_X) - 11*2, "[arrow_up]|[r]: rotate right");
    mvprintw(LINES/2.0 - LINES * 0.3 + 11 , COLS - (CENTER_X) - 11*2, "[d]:            rotate left");
    mvprintw(LINES/2.0 - LINES * 0.3 + 10 , COLS - (CENTER_X) - 11*2, "[arrow_down]:   soft drop");
    mvprintw(LINES/2.0 - LINES * 0.3 + 12 , COLS - (CENTER_X) - 11*2, "[space]:        hard drop");
    color_on(GREEN);
    mvprintw(LINES/2.0 - LINES * 0.3 + 13  , COLS - (CENTER_X) - 11*2, "[press SPACE to start!]");
    color_off(GREEN);
    if((getch()) == ' ') {
        state->game_state = GAME;
        clear();
    }
}

void game_over(State *state)
{
    color_on(BORDERS);
    mvprintw(LINES/2.0 - LINES * 0.3 + 4  , COLS - (CENTER_X) - 14*2,  "== GAME OVER ==       ");
    mvprintw(LINES/2.0 - LINES * 0.3 + 5  , COLS - (CENTER_X) - 14*2,  "Final Score: %d        ",state->score);
    mvprintw(LINES/2.0 - LINES * 0.3 + 6  , COLS - (CENTER_X) - 14*2,  "Level:       %d        ",state->level);
    mvprintw(LINES/2.0 - LINES * 0.3 + 7  , COLS - (CENTER_X) - 14*2,  "[press SPACE to exit!]");
    color_off(BORDERS);
    if((getch()) == ' ') {
        state->game_state = EXIT;
        clear();
    }
}

void game_loop(State *state)
{
    while(state->game_state == GAME) {
        int lines_clear = clean_fill_line(state->grid);
        get_tetro_pos(state->current_tetrimino.positions, state->current_tetrimino.ttm.matrix, state->current_tetrimino.pos_x,state->current_tetrimino.pos_y);
        update_score(state, lines_clear);
        update_render(state);
        state->key = getch();
        Action action = get_action(state->key);
        exec_action(action,state);
        if(state->current_frame >= frames_per_level[state->level]){
            exec_action(ACT_DOWN,state);
            state->current_frame = 0;
        }
        state->current_frame ++;
        usleep(16667);
    }
}

int main(void)
{
    //Terminal setup
    initscr();
    terminal_has_colors();
    curs_set(0);
    noecho();
    cbreak();
    setlocale(LC_ALL, "");
    nodelay(stdscr, TRUE);
    keypad(stdscr,  TRUE);

    //Starting the Game
    State state;
    init_state(&state);
    reset_current_tetrimino(&state);
    while(state.game_state != EXIT) {
        if(state.game_state == MENU)      start_menu(&state);
        if(state.game_state == GAME)      game_loop(&state);
        if(state.game_state == GAME_OVER) game_over(&state);
    }
    endwin();
    return 0;
}
