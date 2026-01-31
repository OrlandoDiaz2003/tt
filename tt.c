#include <curses.h>

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
} Tetriminos;

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

void get_tetro_pos(int tetrimino[TETRIMINO_SIZE][TETRIMINO_SIZE], int positions[TETRIMINO_SIZE])
{
    int index = 0;
    for (int y = 0; y < TETRIMINO_SIZE; y ++) {
        for (int x = 0; x < TETRIMINO_SIZE; x ++) {
            if(tetrimino[y][x] == 0) {
                continue;
            }
            int position = INITIAL_POS_X + x + (y * GRID_COLUMN);
            positions[index++] = position;
            // int current_row = (position / GRID_COLUMN);
            // int current_column = position % GRID_COLUMN;
            // mvprintw((CENTER_Y + current_row) ,CENTER_X + OFFSET_IN(current_column), "[]");
        }
    }
}

void render_tetrimino(int positions[TETRIMINO_SIZE])
{
    int pos_x = 0;
    int pos_y = 0;
    for(int i = 0; i < TETRIMINO_SIZE; i++){
        pos_x = positions[i] % GRID_COLUMN;
        pos_y = positions[i] / GRID_COLUMN;
        mvprintw((CENTER_Y + pos_y), CENTER_X + OFFSET_IN(pos_x),"[]");
    }
}

void handle_tetrimino(Tetriminos type)
{
    int positions[TETRIMINO_SIZE] = {0};
    int (*tetrimino)[TETRIMINO_SIZE][TETRIMINO_SIZE];

    switch(type)
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
    get_tetro_pos(*tetrimino, positions);
    render_tetrimino(positions);
 }

 int main(void)
 {
     initscr();
     curs_set(0);
     noecho();
     cbreak();
     Tetriminos test = TETRI_Z;
     for(int i = 0; i < GRID_ROW; i ++) {
        draw_grid();
        handle_tetrimino(test);
        getch();
    }
    endwin();
    return 0;
}
