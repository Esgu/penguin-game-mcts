#include "render.h"

// Ugly functions to render the game in 3D based on the board

const float HEX_SIZE = 2.0f;
const float HEX_HEIGHT = 0.3f;
const float HEX_SPACING = 4.0f;

float selectedSizeIncrease = 0.0f; // Size increase when a tile is clicked on
boardPos selectedPos = (boardPos) {.x = 0, .y = 0}; // Memorize the last selected tile

Ray ray = {0};  
RayCollision collision = {0}; 

Model blueFish;
Model purpleFish;
Model magentaFish;
ModelAnimation* penguinAnimations;
ModelAnimation* crocoAnimations;

unsigned int nbPenguinAnims;
unsigned int nbCrocoAnims;


////////////////////////////////////////////////////////////////////////////
// Load 3D models

void load3DModels() {

    penguinAnimations = LoadModelAnimations("../resources/models/penguin/penguinAnims.glb", &nbPenguinAnims);
    crocoAnimations = LoadModelAnimations("../resources/models/crocodile/crocoAnims.glb", &nbCrocoAnims);

    blueFish = LoadModel("../resources/models/fish/scene.gltf");
    Texture2D blueTexture = LoadTexture("../resources/models/fish/textures/blueFish.png");
    blueFish.materials[2].maps[MATERIAL_MAP_DIFFUSE].texture = blueTexture; 
    blueFish.transform = MatrixMultiply(MatrixMultiply(MatrixTranslate(1.5f, 0.0f, 0.0f), MatrixScale(0.15f, 0.15f, 0.15f)), MatrixRotateZ(PI / 2.0f));

    purpleFish = LoadModel("../resources/models/fish/scene.gltf");
    Texture2D purpleTexture = LoadTexture("../resources/models/fish/textures/purpleFish.png");
    purpleFish.materials[2].maps[MATERIAL_MAP_DIFFUSE].texture = purpleTexture; 
    purpleFish.transform = MatrixMultiply(MatrixMultiply(MatrixTranslate(1.5f, 0.0f, 0.0f), MatrixScale(0.15f, 0.15f, 0.15f)), MatrixRotateZ(PI / 2.0f));

    magentaFish = LoadModel("../resources/models/fish/scene.gltf");
    Texture2D magentaTexture = LoadTexture("../resources/models/fish/textures/magentaFish.png");
    magentaFish.materials[2].maps[MATERIAL_MAP_DIFFUSE].texture = magentaTexture; 
    magentaFish.transform = MatrixMultiply(MatrixMultiply(MatrixTranslate(1.5f, 0.0f, 0.0f), MatrixScale(0.15f, 0.15f, 0.15f)), MatrixRotateZ(PI / 2.0f));  

}


////////////////////////////////////////////////////////////////////////////
// 3D models and animations for penguins and crocodiles

pieceModel* createPieceModel(boardPos pos, bool isPenguin) {
    // A 3D pieceModel contains information about the model, position, angle, animation...

    pieceModel* newPiece = malloc(sizeof(pieceModel));
    newPiece->pos = pos;
    newPiece->angle = 0.0f;

    newPiece->isPenguin = isPenguin;
    
    newPiece->isCurrentlyMoving = false;

    newPiece->movingProgress = 0.0f;
    newPiece->currentMove = (boardMove) {.start = (boardPos) {.x = 0, .y = 0}, .end = (boardPos) {.x = 0, .y = 0}};

    if (isPenguin) {
        newPiece->pieceModel = LoadModel("../resources/models/penguin/penguinAnims.glb"); 
        Texture2D penguinTexture = LoadTexture("../resources/models/penguin/textures/penguin_color.jpg");
        newPiece->pieceModel.materials[1].maps[MATERIAL_MAP_DIFFUSE].texture = penguinTexture; 
        newPiece->pieceModel.transform = MatrixMultiply(MatrixTranslate(0.0f, 0.0f, 0.0f), MatrixMultiply(MatrixRotateX(PI / 2.0f), MatrixScale(0.06f, 0.06f, 0.06f)));
    } else {
        newPiece->pieceModel = LoadModel("../resources/models/crocodile/crocoAnims.glb"); 
        Texture2D crocoTexture = LoadTexture("../resources/models/crocodile/textures/croco_color.jpg");
        newPiece->pieceModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = crocoTexture; 
        newPiece->pieceModel.transform = MatrixMultiply(MatrixTranslate(0.0f, 0.0f, 0.0f), MatrixMultiply(MatrixRotateX(PI / 2.0f), MatrixScale(5.0f, 5.0f, 5.0f)));
    }

    newPiece->currentAnimation = 1;
    newPiece->currentAnimationFrame = 0;

    return newPiece;
}

pieceModelL* createPiecesModels(boardState* board) {
    // Creates a list of all pieces models in a given board
    pieceModelL* pieces = NULL;

    boardPosL* p1Pieces = board->p1Pieces;
    while (p1Pieces != NULL) {
        pieceModelL* newNode = malloc(sizeof(pieceModelL));

        newNode->piece = createPieceModel(p1Pieces->pos, true);
        newNode->next = pieces;

        p1Pieces = p1Pieces->next;
        pieces = newNode;
    }
    
    boardPosL* p2Pieces = board->p2Pieces;
    while (p2Pieces != NULL) {
        pieceModelL* newNode = malloc(sizeof(pieceModelL));

        newNode->piece = createPieceModel(p2Pieces->pos, false);
        newNode->next = pieces;

        p2Pieces = p2Pieces->next;
        pieces = newNode;
    }
    
    return pieces;
}


////////////////////////////////////////////////////////////////////////////
// Useful functions to manipulate 3D models

void changeAnimation(pieceModel* piece, int newAnimationIndex, int newStartingFrame) {
    piece->currentAnimation = newAnimationIndex;
    piece->currentAnimationFrame = newStartingFrame;
}

pieceModel* getPieceModelByPosition(pieceModelL* pieces, boardPos pos) {
    while (pieces != NULL) {
        if (pieces->piece->pos.x == pos.x && pieces->piece->pos.y == pos.y) {
            return pieces->piece;
        }
        pieces = pieces->next;
    }
    return NULL;
}

void updatePiecesWithMove(pieceModelL* pieces, boardMove move) {
    pieceModel* selectedPiece = getPieceModelByPosition(pieces, move.start);
    if (selectedPiece != NULL) {
        changeAnimation(selectedPiece, 3, 0);
        selectedPiece->isCurrentlyMoving = true;
        selectedPiece->currentMove = move;
    }
}

void makePiecesDance(pieceModelL* pieces) {
    while (pieces != NULL) {
        if (pieces->piece->currentAnimation != 3) {
            changeAnimation(pieces->piece, 0, 0);
        }
        pieces = pieces->next;
     }
}

////////////////////////////////////////////////////////////////////////////
// Free allocated stuff

void freePieceModel(pieceModel* piece) {
    UnloadModel(piece->pieceModel);
    free(piece);
}

void freePieceModelL(pieceModelL* pieces) {
    if (pieces != NULL) {
        freePieceModelL(pieces->next);
        freePieceModel(pieces->piece);
    }
    free(pieces);
}

void unloadAllModels(pieceModelL* pieces) {
    UnloadModel(blueFish);
    UnloadModel(purpleFish);
    UnloadModel(magentaFish);
    freePieceModelL(pieces);
}

////////////////////////////////////////////////////////////////////////////
// Rendering the pieces

Vector3 renderPosFromBoardPos(boardPos pos, float y) {
    // Get the position in the 3D world from a boardPos
    float x = pos.x * HEX_SPACING + (pos.y % 2) * (HEX_SPACING / 2.0f);
    float z = pos.y * HEX_SPACING * 0.866f;
    return (Vector3) {.x=x, .y=y, .z=z};
}


void renderPieces(pieceModelL* pieces) {
    // Render all the pieces, either idle or running

    while (pieces != NULL) {
        ModelAnimation anim;

        if (pieces->piece->isPenguin) {
            ModelAnimation anim = penguinAnimations[pieces->piece->currentAnimation];
        } else {
            ModelAnimation anim = crocoAnimations[pieces->piece->currentAnimation];
        }

        pieces->piece->currentAnimationFrame = (pieces->piece->currentAnimationFrame + 1) % anim.frameCount;
        UpdateModelAnimation(pieces->piece->pieceModel, anim, pieces->piece->currentAnimationFrame);

        Vector3 startPos = renderPosFromBoardPos(pieces->piece->pos, 0.6f);
        
        if (pieces->piece->isCurrentlyMoving) {
            Vector3 endPos = renderPosFromBoardPos(pieces->piece->currentMove.end, 0.6f);

            float distance = sqrt(pow(endPos.x - startPos.x, 2) + pow(endPos.y - startPos.y, 2));
            
            pieces->piece->pieceModel.transform = MatrixMultiply(pieces->piece->pieceModel.transform, MatrixRotateY(-pieces->piece->angle));
            float angle = 3 * PI / 2 - atan2(startPos.z - endPos.z, startPos.x - endPos.x);
            pieces->piece->angle = angle;
            pieces->piece->pieceModel.transform = MatrixMultiply(pieces->piece->pieceModel.transform, MatrixRotateY(pieces->piece->angle));
            


            if (pieces->piece->movingProgress < distance && distance > 0) {
                // Running
                Vector3 currentPos = 
                {.x = startPos.x + (endPos.x - startPos.x) / distance * pieces->piece->movingProgress, 
                .y = 0.6f, 
                .z = startPos.z + (endPos.z - startPos.z) / distance * pieces->piece->movingProgress};

                DrawModel(pieces->piece->pieceModel, currentPos, 1.0f, WHITE);
                pieces->piece->movingProgress += 0.2f;

            }

            if (pieces->piece->movingProgress >= distance) {
                // Stop running
                pieces->piece->isCurrentlyMoving = false;
                pieces->piece->pos = pieces->piece->currentMove.end;
                pieces->piece->movingProgress = 0.0f;
                changeAnimation(pieces->piece, 1, 0);
            }
     
        } else {
            // Idle
            DrawModel(pieces->piece->pieceModel, startPos, 1.0f, WHITE);
        }

        pieces = pieces->next;
    }
}

bool isPlayingPieceSelected(boardState* board) {
    return board->map[selectedPos.x][selectedPos.y] == board->playerToPlay;
}

////////////////////////////////////////////////////////////////////////////
// Rendering the board

void drawHexagonalPrism(Vector3 position, bool isSelected) {
    // Iceberg rendering

    float size = HEX_SIZE;
    float height = HEX_HEIGHT;

    if (isSelected) {
        size +=selectedSizeIncrease;
    }

    Vector3 baseVertices[6];

    float angle = 2.0f * PI / 6.0f;
    for (int i = 0; i < 6; i++) {
        baseVertices[i].x = position.x + size * cosf((i + 0.5) * angle);
        baseVertices[i].z = position.z + size * sinf((i + 0.5) * angle);
        baseVertices[i].y = position.y;
    }

    // Iceberg base
    for (int i = 0; i < 6; i++) {
        DrawLine3D(baseVertices[i], baseVertices[(i + 1) % 6], SKYBLUE);
    }

    // Iceberg top
    Vector3 topVertices[6];
    for (int i = 0; i < 6; i++) {
        topVertices[i] = (Vector3){
            position.x + size * cosf((i + 0.5) * angle),
            position.y + height,
            position.z + size * sinf((i + 0.5) * angle)
        };
    }

    // Iceberg sides
    for (int i = 0; i < 6; i++) {
        Vector3 p1 = baseVertices[i];
        Vector3 p2 = baseVertices[(i + 1) % 6];
        Vector3 p3 = topVertices[i];
        Vector3 p4 = topVertices[(i + 1) % 6];

        if (isSelected) {
            DrawTriangle3D(p1, p3, p2, ORANGE); 
            DrawTriangle3D(p2, p3, p4, ORANGE);
        } else {
            DrawTriangle3D(p1, p3, p2, SKYBLUE); 
            DrawTriangle3D(p2, p3, p4, SKYBLUE);
        }
    }

    // Iceberg top
    for (int i = 0; i < 6; i++) {
        Vector3 p1 = topVertices[i];
        Vector3 p2 = topVertices[(i + 1) % 6];
        Vector3 p3 = position;

        if (isSelected) {
            DrawTriangle3D(p1, p3, p2, ORANGE);
        } else {
            DrawTriangle3D(p1, p3, p2, (Color){0xd8, 0xee, 0xff, 0xff});
        }

    }
}

void drawNeighbours(boardPosL* neighboursL) {
    // Small highlights when a piece is selected to show possible moves
    while (neighboursL != NULL) {

        Vector3 neighbourPos = renderPosFromBoardPos(neighboursL->pos, 0.8f);

        DrawCylinder(neighbourPos, 1.5f-selectedSizeIncrease, 1.5f-selectedSizeIncrease, 0.01f, 20, (Color){0x44, 0x00, 0x11, 0x22});
        neighboursL = neighboursL->next;
    }
}

void drawMove(boardMove move) {
    // Draws an arrow to indicate a move on the board
    Vector3 startPos = renderPosFromBoardPos((boardPos){.x=move.start.x, .y=move.start.y}, 1.0f);
    Vector3 endPos = renderPosFromBoardPos((boardPos){.x=move.end.x, .y=move.end.y}, 1.0f);

    float angle = atan2(startPos.z - endPos.z, startPos.x - endPos.x);

    Vector3 newDot = (Vector3) {.x=endPos.x + cosf(angle + PI/5.0f) * 2.0f, .y=1.0f, .z=endPos.z + sinf(angle + PI/5.0f) * 2.0f};
    Vector3 newDot2 = (Vector3) {.x=endPos.x + cosf(angle - PI/5.0f) * 2.0f, .y=1.0f, .z=endPos.z + sinf(angle - PI/5.0f) * 2.0f};

    Vector3 base1 = (Vector3) {.x=startPos.x + cosf(angle + PI/2.0f) * 0.5f, .y=1.0f, .z=startPos.z + sinf(angle + PI/2.0f) * 0.5f};
    Vector3 base2 = (Vector3) {.x=startPos.x + cosf(angle - PI/2.0f) * 0.5f, .y=1.0f, .z=startPos.z + sinf(angle - PI/2.0f) * 0.5f};
    Vector3 end1 = (Vector3) {.x=endPos.x + cosf(angle + PI/2.0f) * 0.5f + cosf(angle) * 0.7f, .y=1.0f, .z=endPos.z + sinf(angle + PI/2.0f) * 0.5f + sinf(angle) * 0.7f};
    Vector3 end2 = (Vector3) {.x=endPos.x + cosf(angle - PI/2.0f) * 0.5f + cosf(angle) * 0.7f, .y=1.0f, .z=endPos.z + sinf(angle - PI/2.0f) * 0.5f + sinf(angle) * 0.7f};

    DrawTriangle3D(base1, base2, end1, ORANGE);
    DrawTriangle3D(end1, base2, end2, ORANGE);
    DrawTriangle3D(endPos, newDot, newDot2, ORANGE);
    
}

void renderBoard(boardState* board) {

    DrawPlane((Vector3){0.0f, -1.0f, 0.0f}, (Vector2){5000.0f, 5000.0f}, (Color){0x03, 0x52, 0x8e, 0xff});

    for (int i = 0; i < board->sizeX; i++) {
        for (int j = 0; j < board->sizeY; j++) {

            Vector3 renderPos = renderPosFromBoardPos((boardPos){.x=i, .y=j}, 0.0f);
            
            if (board->map[i][j] > 0) {
                // Iceberg
                float rotationAngle = PI / 6.0f;
                drawHexagonalPrism((Vector3){renderPos.x, 0.5f, renderPos.z}, (i == selectedPos.x && j == selectedPos.y));
            }

            if (board->map[i][j] == 1) {
                DrawModel(blueFish, (Vector3){renderPos.x, 0.6f, renderPos.z}, 1.0f, WHITE);
            }

            if (board->map[i][j] == 2) {
                DrawModel(purpleFish, (Vector3){renderPos.x - 0.7f, 0.6f, renderPos.z}, 1.0f, WHITE);
                DrawModel(purpleFish, (Vector3){renderPos.x + 0.7f, 0.6f, renderPos.z}, 1.0f, WHITE);
            }

            if (board->map[i][j] == 3) {
                DrawModel(magentaFish, (Vector3){renderPos.x - 0.3f, 0.6f, renderPos.z}, 1.0f, WHITE);
                DrawModel(magentaFish, (Vector3){renderPos.x + 0.3f, 0.6f, renderPos.z}, 1.0f, WHITE);
                DrawModel(magentaFish, (Vector3){renderPos.x, 1.0f, renderPos.z}, 1.0f, WHITE);
            }

        }
    }

    if (isPlayingPieceSelected(board)) {
        boardPosL* neighboursOfSelected = neighbours(selectedPos, board);
        drawNeighbours(neighboursOfSelected);
        freeBoardPosL(neighboursOfSelected);
    }

    selectedSizeIncrease *= 0.9;
    
}


////////////////////////////////////////////////////////////////////////////
// Hitboxes and click detection

BoundingBox hexHitbox(int i, int j) {
    // A basic box to intercept the ray from the camera
    Vector3 renderPos = renderPosFromBoardPos((boardPos){.x=i, .y=j}, HEX_HEIGHT);

    return (BoundingBox){(Vector3){renderPos.x - 1.5f, renderPos.y - 0.1f, renderPos.z - 1.5f},
                        (Vector3){renderPos.x + 1.5f, renderPos.y + 0.1f, renderPos.z + 1.5f}};
}

boardMoveL* updateSelectedPos(boardState* mainBoard, Camera3D camera, pieceModelL* pieces, bool hasClicked) {
    // Check if any iceberg hitbox intercepts the camera ray
    // Returns a move list containing the move to do, or an empty move list
    collision.hit = false;

    boardMoveL* movesDetected = NULL;

    boardPosL* neighboursOfSelected = neighbours(selectedPos, mainBoard);
            
    ray = GetScreenToWorldRay(GetMousePosition(), camera);

    for (int i=0; i < mainBoard->sizeX; i++) {
        for (int j=0; j < mainBoard->sizeY; j++) {
            collision = GetRayCollisionBox(ray, hexHitbox(i, j));
            if (collision.hit) {
                
                boardPos hitPos = (boardPos){.x = i, .y = j};
                if (isPlayingPieceSelected(mainBoard) && posInList(hitPos, neighboursOfSelected)) {

                    if (hasClicked) {
                        // The move is played
                        boardMove moveToPlay = (boardMove) {.start = (boardPos){.x = selectedPos.x, .y = selectedPos.y}, hitPos};
                        
                        selectedPos.x = 0;
                        selectedPos.y = 0;

                        movesDetected = addMove(movesDetected, moveToPlay);
                    } else {
                        drawMove((boardMove) {.start = selectedPos, .end=hitPos});
                    }

                } else {
                    if (hasClicked) {
                        // A tile is selected
                        pieceModel* selectedPiece = getPieceModelByPosition(pieces,hitPos);
                        if (selectedPiece != NULL) {
                            changeAnimation(selectedPiece, 2, 60);
                        }

                        selectedPos.x = i;
                        selectedPos.y = j;
                        selectedSizeIncrease = 0.8f;
                    }
                }   
            
            }
        }
    }    

    freeBoardPosL(neighboursOfSelected);
    return movesDetected;       
}

////////////////////////////////////////////////////////////////////////////
// Rendering estimation bar

void drawWinningEstimation(int posX, int posY, int width, float winRatio) {
    int sepPos = (int) (winRatio * width);
    DrawRectangle(posX, posY, sepPos, 50, WHITE);
    DrawRectangle(posX + sepPos , posY, width - sepPos, 50, DARKGREEN);
    DrawRectangle(posX + width / 2 - 5, posY, 10, 50, BLACK);
}