#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL.h>

#define N 150
#define CELL_SIZE 5
#define WIDTH (N * CELL_SIZE)
#define HEIGHT (N * CELL_SIZE)
#define GENERATION_DELAY_MS 100

typedef struct {
    int alive;
    int age;
} Cell;

void drawGrid(SDL_Renderer *renderer, Cell grid[N][N]) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (grid[i][j].alive) {
                Uint8 age = grid[i][j].age;
                Uint8 r = 255;  // Rouge max
                Uint8 g = 255 - (age * 5 > 255 ? 255 : age * 5);  // Vert diminue
                Uint8 b = 255;  // Bleu max
                SDL_SetRenderDrawColor(renderer, r, g, b, 255);
                SDL_Rect cell = { j * CELL_SIZE, i * CELL_SIZE, CELL_SIZE, CELL_SIZE };
                SDL_RenderFillRect(renderer, &cell);
            }
        }
    }
    SDL_RenderPresent(renderer);
}

void nextGeneration(Cell matrice[N][N], Cell matriceNplus1[N][N], int i, int j) {
    int compteurDeCellulesAdjacentesVivantes = 0;
    for (int compteuri = i-1; compteuri <= i+1; compteuri++) {
        if (compteuri >= 0 && compteuri < N) {
            for (int compteurj = j-1; compteurj <= j+1; compteurj++) {
                if (compteurj >= 0 && compteurj < N) {
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

    Cell logicGrid[N][N];
    if (mode == 1) {
        printf("Quelle est le taux de cellule vivante a l'initialisation ? (Default : 50%%)\n");
        if (scanf("%i", &tauxCellulesVivantes) != 1) {
            while (getchar() != '\n');
        }
        srand(time(0));
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                logicGrid[i][j].alive = (rand() % 100 + 1 <= tauxCellulesVivantes) ? 1 : 0;
                logicGrid[i][j].age = logicGrid[i][j].alive ? 1 : 0;
            }
        }
    } else {
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                logicGrid[i][j].alive = 0;
                logicGrid[i][j].age = 0;
            }
        }
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0) { return 1; }
    SDL_Window* window = SDL_CreateWindow("Jeu de la Vie", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    if (window == NULL) { SDL_Quit(); return 1; }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) { SDL_DestroyWindow(window); SDL_Quit(); return 1; }

    int generation = 0;
    int running = 1;
    int isPaused = 1;
    int mousePressed = 0;
    SDL_Event event;
    Uint32 lastGenerationTime = SDL_GetTicks();
    drawGrid(renderer, logicGrid);

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_q: running = 0; break;
                    case SDLK_SPACE:
                        isPaused = !isPaused;
                        if (!isPaused && generation == 0) {
                            generation = 1;
                            lastGenerationTime = SDL_GetTicks();
                        }
                        break;
                }
            }
            if (isPaused) {
                if (event.type == SDL_MOUSEBUTTONDOWN) {
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        mousePressed = 1;
                        int j = event.button.x / CELL_SIZE;
                        int i = event.button.y / CELL_SIZE;
                        if (i >= 0 && i < N && j >= 0 && j < N) {
                            logicGrid[i][j].alive = !logicGrid[i][j].alive;
                            logicGrid[i][j].age = logicGrid[i][j].alive ? 1 : 0;
                        }
                    }
                }
                if (event.type == SDL_MOUSEBUTTONUP) {
                    if (event.button.button == SDL_BUTTON_LEFT) mousePressed = 0;
                }
                if (event.type == SDL_MOUSEMOTION) {
                    if (mousePressed) {
                        int j = event.motion.x / CELL_SIZE;
                        int i = event.motion.y / CELL_SIZE;
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
            if (generation == 0) snprintf(title, 100, "Mode Editeur - Dessinez puis Espace");
            else snprintf(title, 100, "Gen %d (PAUSE)", generation);
        } else {
            snprintf(title, 100, "Gen %d", generation);
        }
        SDL_SetWindowTitle(window, title);

        Uint32 currentFrameTime = SDL_GetTicks();
        if (!isPaused) {
            if (currentFrameTime - lastGenerationTime > GENERATION_DELAY_MS) {
                Cell logicTemp[N][N];
                for (int i = 0; i < N; i++) {
                    for (int j = 0; j < N; j++) {
                        logicTemp[i][j] = logicGrid[i][j];
                        nextGeneration(logicGrid, logicTemp, i, j);
                    }
                }
                for (int i = 0; i < N; i++) {
                    for (int j = 0; j < N; j++) {
                        logicGrid[i][j] = logicTemp[i][j];
                    }
                }
                generation++;
                lastGenerationTime = currentFrameTime;
            }
            drawGrid(renderer, logicGrid);
        } else {
            drawGrid(renderer, logicGrid);
            SDL_Delay(50);
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
