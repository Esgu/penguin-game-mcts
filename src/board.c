#include "board.h"
#include <stdbool.h>

// Core implementation of the game


#define ROWS 12
#define COLUMNS 13


////////////////////////////////////////////////////////////////////////////
// Types of tiles

bool isTerrain(boardState* board, boardPos current) {
    return 0 < board->map[current.x][current.y];
}

bool isReachable(boardState* board, boardPos current) {
    return 0 < board->map[current.x][current.y] && board->map[current.x][current.y] < 4;
}

////////////////////////////////////////////////////////////////////////////
// boardPos and boardMove lists manipulation

boardPosL* addPos(boardPosL* posL, boardPos pos) {
    // Add a boardPos at the beginning of the list
    boardPosL* newNode = malloc(sizeof(boardPosL));
    newNode->next = posL;
    newNode->pos = pos;
    
    return newNode;
}

boardMoveL* addMove(boardMoveL* moveL, boardMove move) {
    // Add a boardMove at the beginning of the list
    boardMoveL* newNode = malloc(sizeof(boardMoveL));
    newNode->next = moveL;
    newNode->move = move;
    
    return newNode;
}

int boardPosLSize(boardPosL* posL) {
    if (posL == NULL) {
        return 0;
    }
    return 1 + boardPosLSize(posL->next);
}

int boardMoveLSize(boardMoveL* moveL) {
    if (moveL == NULL) {
        return 0;
    }
    return 1 + boardMoveLSize(moveL->next);
}

boardMove getMoveByIndex(boardMoveL* moveL, int i) {
    // Returns the ith element of the move list

    if (moveL == NULL || i < 0) {
        return (boardMove) {.start=(boardPos){.x=0, .y=0}, .end=(boardPos){.x=0, .y=0}};
    }
    if (i == 0) {
        return moveL->move;
    }
    return getMoveByIndex(moveL->next, i-1);
}

bool posInList(boardPos pos, boardPosL* posL) {
    // Checks if a pos is in a list

    if (posL == NULL) {
        return false;
    }
    if (pos.x == posL->pos.x && pos.y == posL->pos.y) {
        return true;
    }
    return posInList(pos, posL->next);
}

boardPosL* piecesPosL(boardState* board, int pieceNumber) {
    // Get the list of positions of all pieces of a kind
    boardPosL* piecesList = NULL;
    for (int i = 0; i < board->sizeX; i++) {
        for (int j = 0; j < board->sizeY; j++) {
            if (board->map[i][j] == pieceNumber) {
                piecesList = addPos(piecesList, (boardPos) {.x=i, .y=j});
            }
        }
    }
    return piecesList;
}

void findAndReplace(boardPosL* posL, boardPos target, boardPos replace) {
    // Searches for the first occurence of a pos in a list and modifies it
    if (posL != NULL) {
        if (posL->pos.x == target.x && posL->pos.y == target.y) {
            posL->pos.x = replace.x;
            posL->pos.y = replace.y;
        } else {
            findAndReplace(posL->next, target, replace);
        }
    }
}

void freeBoardPosL(boardPosL* posL) {
    if (posL != NULL) {
        freeBoardPosL(posL->next);
    }
    free(posL);
}

void freeBoardMoveL(boardMoveL* moveL) {
    if (moveL != NULL) {
        freeBoardMoveL(moveL->next);
    }
    free(moveL);
}
////////////////////////////////////////////////////////////////////////////
// Neighbours of a position

boardPosL* addNeighbours(boardPos pos, boardState* board, boardPosL* neighboursL) {
    // Returns the list of all boardPos that can be reached from pos in the board
    boardPos current;

    // Lateral tiles
    current = (boardPos){.x = pos.x, .y = pos.y};
    while (isTerrain(board, current)) {
        current.x += 1;
        current.y += 0;
        if (isReachable(board, current)) {neighboursL = addPos(neighboursL, current);} else {break;}
    }

    current = (boardPos){.x = pos.x, .y = pos.y};
    while (isTerrain(board, current)) {
        current.x -= 1;
        current.y += 0;
        if (isReachable(board, current)) {neighboursL = addPos(neighboursL, current);} else {break;}
    }

    // Diagonal tiles
    current = (boardPos){.x = pos.x, .y = pos.y};
    while (isTerrain(board, current)) {
        current.x += current.y%2;
        current.y += 1;
        if (isReachable(board, current)) {neighboursL = addPos(neighboursL, current);} else {break;}
    }

    current = (boardPos){.x = pos.x, .y = pos.y};
    while (isTerrain(board, current)) {
        current.x += current.y%2;
        current.y -= 1;
        if (isReachable(board, current)) {neighboursL = addPos(neighboursL, current);} else {break;}       
    }

    current = (boardPos){.x = pos.x, .y = pos.y};
    while (isTerrain(board, current)) {
        current.x -= 1 - current.y%2;
        current.y += 1;
        if (isReachable(board, current)) {neighboursL = addPos(neighboursL, current);} else {break;}
    }

    current = (boardPos){.x = pos.x, .y = pos.y};
    while (isTerrain(board, current)) {
        current.x -= 1 - current.y%2;
        current.y -= 1;
        if (isReachable(board, current)) {neighboursL = addPos(neighboursL, current);} else {break;}
    }
    
    return neighboursL;
}

boardPosL* neighbours(boardPos pos, boardState* board) {
    return addNeighbours(pos, board, NULL);
}


////////////////////////////////////////////////////////////////////////////
// boardState functions

boardState* freshBoard() {
    // Fresh empty board allocation

    int** mat = (int**)calloc(ROWS, sizeof(int*));
    for (int i = 0; i < ROWS; i++) {
        mat[i] = (int*)calloc(COLUMNS, sizeof(int));
    }

    boardState* board = (boardState*)malloc(sizeof(boardState));
    board->sizeX = ROWS;
    board->sizeY = COLUMNS;
    board->map = mat;
    board->playerToPlay = 4;
    board->p1Score = 0;
    board->p2Score = 0;

    return board;
}

void initializeBoard(boardState* board) {
    // Board initialization with random amount of fishes

    int placeHolders[ROWS][COLUMNS] = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0},
        {0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0},
        {0, 0, 1, 4, 1, 1, 1, 1, 1, 5, 1, 0, 0},
        {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
        {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
        {0, 0, 1, 4, 1, 1, 1, 1, 1, 5, 1, 0, 0},
        {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
        {0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    };
    for (int i = 0; i < board->sizeX; i++) {
        for (int j = 0; j < board->sizeY; j++) {

            if (placeHolders[i][j] == 1) {
                board->map[i][j] = rand() % 3 + 1;
            } else {
                board->map[i][j] = placeHolders[i][j];
            }

        }
    }

    board->p1Pieces = piecesPosL(board, 4);
    board->p2Pieces = piecesPosL(board, 5);
}

boardState* copyBoardState(boardState* board) {
    // Allocates of copy of a given board

    int** mat = (int**)calloc(ROWS, sizeof(int*));
    for (int i = 0; i < ROWS; i++) {
        mat[i] = (int*)calloc(COLUMNS, sizeof(int));
    }

    boardState* boardCopy = (boardState*) malloc(sizeof(boardState));
    boardCopy->sizeX = ROWS;
    boardCopy->sizeY = COLUMNS;
    boardCopy->map = mat;
    boardCopy->playerToPlay = board->playerToPlay;
    boardCopy->p1Score = board->p1Score;
    boardCopy->p2Score = board->p2Score;

    for (int i = 0; i < board->sizeX; i++) {
        for (int j = 0; j < board->sizeY; j++) {
            boardCopy->map[i][j] = board->map[i][j]; 
        }
    }

    boardCopy->p1Pieces = piecesPosL(board, 4);
    boardCopy->p2Pieces = piecesPosL(board, 5);

    return boardCopy;
}

void freeBoardState(boardState* board) {
    for (int i = 0; i < ROWS; i++) {
        free(board->map[i]);
    }
    free(board->map);
    freeBoardPosL(board->p1Pieces);
    freeBoardPosL(board->p2Pieces);
    free(board);
}


////////////////////////////////////////////////////////////////////////////
// Moving the pieces

boardMoveL* addBoardMoves(boardPos start, boardPosL* ends, boardMoveL* otherMoves) {
    // All moves from a starting position and a list of end positions
    boardMoveL* moves = otherMoves;

    while (ends != NULL) {
        moves = addMove(moves, (boardMove) {.start=start, .end=ends->pos});
        ends = ends->next;
    }

    return moves;
}

boardMoveL* allPossibleMoves(boardState* board) {
    // Returns the list of all possible moves for current player

    boardMoveL* allMoves = NULL;
    boardPosL* playingPiecesPos = NULL;

    if (board->playerToPlay == 4) {
        playingPiecesPos = board->p1Pieces;
    } else {
        playingPiecesPos = board->p2Pieces;
    }

    while (playingPiecesPos != NULL) {
        boardPosL* reachablePos = neighbours(playingPiecesPos->pos, board);
        allMoves = addBoardMoves(playingPiecesPos->pos, reachablePos, allMoves);
        playingPiecesPos = playingPiecesPos->next;
        freeBoardPosL(reachablePos);
    }

    return allMoves;
}

bool currentPlayerCanPlay(boardState* board) {
    boardMoveL* allMoves = allPossibleMoves(board);
    return (allMoves != NULL);
}

void movePenguin(boardState* board, boardMove move) {
    // Update the board to make a move

    if (board->playerToPlay == 4) {
        board->p1Score += board->map[move.end.x][move.end.y];
        board->map[move.end.x][move.end.y] = 4;
        board->map[move.start.x][move.start.y] = 0;
        board->playerToPlay = 5;

        findAndReplace(board->p1Pieces, move.start, move.end);

    } else {
        board->p2Score += board->map[move.end.x][move.end.y];
        board->map[move.end.x][move.end.y] = 5;
        board->map[move.start.x][move.start.y] = 0;
        board->playerToPlay = 4;

        findAndReplace(board->p2Pieces, move.start, move.end);
    }
}

