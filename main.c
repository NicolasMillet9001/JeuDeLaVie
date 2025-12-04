#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL.h>

#define GENERATION_DELAY_MS 100

typedef struct {
    int alive;
    int age;
} Cell;

int N;
int CELL_SIZE;

void drawGrid(SDL_Renderer *renderer, Cell **grid, int cell_size, int offset_x, int offset_y) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect border = { offset_x - 1, offset_y - 1, N * cell_size + 2, N * cell_size + 2 };
    SDL_RenderDrawRect(renderer, &border);

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (grid[i][j].alive) {
                Uint8 age = grid[i][j].age;
                Uint8 r = 255;
                Uint8 g = 255 - (age > 255 ? 255 : age);
                Uint8 b = 255 - (age > 255 ? 255 : age);
                SDL_SetRenderDrawColor(renderer, r, g, b, 255);
                SDL_Rect cell = { offset_x + j * cell_size, offset_y + i * cell_size, cell_size, cell_size };
                SDL_RenderFillRect(renderer, &cell);
            }
        }
    }
    SDL_RenderPresent(renderer);
}

void nextGeneration(Cell **matrice, Cell **matriceNplus1, int n, int i, int j) {
    int compteurDeCellulesAdjacentesVivantes = 0;
    for (int compteuri = i-1; compteuri <= i+1; compteuri++) {
        if (compteuri >= 0 && compteuri < n) {
            for (int compteurj = j-1; compteurj <= j+1; compteurj++) {
                if (compteurj >= 0 && compteurj < n) {
                    if (matrice[compteuri][compteurj].alive) {
                        if (i != compteuri || j != compteurj) {
                            compteurDeCellulesAdjacentesVivantes++;
                        }
                    }
                }
            }
        }
    }
    if (matrice[i][j].alive) {
        if (compteurDeCellulesAdjacentesVivantes == 2 || compteurDeCellulesAdjacentesVivantes == 3) {
            matriceNplus1[i][j].alive = 1;
            if (matrice[i][j].age < 255)
                matriceNplus1[i][j].age = matrice[i][j].age + 1;
        } else {
            matriceNplus1[i][j].alive = 0;
            matriceNplus1[i][j].age = 0;
        }
    } else {
        if (compteurDeCellulesAdjacentesVivantes == 3) {
            matriceNplus1[i][j].alive = 1;
            matriceNplus1[i][j].age = 1;
        } else {
            matriceNplus1[i][j].alive = 0;
            matriceNplus1[i][j].age = 0;
        }
    }
}

// Fonction pour réinitialiser la grille
void resetGrid(Cell **grid, int mode, int tauxCellulesVivantes) {
    if (mode == 1) {
        srand(time(0));
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                grid[i][j].alive = (rand() % 100 + 1 <= tauxCellulesVivantes) ? 1 : 0;
                grid[i][j].age = grid[i][j].alive ? 1 : 0;
            }
        }
    } else {
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                grid[i][j].alive = 0;
                grid[i][j].age = 0;
            }
        }
    }
}

int main(int argc, char* argv[]) {
    int mode = 0;
    int tauxCellulesVivantes = 50;
    printf("Choisissez votre mode :\n");
    printf("  1: Simulation Aleatoire\n");
    printf("  2: Mode Editeur (creer vous-meme)\n");
    printf("Votre choix : ");
    if (scanf("%d", &mode) != 1) {
        mode = 1;
        while (getchar() != '\n');
    }

    if (mode == 1) {
        printf("Quelle est le taux de cellule vivante a l'initialisation ? (Default : 50%%)\n");
        if (scanf("%i", &tauxCellulesVivantes) != 1) {
            while (getchar() != '\n');
        }
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        return 1;
    }
    SDL_DisplayMode displayMode;
    SDL_GetCurrentDisplayMode(0, &displayMode);
    int screenWidth = displayMode.w;
    int screenHeight = displayMode.h;

    float ratio = (float)screenWidth / (float)screenHeight;
    int maxWidthCells = screenWidth / 8;
    int maxHeightCells = screenHeight / 8;
    N = (maxWidthCells < maxHeightCells) ? maxWidthCells : maxHeightCells;
    if (N > 200) N = 200;
    CELL_SIZE = (screenWidth < screenHeight) ? (screenWidth / N) : (screenHeight / N);

    int gridWidth = N * CELL_SIZE;
    int gridHeight = N * CELL_SIZE;
    int offset_x = (screenWidth - gridWidth) / 2;
    int offset_y = (screenHeight - gridHeight) / 2;

    Cell **logicGrid = malloc(N * sizeof(Cell*));
    for (int i = 0; i < N; i++) {
        logicGrid[i] = malloc(N * sizeof(Cell));
    }

    // Initialisation de la grille
    resetGrid(logicGrid, mode, tauxCellulesVivantes);

    SDL_Window* window = SDL_CreateWindow(
        "Jeu de la Vie",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        screenWidth,
        screenHeight,
        SDL_WINDOW_FULLSCREEN_DESKTOP
    );

    if (window == NULL) {
        for (int i = 0; i < N; i++) free(logicGrid[i]);
        free(logicGrid);
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        SDL_DestroyWindow(window);
        for (int i = 0; i < N; i++) free(logicGrid[i]);
        free(logicGrid);
        SDL_Quit();
        return 1;
    }

    int generation = 0;
    int running = 1;
    int isPaused = 1;
    int mousePressed = 0;
    SDL_Event event;
    Uint32 lastGenerationTime = SDL_GetTicks();

    drawGrid(renderer, logicGrid, CELL_SIZE, offset_x, offset_y);

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_q:
                        running = 0;
                        break;
                    case SDLK_SPACE:
                        isPaused = !isPaused;
                        if (!isPaused && generation == 0) {
                            generation = 1;
                            lastGenerationTime = SDL_GetTicks();
                        }
                        break;
                    case SDLK_f:
                        Uint32 flags = SDL_GetWindowFlags(window);
                        if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) {
                            SDL_SetWindowFullscreen(window, 0);
                        } else {
                            SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                        }
                        break;
                    case SDLK_r:  // Réinitialiser la grille
                        resetGrid(logicGrid, mode, tauxCellulesVivantes);
                        generation = 0;
                        isPaused = 1;
                        drawGrid(renderer, logicGrid, CELL_SIZE, offset_x, offset_y);
                        break;
                }
            }
            if (isPaused) {
                if (event.type == SDL_MOUSEBUTTONDOWN) {
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        mousePressed = 1;
                        int j = (event.button.x - offset_x) / CELL_SIZE;
                        int i = (event.button.y - offset_y) / CELL_SIZE;
                        if (i >= 0 && i < N && j >= 0 && j < N) {
                            logicGrid[i][j].alive = !logicGrid[i][j].alive;
                            logicGrid[i][j].age = logicGrid[i][j].alive ? 1 : 0;
                        }
                    }
                }
                if (event.type == SDL_MOUSEBUTTONUP) {
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        mousePressed = 0;
                    }
                }
                if (event.type == SDL_MOUSEMOTION) {
                    if (mousePressed) {
                        int j = (event.motion.x - offset_x) / CELL_SIZE;
                        int i = (event.motion.y - offset_y) / CELL_SIZE;
                        if (i >= 0 && i < N && j >= 0 && j < N) {
                            logicGrid[i][j].alive = 1;
                            logicGrid[i][j].age = 1;
                        }
                    }
                }
            }
        }

        char title[100];
        if (isPaused) {
            if (generation == 0) {
                snprintf(title, 100, "Mode Editeur - Dessinez puis Espace (R pour réinitialiser)");
            } else {
                snprintf(title, 100, "Gen %d (PAUSE - R pour réinitialiser)", generation);
            }
        } else {
            snprintf(title, 100, "Gen %d (R pour réinitialiser)", generation);
        }
        SDL_SetWindowTitle(window, title);

        Uint32 currentFrameTime = SDL_GetTicks();
        if (!isPaused) {
            if (currentFrameTime - lastGenerationTime > GENERATION_DELAY_MS) {
                Cell **logicTemp = malloc(N * sizeof(Cell*));
                for (int i = 0; i < N; i++) {
                    logicTemp[i] = malloc(N * sizeof(Cell));
                }
                for (int i = 0; i < N; i++) {
                    for (int j = 0; j < N; j++) {
                        logicTemp[i][j] = logicGrid[i][j];
                        nextGeneration(logicGrid, logicTemp, N, i, j);
                    }
                }
                for (int i = 0; i < N; i++) {
                    for (int j = 0; j < N; j++) {
                        logicGrid[i][j] = logicTemp[i][j];
                    }
                }
                for (int i = 0; i < N; i++) free(logicTemp[i]);
                free(logicTemp);
                generation++;
                lastGenerationTime = currentFrameTime;
            }
            drawGrid(renderer, logicGrid, CELL_SIZE, offset_x, offset_y);
        } else {
            drawGrid(renderer, logicGrid, CELL_SIZE, offset_x, offset_y);
            SDL_Delay(50);
        }
    }

    for (int i = 0; i < N; i++) free(logicGrid[i]);
    free(logicGrid);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
