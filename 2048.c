#include "stdlib.h"
#include "fxlib.h"
#include "MonochromeLib.h"


// Sprites
	// Tiles 13*13
	char tile0[]  = {127,240,213,88,170,168,213,88,170,168,213,88,170,168,213,88,170,168,213,88,170,168,213,88,127,240}; // Vide
	char tile1[]  = {127,240,128,8,128,8,135,8,136,136,128,136,129,8,130,8,132,8,143,136,128,8,128,8,127,240}; // 2
	char tile2[]  = {127,240,128,8,128,8,129,8,131,8,133,8,137,8,143,136,129,8,129,8,128,8,128,8,127,240}; // 4
	char tile3[]  = {127,240,128,8,128,8,135,8,136,136,136,136,135,8,136,136,136,136,135,8,128,8,128,8,127,240}; // 8
	char tile4[]  = {127,240,128,8,128,8,144,200,177,8,146,8,147,200,146,40,146,40,185,200,128,8,128,8,127,240}; // 16
	char tile5[]  = {127,240,128,8,128,8,128,8,184,200,133,40,152,72,132,136,185,232,128,8,128,8,128,8,127,240}; // 32
	char tile6[]  = {127,240,128,8,128,8,128,8,152,104,160,168,185,40,165,232,152,40,128,8,128,8,128,8,127,240}; // 64
	char tile7[]  = {127,240,253,248,249,248,253,248,253,248,248,248,255,248,231,56,218,216,247,56,238,216,195,56,127,240}; // 128
	char tile8[]  = {127,240,249,248,246,248,253,248,251,248,240,248,255,248,195,56,222,248,194,56,250,216,199,56,127,240}; // 256
	char tile9[]  = {127,240,240,248,247,248,240,248,254,248,241,248,255,248,247,56,230,216,247,184,247,120,226,24,127,240}; // 512
	char tile10[]  = {127,240,247,56,230,216,246,216,246,216,227,56,255,248,231,152,219,88,246,216,238,24,195,216,127,240}; // 1024
	char tile11[] = {127,240,231,56,218,216,246,216,238,216,195,56,255,248,243,56,234,216,219,56,194,216,251,56,127,240}; // 2048
	char tile12[] = {127,240,255,248,255,248,248,248,247,120,255,120,254,248,253,248,255,248,253,248,255,248,255,248,127,240}; // 4096
	char tile13[] = {127,240,255,248,255,248,253,248,253,248,253,248,253,248,253,248,255,248,253,248,255,248,255,248,127,240}; // WTF
	// Autre
	char scoreBG[] = {63,255,255,255,254,0,127,255,255,255,255,0,255,255,255,255,255,128,255,255,255,255,255,128,255,255,255,255,255,128,255,255,255,255,255,128,255,255,255,255,255,128,255,255,255,255,255,128,255,255,255,255,255,128,255,255,255,255,255,128,255,255,255,255,255,128,255,255,255,255,255,128,255,255,255,255,255,128,255,255,255,255,255,128,255,255,255,255,255,128,255,255,255,255,255,128,255,255,255,255,255,128,127,255,255,255,255,0,63,255,255,255,254,0}; // 41*19

// Déclare les booléens ><
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
    Cell previousPosition;
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

int Game_score = 0;
bool Game_over = false;
bool Game_won = false;
bool Game_terminated = false;
bool Game_keepPlaying = true;

int storage_bestScore = 6357;

Grid Grid_grid;

// Fonctions
int rand_int(int min, int max) {
    return min + (rand() % (int)(max - min + 1));
}

int drawFixedTiles() {
	// Variables
    int x, y;

    // Éxecution
    for (x = 0; x <= 3; x++) {
    	for (y = 0; y <= 3; y++) {
    		drawTileCase(Grid_grid.array[x][y]);
    	}
    }
}
int drawTile(int x, int y, int value) {
	ML_rectangle(x + 1, y + 1, x + 11, y + 11, 0, ML_TRANSPARENT, ML_WHITE);
	switch (value) {
		case 0:
			ML_bmp_or(tile0, x, y, 13, 13);
			break;
		case 1:
			ML_bmp_or(tile1, x, y, 13, 13);
			break;
		case 2:
			ML_bmp_or(tile2, x, y, 13, 13);
			break;
		case 3:
			ML_bmp_or(tile3, x, y, 13, 13);
			break;
		case 4:
			ML_bmp_or(tile4, x, y, 13, 13);
			break;
		case 5:
			ML_bmp_or(tile5, x, y, 13, 13);
			break;
		case 6:
			ML_bmp_or(tile6, x, y, 13, 13);
			break;
		case 7:
			ML_bmp_or(tile7, x, y, 13, 13);
			break;
		case 8:
			ML_bmp_or(tile8, x, y, 13, 13);
			break;
		case 9:
			ML_bmp_or(tile9, x, y, 13, 13);
			break;
		case 10:
			ML_bmp_or(tile10, x, y, 13, 13);
			break;
		case 11:
			ML_bmp_or(tile11, x, y, 13, 13);
			break;
		default:
			ML_bmp_or(tile12, x, y, 13, 13);
			break;
	}
}

int drawTileCase(Tile tile) {
	drawTile(5+tile.x*14, 5+tile.y*14, tile.value);
}

// Grid

bool Grid_withinBounds(Cell position) { // En abuser
	return (position.x >= 0 && position.x < 4 && position.y >= 0 && position.y < 4);
}

Tile Grid_cellContent(Cell cell) {
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

bool Grid_cellAvailable(Cell cell) { // Pareil
  return !Grid_cellOccupied(cell);
}

int Grid_insertTile(Tile tile) {
	Grid_grid.array[tile.x][tile.y] = tile;
}

int Grid_removeTile(Tile tile) {
	Cell emptyCell;
	Tile emptyTile;
	
	emptyCell.x = -1;
	emptyCell.y = -1;
	emptyTile.x = tile.x;
	emptyTile.y = tile.y;
	emptyTile.value = 0;
	emptyTile.hasMerged = false;
	emptyTile.previousPosition = emptyCell;

	Grid_grid.array[tile.x][tile.y] = emptyTile;
}

int Grid_avaiableCellsAmount() {
	int avaiableCellsNumber = 0, x, y;
	Cell testCell;
	for (x = 0; x <= 3; x++) {
    	for (y = 0; y <= 3; y++) {
    		testCell.x = x;
    		testCell.y = y;
    		if (Grid_cellAvailable(testCell)) {
    			avaiableCellsNumber++;
    		}
    	}
    }
    return avaiableCellsNumber;
}

void storage_setBestScore(int bestScore) {
	storage_bestScore = bestScore;
	// Sauvegarder dans la mémoire
}

// Screen (O HTML_Actuator)

void Screen_updateScore() {
	// Affiche le score à l'écran
}

void Screen_updateBestScore() {
	// Affiche le meilleur score à l'écran
}

void Screen_message(bool won) {
	if (won) { // PHD
		PrintXY(67, 54, "WON", 0);
	} else {
		PrintXY(67, 54, "LOSE", 0);
	}
}

void Screen_actuate() {

	drawFixedTiles(); // O self.addTile(cell);

	Screen_updateScore(); // O self.updateScore(metadata.score);
	Screen_updateBestScore(); // O self.updateBestScore(metadata.bestScore);

	if (Game_terminated) {
		if (Game_over) {
			Screen_message(false);
		} else if (Game_won) {
			Screen_message(true);
		}
	}

	ML_display_vram();
}

Cell convertNumber2Pos(int number) {
	Cell position;
	position.x = number/4;
	position.x = number%4;
}

// Game (O Game_manager)

bool Game_isGameTerminated() {
	return (Game_over || (Game_won && !Game_keepPlaying));
}

void Game_actuate() {
	if (storage_bestScore < Game_score) {
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
	Cell previousPosition;
	int x, y;
	for (x = 0; x <= 3; x++) {
    	for (y = 0; y <= 3; y++) {
    		Grid_grid.array[x][y].hasMerged =  false;
    		previousPosition.x = Grid_grid.array[x][y].x;
    		previousPosition.y = Grid_grid.array[x][y].y;
    		Grid_grid.array[x][y].previousPosition = previousPosition;
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

Game_moveTile(Tile tile, Cell cell) {
	Grid_removeTile(tile);
	tile.x = cell.x;
	tile.y = cell.y;
	Grid_insertTile(tile);
}

bool Game_positionsEqual(Cell first, Tile second) {
	return (first.x == second.x && first.y == second.y);
}

Cell Grid_randomAvaiableCell() {
	int avaiableCellsNumber, choosenCellNumber, x, y;
	Cell position;

	avaiableCellsNumber = Grid_avaiableCellsAmount();
	choosenCellNumber = rand_int(1, avaiableCellsNumber);
	avaiableCellsNumber = 0; // Sert de compteur ici
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

void Game_addRandomTile() {
	Tile tile; Cell position;

	if (Grid_avaiableCellsAmount() > 0) {
		position = Grid_randomAvaiableCell();
		tile.value = (rand_int(0, 10) < 9 ? 1 : 2);
		tile.x = position.x;
		tile.y = position.y;
		tile.previousPosition = position;
		tile.hasMerged = false;
		Grid_insertTile(tile);
	}
}

void Game_move(int direction) { // 0: up, 1: right, 2: down, 3: left
	Cell vector, cell, farthest, next;
	Tile tile, merged;
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
				next = position.next;
				farthest = position.farthest;
				if (Grid_cellContent(next).value == tile.value && !tile.hasMerged) { // Merge
					merged.x = next.x;
					merged.y = next.y;
					merged.value = tile.value + 1;
					merged.hasMerged = true;
					merged.previousPosition = cell;

					Grid_insertTile(merged);
					Grid_removeTile(tile);

					tile.x = next.x;
					tile.y = next.y;

					Game_score += merged.value;

					if (merged.value == 11) {
						Game_won = true;
					}
					moved = true;
				} else {
					Game_moveTile(tile, farthest);
					moved = true;
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

	ML_clear_screen();

	// Draw Titre
    PrintXY(67, 54, "2048", 0);

    // Reset variables
	Game_score = 0;
	Game_over = false;
	Game_won = false;
	Game_terminated = false;
	Game_keepPlaying = true;

	for (x = 0; x <= 3; x++) {
    	for (y = 0; y <= 3; y++) {
    		Grid_grid.array[x][y].x = x;
    		Grid_grid.array[x][y].y = y;
    		Grid_grid.array[x][y].value = 0;
    	}
    }

	drawFixedTiles();

	// Draw Score
    ML_bmp_or(scoreBG, 68, 4, 41, 19);
    ML_bmp_or(scoreBG, 68, 25, 41, 19);
    PrintXY(70, 6, "SCORE", 1);
    PrintXY(100, 14, "0", 1);
    PrintXY(70, 27, "BEST", 1);
    PrintXY(100, 35, "0", 1);

    Game_addRandomTile();
    Game_addRandomTile();

    Game_actuate();

}

int AddIn_main(int isAppli, unsigned short OptionNum) {
	// Variables
    unsigned int key;

    while (1) { // Tant qu'on joue...
    	initGame();
    	while (1) { // Boucle des mouvements
        	GetKey(&key);
        	switch (key) {
    			case KEY_CTRL_UP:
    				Game_move(0);
    				break;
    			case KEY_CTRL_RIGHT:
    				Game_move(1);
    				break;
    			case KEY_CTRL_DOWN:
    				Game_move(2);
    				break;
    			case KEY_CTRL_LEFT:
    				Game_move(3);
    				break;
    			case KEY_CTRL_DEL:
    				initGame();
    				break;
    			case KEY_CHAR_PLUS:
    				Game_addRandomTile();
    				Game_actuate();
    				break;
    			case KEY_CHAR_STORE:
    				Game_actuate();
    				break;
    			default:
    				break;
    		}
        }
    }

    return 1;
}

// Code propre au SDK. NE PAS MODIFIER !

#pragma section _BR_Size
unsigned long BR_Size;
#pragma section


#pragma section _TOP

int InitializeSystem(int isAppli, unsigned short OptionNum) {
    return INIT_ADDIN_APPLICATION(isAppli, OptionNum);
}

#pragma section

