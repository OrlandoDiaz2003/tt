# tt 

Personal hobby project made for learning purposes. Classic Tetris implementation in C using ncurses.

## Build
```bash
gcc tt.c -lncursesw -o tt
```

## Run
```bash
./tt
```

## What it has

- SRS rotation system with wall kicks
- 7-bag randomizer (no piece droughts)
- Ghost piece showing drop position
- 30 speed levels
- Standard Tetris scoring

## Controls

- Arrow keys: Move left/right/down
- Arrow up / R: Rotate right
- D: Rotate left
- Space: Hard drop

## What it dont have 

- No pause button yet
- Hold piece
- Lock Delay
