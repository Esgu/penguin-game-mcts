#include <stdbool.h>
#include <stdlib.h>

#include "raylib.h"
#include "raymath.h"
#include "math.h"

#include "board.h"

////////////////////////////////////////////////////////////////////////////
// Data structures

typedef struct _mcts {

    int nbVisits;
    int nbP1Wins;
    int nbP2Wins;

    int nbSons;

    boardMove* moveArray;
    struct _mcts** sonsArray;

} mcts;

////////////////////////////////////////////////////////////////////////////
// Monte-Carlo Tree data structure initialization and free

mcts* createNode();
int initNode(mcts* node, boardState* board);
mcts* newMCTS(boardState* board);
int treeSize(mcts* tree);
void freeNode(mcts* node);
void freeMCTS(mcts* tree);
void freeMCTSExceptOneSon(mcts* tree, int sonIndex);

////////////////////////////////////////////////////////////////////////////
// Random games simulation to estimate a node

bool randomGame(boardState* board);
int nbWinsFromRandomGames(boardState* board, int nbSims);

////////////////////////////////////////////////////////////////////////////
// Monte-Carlo Tree Search

float UCB(mcts* son, int nbFatherVisits, int FatherPlayer);
int bestSonIndex(mcts* tree, int currentPlayer);
int mctsStep(mcts* tree, boardState* board);
void mctsSteps(mcts* tree, boardState* board, int nbSteps);


////////////////////////////////////////////////////////////////////////////
// Making a move !

boardMove bestMove(mcts* tree);
mcts* makeMove(mcts* tree, boardMove move);
