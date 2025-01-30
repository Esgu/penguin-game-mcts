#include <stdbool.h>
#include <stdlib.h>

#include "raylib.h"
#include "raymath.h"
#include "math.h"

#include "board.h"

////////////////////////////////////////////////////////////////////////////
// Data structures

typedef struct _pieceModel {
    boardPos pos;
    float angle;

    bool isPenguin;
    bool isCurrentlyMoving;

    float movingProgress;
    boardMove currentMove;

    Model pieceModel;
    ModelAnimation* animations;
    int currentAnimation;
    int currentAnimationFrame;

} pieceModel;

typedef struct _pieceModelL {
    pieceModel* piece;
    struct _pieceModelL* next;
} pieceModelL;


////////////////////////////////////////////////////////////////////////////
// Load 3D models

void load3DModels();

////////////////////////////////////////////////////////////////////////////
// 3D models and animations for penguins and crocodiles

pieceModel* createPieceModel(boardPos pos, bool isPenguin);
pieceModelL* createPiecesModels(boardState* board);

////////////////////////////////////////////////////////////////////////////
// Useful functions to manipulate 3D models

void changeAnimation(pieceModel* piece, int newAnimationIndex, int newStartingFrame);
pieceModel* getPieceModelByPosition(pieceModelL* pieces, boardPos pos);
void updatePiecesWithMove(pieceModelL* pieces, boardMove move);
void makePiecesDance(pieceModelL* pieces);

////////////////////////////////////////////////////////////////////////////
// Free allocated stuff

void freePieceModel(pieceModel* piece);
void freePieceModelL(pieceModelL* pieces);
void unloadAllModels(pieceModelL* pieces);

////////////////////////////////////////////////////////////////////////////
// Rendering the pieces

Vector3 renderPosFromBoardPos(boardPos pos, float y);
void renderPieces(pieceModelL* pieces);
bool isPlayingPieceSelected(boardState* board);

////////////////////////////////////////////////////////////////////////////
// Rendering the board

void drawHexagonalPrism(Vector3 position, bool isSelected);
void drawNeighbours(boardPosL* neighboursL);
void drawMove(boardMove move);
void renderBoard(boardState* board);

////////////////////////////////////////////////////////////////////////////
// Hitboxes and click detection

BoundingBox hexHitbox(int i, int j);
boardMoveL* updateSelectedPos(boardState* mainBoard, Camera3D camera, pieceModelL* pieces,bool hasClicked);

////////////////////////////////////////////////////////////////////////////
// Rendering estimation bar

void drawWinningEstimation(int posX, int posY, int width, float winRatio);
