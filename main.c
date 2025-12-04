#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL.h>

#define N 150
#define CELL_SIZE 5
#define WIDTH (N * CELL_SIZE)
#define HEIGHT (N * CELL_SIZE)
#define GENERATION_DELAY_MS 100 // Délai en ms entre les générations

// Fonction d'affichage simplifiée : Une seule couleur, pas de dégradé
void drawGrid(SDL_Renderer *renderer, int grid[N][N]) {

    // 1. On dessine le fond en noir
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // 2. On définit la couleur des cellules (VERT)
    // Vous pouvez changer les valeurs ici (R, G, B)
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);

    // 3. On dessine les cellules vivantes
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            // Si la cellule est vivante (valeur 1)
            if (grid[i][j] == 1) {
                SDL_Rect cell = { j * CELL_SIZE, i * CELL_SIZE, CELL_SIZE, CELL_SIZE };
                SDL_RenderFillRect(renderer, &cell);
            }
        }
    }

    // 4. On affiche le tout à l'écran
    SDL_RenderPresent(renderer);
}

// Définie dans une matrice temporaire l'état de la case [i][j] en fonction de la génération actuelle
void nextGeneration(int matrice[N][N], int matriceNplus1[N][N], int i, int j) {
    int compteurDeCellulesAdjacentesVivantes = 0;
    int currentState = matrice[i][j];

    //Compte le nombre de cellules adjacentes vivantes
    for (int compteuri = i-1; compteuri<= i+1; compteuri++) {
        if (compteuri>=0 && compteuri < N) {
            for (int compteurj = j-1; compteurj<= j+1; compteurj++) {
                if (compteurj>=0 && compteurj < N) {
                    if (matrice[compteuri][compteurj] == 1) {
                        if (i != compteuri || j != compteurj) {
                            compteurDeCellulesAdjacentesVivantes++;
                        }
                    }
                }
            }
        }
    }

    if (currentState == 0) {
        if(compteurDeCellulesAdjacentesVivantes == 3) {
            matriceNplus1[i][j] = 1;
        }
    }else {
        if (compteurDeCellulesAdjacentesVivantes < 2 || compteurDeCellulesAdjacentesVivantes > 3) {
            matriceNplus1[i][j] = 0;
        }
    }
}

int main(int argc, char* argv[]) {

    int mode = 0;
    int tauxCellulesVivantes = 50;

    // --- Menu de sélection ---
    printf("Choisissez votre mode :\n");
    printf("  1: Simulation Aleatoire\n");
    printf("  2: Mode Editeur (creer vous-meme)\n");
    printf("Votre choix : ");
    if (scanf("%d", &mode) != 1) {
        mode = 1;
        while (getchar() != '\n');
    }

    // --- Une seule grille nécessaire maintenant (la logique) ---
    int logicGrid[N][N];

    if (mode == 1) {
        // --- MODE 1 : ALÉATOIRE ---
        printf("Quelle est le taux de cellule vivante a l'initialisation ? (Default : 50%%)\n");
        if (scanf("%i", &tauxCellulesVivantes) != 1) {
            while (getchar() != '\n');
        }
        srand(time(0));
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                logicGrid[i][j] = (rand() % 100 + 1 <= tauxCellulesVivantes) ? 1 : 0;
            }
        }
    } else {
        // --- MODE 2 : EDITEUR ---
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                logicGrid[i][j] = 0;
            }
        }
    }

    // --- Initialisation de SDL ---
    if (SDL_Init(SDL_INIT_VIDEO) != 0) { return 1; }
    SDL_Window* window = SDL_CreateWindow("Jeu de la Vie", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    if (window == NULL) { SDL_Quit(); return 1; }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) { SDL_DestroyWindow(window); SDL_Quit(); return 1; }
    // --- Fin Initialisation SDL ---


    int generation = 0;
    int running = 1;
    int isPaused = 1;
    int mousePressed = 0;
    SDL_Event event;

    // Timer pour la logique de jeu
    Uint32 lastGenerationTime = SDL_GetTicks();

    // --- Affichage initial ---
    drawGrid(renderer, logicGrid);

    // Boucle principale
    while (running) {

        // --- 1. Gestion des événements ---
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

            // Gestion souris (Mode Editeur)
            if (isPaused) {
                if (event.type == SDL_MOUSEBUTTONDOWN) {
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        mousePressed = 1;
                        int j = event.button.x / CELL_SIZE;
                        int i = event.button.y / CELL_SIZE;
                        if (i >= 0 && i < N && j >= 0 && j < N) {
                            logicGrid[i][j] = !logicGrid[i][j]; // Inverse état
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
                            logicGrid[i][j] = 1; // Force à vivant
                        }
                    }
                }
            }
        } // Fin events


        // --- 2. Mise à jour du titre ---
        char title[100];
        if (isPaused) {
            if (generation == 0) snprintf(title, 100, "Mode Editeur - Dessinez puis Espace");
            else snprintf(title, 100, "Gen %d (PAUSE)", generation);
        } else {
            snprintf(title, 100, "Gen %d", generation);
        }
        SDL_SetWindowTitle(window, title);


        // --- 3. Logique du jeu ---
        Uint32 currentFrameTime = SDL_GetTicks();

        if (!isPaused) {
            // Si le temps écoulé dépasse le délai, on calcule la nouvelle génération
            if (currentFrameTime - lastGenerationTime > GENERATION_DELAY_MS) {
                int logicTemp[N][N];
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

            // On dessine l'état actuel
            drawGrid(renderer, logicGrid);

        } else {
            // En pause : on redessine juste pour voir les modifications souris
            drawGrid(renderer, logicGrid);
            SDL_Delay(50); // Petite pause CPU
        }

    } // Fin boucle while

    // --- Fin et Nettoyage ---
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}