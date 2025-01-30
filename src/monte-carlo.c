#include "monte-carlo.h"

// A minimal Monte-Carlo Tree Search implementation
// https://en.wikipedia.org/wiki/Monte_Carlo_tree_search

// Number of random games played to estimate a node
const int NB_SIMS = 4;

// Tradeoff between exploration (big value) and exploitation (low value)
const float EXPLORATION_CONSTANT = 1.41;



////////////////////////////////////////////////////////////////////////////
// Monte-Carlo Tree data structure initialization and free

mcts* createNode() { 
    // New node in the heap with no sons initialized yet
    mcts* newNode = malloc(sizeof(mcts));
    newNode->nbVisits = 0;
    newNode->nbP1Wins = 0;
    newNode->nbP2Wins = 0;

    newNode->moveArray = NULL;
    newNode->sonsArray = NULL;
    
    return newNode;
}

int initNode(mcts* node, boardState* board) {
    // Get the sons of a node and creates the sons array
    int nbWins = nbWinsFromRandomGames(board, NB_SIMS);
    node->nbVisits += NB_SIMS;

    boardMoveL* allMoves = allPossibleMoves(board);

    node->nbP1Wins += nbWins;
    node->nbP2Wins += NB_SIMS - nbWins;

    node->nbSons = boardMoveLSize(allMoves);

    if (node->nbSons == 0) {
        // Final node : if there is at least one way to win
        // for the only remaining player, it wins
        if (board->playerToPlay == 4 && nbWins < NB_SIMS) {
            nbWins = 0;
        }
        if (board->playerToPlay == 5 && nbWins > 0) {
            nbWins = NB_SIMS;
        }

    } else {
        // Sons array initialization
        node->moveArray = (boardMove*) calloc(node->nbSons, sizeof(boardMove));
        node->sonsArray = (mcts**) calloc(node->nbSons, sizeof(mcts*));

        int i = 0;
        boardMoveL* sonList = allMoves;
        while (sonList != NULL) {
            node->moveArray[i] = sonList->move;
            node->sonsArray[i] = createNode();
            i++;
            sonList = sonList->next;
        }
    }

    freeBoardMoveL(allMoves);
    return nbWins;
}

mcts* newMCTS(boardState* board) {
    mcts* newTree = createNode();
    int nbWins = initNode(newTree, board);
    return newTree;
}

int treeSize(mcts* tree) {
    return tree->nbVisits / NB_SIMS;
}

void freeNode(mcts* node) {
    if (node->moveArray != NULL) {
        free(node->moveArray);
    }
    if (node->sonsArray != NULL) {
        free(node->sonsArray);
    }
    free(node);
}

void freeMCTS(mcts* tree) {
    if (tree->nbVisits > 0) {
        for (int i = 0; i < tree->nbSons; i++) {
            freeMCTS(tree->sonsArray[i]);
        }
    }
    freeNode(tree);
}

void freeMCTSExceptOneSon(mcts* tree, int sonIndex) {
    // Useful function when a move is done so we can
    // only keep the current subtree and free the rest
    if (tree->nbVisits > 0) {
        for (int i = 0; i < tree->nbSons; i++) {
            if (i !=sonIndex) {
                freeMCTS(tree->sonsArray[i]);
            }  
        }
    }
    freeNode(tree);
}

////////////////////////////////////////////////////////////////////////////
// Random games simulation to estimate a node


bool randomGame(boardState* board) {
    // Make moves at random and returns true if penguins (P1) win
    boardState* boardCopy = copyBoardState(board);
    int consecutivePasses = 0;

    while (consecutivePasses < 2) {
        boardMoveL* allMoves = allPossibleMoves(boardCopy);

        if (allMoves == NULL) {
        consecutivePasses += 1;
        if (boardCopy->playerToPlay == 4) {
            boardCopy->playerToPlay = 5;
        } else {
            boardCopy->playerToPlay = 4;
        }
        } else {
        consecutivePasses = 0;
        int nbMoves = boardMoveLSize(allMoves);
        int randomMoveIndex = rand() % nbMoves;

        boardMove randomlySelectedMove = getMoveByIndex(allMoves, randomMoveIndex);
        movePenguin(boardCopy, randomlySelectedMove);
        }

        freeBoardMoveL(allMoves);
    }
    
    bool p1Victory = (boardCopy->p1Score > boardCopy->p2Score);
    freeBoardState(boardCopy);

    return p1Victory;
}

int nbWinsFromRandomGames(boardState* board, int nbSims) {
    int nbWins = 0;
    for (int i=0; i < nbSims; i++) {
        if (randomGame(board)) {
            nbWins += 1;
        }
    }
    return nbWins;
}


////////////////////////////////////////////////////////////////////////////
// Monte-Carlo Tree Search

float UCB(mcts* son, int nbFatherVisits, int FatherPlayer) {
    // Attractiveness score of a son based on the UCB

    if (son->nbVisits == 0) {
        return INFINITY;
    }

    float sonWinRatio;

    if (FatherPlayer == 4) {
        sonWinRatio = (float) son->nbP1Wins / (float) son->nbVisits;
    } else {
        sonWinRatio = (float) son->nbP2Wins / (float) son->nbVisits;
    }

    return sonWinRatio + EXPLORATION_CONSTANT * sqrt(log((float) nbFatherVisits) / (float) son->nbVisits);
}


int bestSonIndex(mcts* tree, int currentPlayer) {
    // Find the most attractive son based on UCB

    int bestIndex = 0;
    float bestScore = -INFINITY;

    for (int i = 0; i < tree->nbSons; i++) {
        float sonScore = UCB(tree->sonsArray[i], tree->nbVisits, currentPlayer);
        if (sonScore > bestScore) {
            bestIndex = i;
            bestScore = sonScore;
        }
    }

    return bestIndex;
}

int mctsStep(mcts* tree, boardState* board) {
    // Recursive tree update, returning the number of P1 wins 

    int nbWins;

    if (tree->nbVisits == 0) {   
        nbWins = initNode(tree, board);
        return nbWins;

    } else {
        if (tree->nbSons > 0) {
            int i = bestSonIndex(tree, board->playerToPlay);
            movePenguin(board, tree->moveArray[i]);
            nbWins = mctsStep(tree->sonsArray[i], board);
        } else {
            nbWins = nbWinsFromRandomGames(board, NB_SIMS);
        }

        tree->nbVisits += NB_SIMS;
        tree->nbP1Wins += nbWins;
        tree->nbP2Wins += NB_SIMS - nbWins;

        return nbWins;
    }
}

void mctsSteps(mcts* tree, boardState* board, int nbSteps) {
    for (int i = 0; i < nbSteps; i++) {
        boardState* boardCopy = copyBoardState(board);
        int nbWins = mctsStep(tree, boardCopy);
        freeBoardState(boardCopy);
    }
}


////////////////////////////////////////////////////////////////////////////
// Making a move !

boardMove bestMove(mcts* tree) {
    // The best move is the one that has been visited the most
    int biggestNbVisits = -1;
    int sonIndex = 0;
    for (int i = 0; i < tree->nbSons; i++) {
        if (tree->sonsArray[i]->nbVisits > biggestNbVisits) {
            biggestNbVisits = tree->sonsArray[i]->nbVisits;
            sonIndex = i;
        }
    }
    
    return tree->moveArray[sonIndex];
} 

mcts* makeMove(mcts* tree, boardMove move) {
    // Searching for the move and updating the tree

    int sonIndex = -1;
    mcts* chosenSon = NULL;

    for (int i = 0; i < tree->nbSons; i++) {

        boardMove m = tree->moveArray[i];

        if (m.start.x == move.start.x 
            && m.start.y == move.start.y
            && m.end.x == move.end.x
            && m.end.y == move.end.y) {

                sonIndex = i;
                chosenSon = tree->sonsArray[i];
            }
    }

    freeMCTSExceptOneSon(tree, sonIndex);
    return chosenSon;
}

