#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include "raylib.h"
#include "raymath.h"
#include "math.h"

#include "render.h"
#include "monte-carlo.h"


// Penguin game with Monte-Carlo Tree Search


int main(void) {

    srand(time(NULL));

    // Window initialisation
    InitWindow(0, 0, "Jeu des pingouins");
    const int WINDOWS_SIZE_X = GetScreenWidth();
    const int WINDOWS_SIZE_Y = GetScreenHeight();
    SetTargetFPS(60);

    // Board initialisation
    boardState* mainBoard = freshBoard();
    initializeBoard(mainBoard);

    // Cam initialisation
    Camera3D camera = {0};
    float camAngle = 0.0f;
    float dtheta = 0.1f;
    camera.position = (Vector3){cos(camAngle) * 30.0f, 32.0f, sin(camAngle) * 30.0f };
    camera.target = (Vector3){26.0f, 0.0f, 20.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;

    // 3D models and animation stuff
    load3DModels();
    pieceModelL* piecesModels = createPiecesModels(mainBoard);
    assert(piecesModels != NULL);

    // Monte-Carlo Tree initialisation
    mcts* tree = newMCTS(mainBoard);
    int countDown = 0;
    const int AI_THINKING_FRAMES = 300;
    const int NB_TREE_STEPS = 10;

    // Interface 
    bool showDetails = true;
    int gameMode = 0; // 0 -> human vs human game, 1 -> human vs AI game, 2 -> AI vs AI game


    ////////////////////////////////////////////////////////////////////////////////////////////
    // Main loop of the game
    while (!WindowShouldClose()) {

        // Cam update
        camAngle += dtheta;
        camera.position = (Vector3){cos(camAngle) * 35.0f + 18.0f, 30.0f, sin(camAngle) * 35.0f + 24.0f };
        UpdateCamera(&camera, CAMERA_PERSPECTIVE);

        if (IsKeyDown(KEY_RIGHT) && dtheta < 0.05f) {
            dtheta += 0.01f;
        } else {
            if (IsKeyDown(KEY_LEFT) && dtheta > -0.05f) {
                dtheta -= 0.01f;
            } else {
                dtheta *= 0.9f;
            }
        }

        // Interface update
        if (IsKeyPressed(KEY_A)) {
            showDetails = !showDetails;
        }
        if (IsKeyPressed(KEY_SPACE)) {
            gameMode = (gameMode + 1) % 3;
            if (countDown == 0 && (gameMode == 1 || gameMode == 2)) {
                countDown = AI_THINKING_FRAMES;
            }
        }
        if (IsKeyPressed(KEY_D)) {
            makePiecesDance(piecesModels);
        }

        // The AI is thinking...
        mctsSteps(tree, mainBoard, NB_TREE_STEPS);

        // The AI is moving
        if (countDown == 1) {
            if (tree->nbSons > 0) {
                boardMove suggestedMove = bestMove(tree);

                movePenguin(mainBoard, suggestedMove);
                tree = makeMove(tree, suggestedMove);
                assert(tree != NULL);

                updatePiecesWithMove(piecesModels, suggestedMove);
            } 
            if (gameMode == 2) {
                // In a AI vs AI game, AI thinks again
                countDown = AI_THINKING_FRAMES;
            }     
        }
        if (countDown > 0) {
            countDown -= 1;
        }
       

        // Board rendering
        BeginDrawing();

        ClearBackground(RAYWHITE);
        BeginMode3D(camera);

        renderBoard(mainBoard);
        renderPieces(piecesModels);

        if (countDown > 0 && tree->nbSons > 0) {
            boardMove suggestedMove = bestMove(tree);
            drawMove(suggestedMove);
        }

        boardMoveL* movesDetected = updateSelectedPos(mainBoard, camera, piecesModels, IsMouseButtonPressed(MOUSE_BUTTON_LEFT));
        if (movesDetected != NULL && countDown == 0) {
            boardMove moveToDo = movesDetected->move;
            movePenguin(mainBoard, moveToDo);
            updatePiecesWithMove(piecesModels, moveToDo);
            tree = makeMove(tree, moveToDo);
            assert(tree != NULL);

            if (gameMode == 1) {
                countDown = AI_THINKING_FRAMES;
            }
        }
        freeBoardMoveL(movesDetected);

        EndMode3D();

        // Interface
        DrawText(TextFormat("%i", mainBoard->p1Score), WINDOWS_SIZE_X / 50, WINDOWS_SIZE_Y / 20, WINDOWS_SIZE_X / 12, WHITE);
        DrawText(TextFormat("%i", mainBoard->p2Score), WINDOWS_SIZE_X * 9 / 10, WINDOWS_SIZE_Y / 20, WINDOWS_SIZE_X / 12, WHITE);
        DrawFPS(10, 10);

        if (showDetails) {
            DrawText(TextFormat("Tree size : %i", treeSize(tree)), WINDOWS_SIZE_X / 4, WINDOWS_SIZE_Y / 9, WINDOWS_SIZE_X / 48, WHITE);
            drawWinningEstimation(WINDOWS_SIZE_X / 4, WINDOWS_SIZE_Y / 15, WINDOWS_SIZE_X / 2, (float) tree->nbP1Wins / (float) tree->nbVisits);

            if (gameMode == 0) {DrawText("Duel mode (space to change)", WINDOWS_SIZE_X / 4, WINDOWS_SIZE_Y / 40, WINDOWS_SIZE_X / 48, WHITE);}
            if (gameMode == 1) {DrawText("Human vs AI mode (space to change)", WINDOWS_SIZE_X / 4, WINDOWS_SIZE_Y / 40, WINDOWS_SIZE_X / 48, WHITE);}
            if (gameMode == 2) {DrawText("AI vs AI mode (space to change)", WINDOWS_SIZE_X / 4, WINDOWS_SIZE_Y / 40, WINDOWS_SIZE_X / 48, WHITE);}
        }

        

        EndDrawing();
    }

    freeMCTS(tree);
    freeBoardState(mainBoard);
    unloadAllModels(piecesModels);
    CloseWindow();
    
    return 0;
}

