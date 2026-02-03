#include <ncursesw/curses.h>

#define GRID_COLUMN 10
#define GRID_ROW 20

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
    FLOOR_COLLISION,
    WALL_COLLISION,
    CEILING_COLLISION,
    NO_COLLISION
} Collision_Type;

typedef struct {
    int positions[TETRIMINO_SIZE];
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
    {0,0,1,0},
    {0,0,1,0},
    {0,0,1,0},
    {0,0,1,0}
};
int tetri_s [TETRIMINO_SIZE][TETRIMINO_SIZE] = {
    {0,0,1,1},
    {0,1,1,0},
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
    {0,0,0,1},
    {0,0,0,1},
    {0,0,1,1},
    {0,0,0,0}
};
int tetri_l [TETRIMINO_SIZE][TETRIMINO_SIZE] = {
    {1,0,0,0},
    {1,0,0,0},
    {1,1,0,0},
    {0,0,0,0}
};

int tetri_t [TETRIMINO_SIZE][TETRIMINO_SIZE] = {
    {0,1,1,1},
    {0,0,1,0},
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

        if (grid[i] == 1) {
            mvprintw(CENTER_Y + line, CENTER_X + OFFSET_IN(limit), "[]");
        } else {
            mvprintw(CENTER_Y + line,CENTER_X + OFFSET_IN(limit), ". ");
        }
        limit ++;
    }
    mvprintw(CENTER_Y + line, CENTER_X + OFFSET_BORDER(limit), "!>");
}

void get_tetro_pos(int tetrimino[TETRIMINO_SIZE][TETRIMINO_SIZE], Actual_Tetrimino *at)
{
    int index = 0;
    for (int y = 0; y < TETRIMINO_SIZE; y ++) {
        for (int x = 0; x < TETRIMINO_SIZE; x ++) {
            if(tetrimino[y][x] == 0) {
                continue;
            }
            int tetro_pos_x =(INITIAL_POS_X + x + at->pos_x);
            int tetro_pos_y =  (y + at->pos_y) * GRID_COLUMN;
            mvprintw(0, 0, "ACTUAL_POS X: %d | Y: %d", tetro_pos_x, tetro_pos_y);
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

Collision_Type get_collision(Actual_Tetrimino *at )
{
    //TODO: IMPLEMENT COLLISION
    for(int i = 0; i< TETRIMINO_SIZE; i ++){
        int pos_x = at->positions[i] % GRID_COLUMN;
        int pos_y = at->positions[i] / GRID_COLUMN;
        mvprintw(1,0,"COLLISION_POS X: %d | Y: %d",pos_x, pos_y);
        if(pos_y + 1 >= GRID_ROW - 1){
            mvprintw(3,0, "FLOOR COLLISON!");

        }
        if(pos_x + 1 > 10 || pos_x +1 < 0){
            mvprintw(4,0, "WALL COLLISON!");

        }

    }
    return NO_COLLISION;
}
void move_tetris(Actual_Tetrimino *at)
{
    int key = getch();
    switch(key)
    {
        case KEY_LEFT:
            at->pos_x -= 1;
            break;
        case KEY_RIGHT:
            at->pos_x += 1;
            break;
        case KEY_UP:
            at->pos_y -= 1;
            break;
        case KEY_DOWN:
            at->pos_y += 1;
            break;
    }
}

Collision_Type handle_tetrimino(Actual_Tetrimino *at)
{
    int (*tetrimino)[TETRIMINO_SIZE][TETRIMINO_SIZE];

    switch(at->type)
    {
        case TETRI_O:
            tetrimino = &tetri_o;
            break;
        case TETRI_I:
            tetrimino = &tetri_i;
            break;
        case TETRI_S:
            tetrimino = &tetri_s;
            break;
        case TETRI_Z:
            tetrimino = &tetri_z;
            break;
        case TETRI_J:
            tetrimino = &tetri_j;
            break;
        case TETRI_L:
            tetrimino = &tetri_l;
            break;
        case TETRI_T:
            tetrimino = &tetri_t;
            break;
        }

    get_tetro_pos(*tetrimino,at);
    render_tetrimino(at);
    Collision_Type collision = get_collision(at);

    if(collision == NO_COLLISION){
        move_tetris(at);
        return collision;
    }
    return collision;


 }

 int main(void)
 {
     initscr();
     curs_set(0);
     noecho();
     cbreak();
     keypad(stdscr, TRUE);
     Actual_Tetrimino at= {0};
     at.type = TETRI_I;
     for(;;) {
         clear();
         draw_grid();
         handle_tetrimino(&at);
    }
    endwin();
    return 0;
}
