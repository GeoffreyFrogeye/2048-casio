#include "stdlib.h"
#include "fxlib.h"
#include "MonochromeLib.h"



// Sprites
	char tile[14][26] = {{127,240,213, 88,170,168,213, 88,170,168,213, 88,170,168,213, 88,170,168,213, 88,170,168,213, 88,127,240},
						 {127,240,128,  8,128,  8,135,  8,136,136,128,136,129,  8,130,  8,132,  8,143,136,128,  8,128,  8,127,240},
						 {127,240,128,  8,128,  8,129,  8,131,  8,133,  8,137,  8,143,136,129,  8,129,  8,128,  8,128,  8,127,240},
						 {127,240,128,  8,128,  8,135,  8,136,136,136,136,135,  8,136,136,136,136,135,  8,128,  8,128,  8,127,240},
						 {127,240,128,  8,128,  8,144,200,177,  8,146,  8,147,200,146, 40,146, 40,185,200,128,  8,128,  8,127,240},
						 {127,240,128,  8,128,  8,128,  8,184,200,133, 40,152, 72,132,136,185,232,128,  8,128,  8,128,  8,127,240},
						 {127,240,128,  8,128,  8,128,  8,152,104,160,168,185, 40,165,232,152, 40,128,  8,128,  8,128,  8,127,240},
						 {127,240,253,248,249,248,253,248,253,248,248,248,255,248,231, 56,218,216,247, 56,238,216,195, 56,127,240},
						 {127,240,249,248,246,248,253,248,251,248,240,248,255,248,195, 56,222,248,194, 56,250,216,199, 56,127,240},
						 {127,240,240,248,247,248,240,248,254,248,241,248,255,248,247, 56,230,216,247,184,247,120,226, 24,127,240},
						 {127,240,247, 56,230,216,246,216,246,216,227, 56,255,248,231,152,219, 88,246,216,238, 24,195,216,127,240},
						 {127,240,231, 56,218,216,246,216,238,216,195, 56,255,248,243, 56,234,216,219, 56,194,216,251, 56,127,240},
						 {127,240,255,248,255,248,248,248,247,120,255,120,254,248,253,248,255,248,253,248,255,248,255,248,127,240},
						 {127,240,255,248,255,248,253,248,253,248,253,248,253,248,253,248,255,248,253,248,255,248,255,248,127,240}};
	char scoreBG[] = {63,255,255,255,254,0,127,255,255,255,255,0,255,255,255,255,255,128,255,255,255,255,255,128,255,255,255,255,255,128,255,255,255,255,255,128,255,255,255,255,255,128,255,255,255,255,255,128,255,255,255,255,255,128,255,255,255,255,255,128,255,255,255,255,255,128,255,255,255,255,255,128,255,255,255,255,255,128,255,255,255,255,255,128,255,255,255,255,255,128,255,255,255,255,255,128,255,255,255,255,255,128,127,255,255,255,255,0,63,255,255,255,254,0}; // 41*19

#define false 0
#define true 1
typedef char bool;

typedef struct Cell {
    int x;
    int y;
} Cell;

typedef struct Tile {
	int x;
	int y;
    int value;
    bool hasMerged;
    bool hasMoved;
    bool isNew;
    Cell previousPosition;
    Cell previousMerge;
} Tile;

typedef struct Grid {
	Tile array[4][4];
} Grid;

typedef struct Traversal {
	int x[4];
	int y[4];
} Traversal;

typedef struct findFarthestPosition_return {
	Cell next;
	Cell farthest;
} findFarthestPosition_return;

#define SCREEN_ANIMATION_TIME 32

int Game_score = 0;
bool Game_over = false;
bool Game_won = false;
bool Game_keepPlaying = true;

int Storage_bestScore = 0;

Grid Grid_grid;

static int SysCallCode[] = {0xD201422B,0x60F20000,0x80010070};
static int (*SysCall)(int R4, int R5, int R6, int R7, int FNo ) = (void*)&SysCallCode;

// Usual functions
int RTC_getTicks() {
     return (*SysCall)(0, 0, 0, 0, 0x3B);
}

unsigned char* intToStr(unsigned char* c, int n) { // Code par Eiyeron, Licence Creative Commons BY-SA
    if(n==0) {
        c[0] = '0';
        c[1] = 0;
    } else {
        int i, l=0;
        if(n<0) {
            c[0] = '-';
            n = abs(n);
            l++;
        }
        for(i=n ; i ; i/=10)
            l++;
        c[l] = 0;
        for(i=n ; i ; i/=10)
            c[--l] = i%10+'0';
    }
	return c;
}

int rand_int(int min, int max) {
    return min + (rand() % (int)(max - min + 1));
}

// Grid

Tile Grid_getEmptyTile() {
	Tile emptyTile; Cell emptyCell;
	emptyCell.x = -1;
	emptyCell.y = -1;
	emptyTile.x = -1;
	emptyTile.y = -1;
    emptyTile.value = 0;
    emptyTile.hasMerged = false;
    emptyTile.hasMoved = false;
    emptyTile.isNew = false;
    emptyTile.previousPosition = emptyCell;
    emptyTile.previousMerge = emptyCell;
    return emptyTile;

}

bool Grid_withinBounds(Cell position) {
	return (position.x >= 0 && position.x < 4 && position.y >= 0 && position.y < 4);
}

Tile Grid_cellContent(Cell cell) { // TODO Make cellContentEdit(Cell cell, Tile tile), because it's read-only this way
	Tile back;
	if (Grid_withinBounds(cell)) {
		return Grid_grid.array[cell.x][cell.y];
	} else {
		back.value = -1;
		return back;
	}
}

bool Grid_cellOccupied(Cell cell) {
	return (Grid_cellContent(cell).value > 0);
}

bool Grid_cellAvailable(Cell cell) {
  return !Grid_cellOccupied(cell);
}

int Grid_insertTile(Tile tile) {
	Grid_grid.array[tile.x][tile.y] = tile;
}

int Grid_removeTile(Tile tile) { // TODO use with cellContentEdit
	Tile emptyTile;
	emptyTile = Grid_getEmptyTile();
	emptyTile.x = tile.x;
	emptyTile.y = tile.y;
	emptyTile.value = 0;
	Grid_grid.array[tile.x][tile.y] = emptyTile;
}

int Grid_avaiableCellsAmount() {
	int avaiableCellsNumber = 0, x, y;
	Cell position;
	for (x = 0; x <= 3; x++) {
    	for (y = 0; y <= 3; y++) {
    		position.x = x;
    		position.y = y;
    		if (Grid_cellAvailable(position)) {
    			avaiableCellsNumber++;
    		}
    	}
    }
    return avaiableCellsNumber;
}

void storage_setBestScore(int bestScore) {
	Storage_bestScore = bestScore;
	// Sauvegarder dans la mémoire
}

// Screen (O HTML_Actuator)

void Screen_drawTile(int x, int y, int value) {
	ML_rectangle(x + 1, y + 1, x + 11, y + 11, 0, ML_TRANSPARENT, ML_WHITE);
	ML_bmp_or(tile[value], x, y, 13, 13);
	//ML_display_vram(); // DEBUG
}

void Screen_drawTileCase(Tile tile) {
	Screen_drawTile(5+tile.x*14, 5+tile.y*14, tile.value);
}

void Screen_drawTileMoving(Tile tile, float percentage) { // TODO Make smaller
	// Draw moved-tiles and under-merged-tile
	int x, y, OTx, OTy, NTx, NTy;
	NTx = 5+tile.x*14;
	NTy = 5+tile.y*14;
	OTx = 5+tile.previousPosition.x*14;
	OTy = 5+tile.previousPosition.y*14;
	x = OTx + (NTx - OTx) * percentage;
	y = OTy + (NTy - OTy) * percentage;
	// Draw over-merged-tile
	if (!tile.hasMerged) {
		Screen_drawTile(x, y, tile.value);
	} else {
		Screen_drawTile(x, y, tile.value-1);
		OTx = 5+tile.previousMerge.x*14;
		OTy = 5+tile.previousMerge.y*14;
		x = OTx + (NTx - OTx) * percentage;
		y = OTy + (NTy - OTy) * percentage;
		Screen_drawTile(x, y, tile.value-1);
	}
}

int Screen_drawFixedTiles(bool dontDrawPreviousPositionTiles) {
    int x, y;
    Tile tile;
    Cell position;
    for (x = 0; x <= 3; x++) {
    	for (y = 0; y <= 3; y++) {
    		position.x = x;
    		position.y = y;
    		tile = Grid_cellContent(position);
    		if (dontDrawPreviousPositionTiles && (tile.hasMoved || tile.hasMerged || tile.isNew)) {
    			tile.value = 0;
    		}
    		Screen_drawTileCase(tile);
    	}
    }
}
int Screen_drawMovingTiles(float percentage) {
    int x, y;
    Tile tile;
    Cell position;
	for (x = 0; x <= 3; x++) {
    	for (y = 0; y <= 3; y++) {
    		position.x = x;
    		position.y = y;
    		tile = Grid_cellContent(position);
    		if (tile.hasMoved || tile.hasMerged) {
    			Screen_drawTileMoving(tile, percentage);
    		}
    	}
    }
}

void Screen_updateScore() {
	unsigned char chaineScore[6];
    PrintXY(70, 14, intToStr(chaineScore, Game_score), 1);
}

void Screen_updateBestScore() {
	unsigned char chaineScore[6];
    PrintXY(70, 35, intToStr(chaineScore, Storage_bestScore), 1);
}

void Screen_message(bool won) {
	if (won) { // PHD
		PrintXY(67, 54, "WON", 0);
	} else {
		PrintXY(67, 54, "LOSE", 0);
	}
}

bool Game_isGameTerminated() { // Intentionally moved here
	return (Game_over || (Game_won && !Game_keepPlaying));
}

void Screen_actuate() {

	int animationStartTime;
	float animationLength;

	Screen_drawFixedTiles(true);
	SaveDisp(SAVEDISP_PAGE1);

	animationStartTime = RTC_getTicks();
	do {
		animationLength = RTC_getTicks() - animationStartTime;
		RestoreDisp(SAVEDISP_PAGE1);
		Screen_drawMovingTiles(animationLength/SCREEN_ANIMATION_TIME);
		ML_display_vram();
	} while (animationLength <= SCREEN_ANIMATION_TIME);

	RestoreDisp(SAVEDISP_PAGE1);
	Screen_drawFixedTiles(false);

	Screen_updateScore();
	Screen_updateBestScore();

	if (Game_isGameTerminated()) {
		if (Game_over) {
			Screen_message(false);
		} else if (Game_won) {
			Screen_message(true);
		}
	}

	ML_display_vram();
}

// Game (O Game_manager)


void Game_actuate() {
	if (Storage_bestScore < Game_score) {
		storage_setBestScore(Game_score);
	}
	Screen_actuate();
}

Traversal Game_buildTraversals(Cell vector) {
	Traversal traversal;
	int i;
	for (i = 0; i <= 3; i++) {
		traversal.x[i] = (vector.x == 1 ? 3-i : i );
		traversal.y[i] = (vector.y == 1 ? 3-i : i );
	}
	return traversal;
}

Cell Game_getVector(int direction) {
	Cell vector;
	switch(direction) {
		case 0:
			vector.x = 0; vector.y = -1;
			break;
		case 1:
			vector.x = 1; vector.y = 0;
			break;
		case 2:
			vector.x = 0; vector.y = 1;
			break;
		case 3:
			vector.x = -1; vector.y = 0;
			break;
	}
	return vector;
}

void Game_prepareTiles() {
	int x, y;
	Cell previousPosition;
	for (x = 0; x <= 3; x++) {
    	for (y = 0; y <= 3; y++) {
    		previousPosition.x = x;
    		previousPosition.y = y;
    		Grid_grid.array[x][y].hasMerged		   =  false;
    		Grid_grid.array[x][y].hasMerged		   =  false;
    		Grid_grid.array[x][y].hasMoved 		   =  false;
    		Grid_grid.array[x][y].isNew    		   =  false;
    		Grid_grid.array[x][y].previousPosition =  previousPosition;

    	}
    }
}

findFarthestPosition_return Game_findFarthestPosition(Cell cell, Cell vector) {
	Cell previous;
	findFarthestPosition_return back;

	// Progress towards the vector direction until an obstacle is found
	do {
		previous = cell;
		cell.x = previous.x + vector.x;
		cell.y = previous.y + vector.y;
	} while (Grid_withinBounds(cell) && Grid_cellAvailable(cell));

	back.farthest = previous;
	back.next = cell; // Used to check if a merge is required

	return back;
}

bool Game_moveTile(Tile tile, Cell cell) {
	Cell previousPosition;
	if (tile.x == cell.x && tile.y == cell.y) {
		return false;
	} else {
		Grid_removeTile(tile);
		previousPosition.x = tile.x;
		previousPosition.y = tile.y;
		// tile.previousPosition = previousPosition;
		tile.hasMoved = true;
		tile.x = cell.x;
		tile.y = cell.y;
		Grid_insertTile(tile);
		return true;
	}
}

bool Game_positionsEqual(Cell first, Tile second) {
	return (first.x == second.x && first.y == second.y);
}

Cell Grid_randomAvaiableCell() {
	int avaiableCellsNumber, choosenCellNumber, x, y;
	Cell position;

	avaiableCellsNumber = Grid_avaiableCellsAmount();
	choosenCellNumber = rand_int(1, avaiableCellsNumber);
	avaiableCellsNumber = 0; // Used as counter here
	for (x = 0; x <= 3; x++) {
    	for (y = 0; y <= 3; y++) {
    		position.x = x;
    		position.y = y;
    		if (Grid_cellAvailable(position)) {
    			avaiableCellsNumber++;
    			if (avaiableCellsNumber == choosenCellNumber) {
    				return position;
    			}
    		}
    	}
    }
}

// Game (O game_manager)

void Game_addRandomTile() {
	Tile tile; Cell position;

	if (Grid_avaiableCellsAmount() > 0) {
		position = Grid_randomAvaiableCell();
		tile = Grid_getEmptyTile();
		tile.value = (rand_int(0, 10) < 9 ? 1 : 2);
		tile.x = position.x;
		tile.y = position.y;
		tile.isNew = true;
		Grid_insertTile(tile);
	}
}

void Game_move(int direction) { // 0: up, 1: right, 2: down, 3: left
	Cell vector, cell, farthest;
	Tile tile, merged, next;
	Traversal traversals;
	findFarthestPosition_return position;
	bool moved = false;
	int xI, yI;

	if (Game_isGameTerminated()) { return; }

	vector = Game_getVector(direction);
	traversals = Game_buildTraversals(vector);
	Game_prepareTiles();
	for (xI = 0; xI <= 3; xI++) {
		for (yI = 0; yI <= 3; yI++) {
			cell.x = traversals.x[xI]; cell.y = traversals.y[yI];
			tile = Grid_cellContent(cell);
			if (tile.value > 0) {
				position = Game_findFarthestPosition(cell, vector);
				next = Grid_cellContent(position.next);
				farthest = position.farthest;
				if (next.value == tile.value && !next.hasMerged) { // Merge
					merged = Grid_getEmptyTile();
					merged.x = next.x;
					merged.y = next.y;
					merged.value = tile.value + 1;
					merged.hasMerged = true;
					merged.previousPosition = next.previousPosition;
					merged.previousMerge = cell;

					Grid_insertTile(merged);
					Grid_removeTile(tile);

					Game_score += merged.value;

					if (merged.value == 11) {
						Game_won = true;
					}
					moved = true;
				} else {
					if (Game_moveTile(tile, farthest)) {
						moved = true;
					}
				}
			}
		}
	}

	if (moved) {
		Game_addRandomTile();

		// if (!Game_moveAvaiable()) {
		// 	this.over = true;
		// }

		Game_actuate();
	}
	
}

int initGame() {
	// Variables
	int x, y;

    // Reset variables
	Game_score = 0;
	Game_over = false;
	Game_won = false;
	Game_keepPlaying = true;

	for (x = 0; x <= 3; x++) {
    	for (y = 0; y <= 3; y++) {
    		Grid_grid.array[x][y] = Grid_getEmptyTile();
    		Grid_grid.array[x][y].x = x;
    		Grid_grid.array[x][y].y = y;
    		Grid_grid.array[x][y].value = 0;
    	}
    }

	// Draw Title
    PrintXY(67, 54, "2048", 0);
    
	Screen_drawFixedTiles(true);

	// Draw Score
    ML_bmp_or(scoreBG, 68, 4, 41, 19);
    ML_bmp_or(scoreBG, 68, 25, 41, 19);
    PrintXY(70, 6, "SCORE", 1);
    PrintXY(70, 27, "BEST", 1);

    Game_addRandomTile();
    Game_addRandomTile();

    Game_actuate();

}

int AddIn_main(int isAppli, unsigned short OptionNum) {
	// Variables
    unsigned int key;

    srand(RTC_getTicks());

    initGame();
    while (1) { // Main loop
       	GetKey(&key);
       	switch (key) {
    		case KEY_CTRL_UP:
    		case KEY_CHAR_8:
    			Game_move(0);
    			break;
    		case KEY_CTRL_RIGHT:
    		case KEY_CHAR_6:
    			Game_move(1);
    			break;
    		case KEY_CTRL_DOWN:
    		case KEY_CHAR_2:
    			Game_move(2);
    			break;
    		case KEY_CTRL_LEFT:
    		case KEY_CHAR_4:
    			Game_move(3);
    			break;
    		case KEY_CTRL_DEL:
    			initGame();
    			break;
    		default:
    			break;
    	}
    }

    return 1; // 1 is OK here
}

// Exclusive code to any Add-in

#pragma section _BR_Size
unsigned long BR_Size;
#pragma section


#pragma section _TOP

int InitializeSystem(int isAppli, unsigned short OptionNum) {
    return INIT_ADDIN_APPLICATION(isAppli, OptionNum);
}

#pragma section
