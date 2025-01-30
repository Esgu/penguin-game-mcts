#ifndef BOARD_H
#define BOARD_H

#include <stdbool.h>
#include <stdlib.h>

////////////////////////////////////////////////////////////////////////////
// Data structures

typedef struct _boardPos {
    int x;
    int y;
} boardPos;

typedef struct _boardPosL {
    boardPos pos;
    struct _boardPosL* next;
} boardPosL;

typedef struct _boardMove {
    boardPos start;
    boardPos end;
} boardMove;

typedef struct _boardMoveL {
    boardMove move;
     struct _boardMoveL* next;
} boardMoveL;

typedef struct _boardState {
    int sizeX;
    int sizeY;

    int** map;

    int playerToPlay;
    int p1Score;
    int p2Score;

    boardPosL* p1Pieces;
    boardPosL* p2Pieces;
} boardState;


////////////////////////////////////////////////////////////////////////////
// Types of tiles

bool isTerrain(boardState* board, boardPos current);
bool isReachable(boardState* board, boardPos current);

////////////////////////////////////////////////////////////////////////////
// boardPos and boardMove lists manipulation

boardPosL* addPos(boardPosL* posL, boardPos pos);
boardMoveL* addMove(boardMoveL* moveL, boardMove move);

int boardPosLSize(boardPosL* posL);
int boardMoveLSize(boardMoveL* moveL);

boardMove getMoveByIndex(boardMoveL* moveL, int i);
bool posInList(boardPos pos, boardPosL* posL);
boardPosL* piecesPosL(boardState* board, int pieceNumber);
void findAndReplace(boardPosL* posL, boardPos target, boardPos replace);

void freeBoardPosL(boardPosL* posL);
void freeBoardMoveL(boardMoveL* moveL);

////////////////////////////////////////////////////////////////////////////
// Neighbours of a position

boardPosL* addNeighbours(boardPos pos, boardState* board, boardPosL* neighboursL);
boardPosL* neighbours(boardPos pos, boardState* board);

////////////////////////////////////////////////////////////////////////////
// boardState functions

boardState* freshBoard(void);
void initializeBoard(boardState* board);
boardState* copyBoardState(boardState* board);
void freeBoardState(boardState* board);

////////////////////////////////////////////////////////////////////////////
// Moving the pieces

boardMoveL* addBoardMoves(boardPos start, boardPosL* ends, boardMoveL* otherMoves);
boardMoveL* allPossibleMoves(boardState* board);
bool currentPlayerCanPlay(boardState* board);
void movePenguin(boardState* board, boardMove move);


#endif