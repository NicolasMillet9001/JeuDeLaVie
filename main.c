#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL.h>
#include <math.h>

// On réduit N car N*N*N ça monte très vite en mémoire et CPU !
#define N 40
#define WIDTH 800
#define HEIGHT 800
#define GENERATION_DELAY_MS 50

// Facteur de zoom pour l'affichage
#define SCALE 10.0f

/**
 * @brief Projette un point 3D (x,y,z) sur un écran 2D avec rotation
 */
void drawCube(SDL_Renderer *renderer, int grid[N][N][N], float angleX, float angleY) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    float cx = WIDTH / 2.0f;
    float cy = HEIGHT / 2.0f;

    // Pour centrer le cube autour de 0,0,0 avant rotation
    float offset = N / 2.0f;

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            for (int k = 0; k < N; k++) {

                if (grid[i][j][k]) {
                    // 1. Coordonnées centrées
                    float x = (i - offset) * SCALE;
                    float y = (j - offset) * SCALE;
                    float z = (k - offset) * SCALE;

                    // 2. Rotation autour de Y (Horizontal)
                    float x1 = x * cosf(angleY) - z * sinf(angleY);
                    float z1 = x * sinf(angleY) + z * cosf(angleY);

                    // 3. Rotation autour de X (Vertical)
                    float y2 = y * cosf(angleX) - z1 * sinf(angleX);
                    float z2 = y * sinf(angleX) + z1 * cosf(angleX);
                    float x2 = x1;

                    // 4. Projection simple (Perspective faible)
                    // On ajoute une profondeur pour l'effet 3D
                    float dist = 400.0f; // Distance de la "caméra"
                    float z_factor = dist / (dist - z2);

                    int screenX = (int)(cx + x2 * z_factor);
                    int screenY = (int)(cy + y2 * z_factor);

                    // 5. Couleur basée sur la profondeur (z2)
                    // Plus c'est loin (z2 petit), plus c'est sombre
                    int brightness = (int)(255 * (1.0f - (z2 + N*SCALE/2) / (N*SCALE*2)));
                    if (brightness < 50) brightness = 50;
                    if (brightness > 255) brightness = 255;

                    // Vert avec variation de luminosité
                    SDL_SetRenderDrawColor(renderer, 0, brightness, 0, 255);

                    // On dessine un petit rectangle pour simuler le voxel
                    // La taille dépend aussi de la distance (perspective)
                    int size = (int)(SCALE * 0.8f * z_factor);
                    if (size < 1) size = 1;

                    SDL_Rect voxel = { screenX, screenY, size, size };
                    SDL_RenderFillRect(renderer, &voxel);
                }
            }
        }
    }
    SDL_RenderPresent(renderer);
}

// Logique 3D
void nextGeneration3D(int grid[N][N][N], int nextGrid[N][N][N], int x, int y, int z) {
    int neighbors = 0;
    int currentState = grid[x][y][z];

    // Vérification du cube 3x3x3
    for (int i = x - 1; i <= x + 1; i++) {
        if (i < 0 || i >= N) continue; // Hors limites X

        for (int j = y - 1; j <= y + 1; j++) {
            if (j < 0 || j >= N) continue; // Hors limites Y

            for (int k = z - 1; k <= z + 1; k++) {
                if (k < 0 || k >= N) continue; // Hors limites Z

                // On ne se compte pas soi-même
                if (i == x && j == y && k == z) continue;

                if (grid[i][j][k] == 1) {
                    neighbors++;
                }
            }
        }
    }

    // --- RÈGLES DU JEU DE LA VIE 3D (Règle 5766 ou 4555) ---
    // En 3D, les voisins max sont 26. Les règles 2D (3, 2-3) tuent tout instantanément.
    // Voici la règle "Bays" standard (Survivre 4-5, Naître 5)

    if (currentState == 1) {
        // Règle de SURVIE : Reste vivant si 4 ou 5 voisins
        if (neighbors >= 4 && neighbors <= 5) {
            nextGrid[x][y][z] = 1;
        } else {
            nextGrid[x][y][z] = 0; // Meurt de solitude ou surpopulation
        }
    } else {
        // Règle de NAISSANCE : Nait si exactement 5 voisins
        if (neighbors == 5) {
            nextGrid[x][y][z] = 1;
        } else {
            nextGrid[x][y][z] = 0;
        }
    }
}

int main(int argc, char* argv[]) {

    // --- Menu ---
    int mode = 0;
    int taux = 20; // Taux par défaut plus bas en 3D sinon c'est illisible

    printf("--- JEU DE LA VIE 3D ---\n");
    printf("Regles utilisees : Survie 4-5, Naissance 5 (Standard 3D)\n\n");
    printf("Choisissez votre mode :\n");
    printf("  1: Cube Aleatoire\n");
    printf("  2: Mode Glider 3D (Forme simple)\n");
    printf("Votre choix : ");
    if (scanf("%d", &mode) != 1) mode = 1;
    while(getchar() != '\n');

    // Allocation des grilles 3D
    // Utilisation de static pour éviter le stack overflow si N est grand
    static int grid[N][N][N];
    static int nextGrid[N][N][N];

    // Initialisation
    if (mode == 1) {
        printf("Taux de remplissage (recommande 10-30%%) : ");
        scanf("%d", &taux);
        srand(time(0));
        for(int i=0; i<N; i++)
            for(int j=0; j<N; j++)
                for(int k=0; k<N; k++)
                    grid[i][j][k] = (rand() % 100 < taux) ? 1 : 0;
    } else {
        // Tout vide
        for(int i=0; i<N; i++)
            for(int j=0; j<N; j++)
                for(int k=0; k<N; k++)
                    grid[i][j][k] = 0;

        // Un petit bloc au centre pour tester
        int c = N/2;
        grid[c][c][c] = 1;
        grid[c+1][c][c] = 1;
        grid[c][c+1][c] = 1;
        grid[c][c][c+1] = 1;
        grid[c-1][c][c] = 1;
    }

    // --- SDL Init ---
    if (SDL_Init(SDL_INIT_VIDEO) != 0) return 1;
    SDL_Window* window = SDL_CreateWindow("Jeu de la Vie 3D", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    int running = 1;
    int paused = 1; // Commence en pause pour laisser le temps de voir
    int generation = 0;

    // Angles pour la rotation de la caméra
    float angleX = 0.0f;
    float angleY = 0.0f;

    Uint32 lastGenTime = SDL_GetTicks();

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;
            if (event.type == SDL_KEYDOWN) {
                switch(event.key.keysym.sym) {
                    case SDLK_q: running = 0; break;
                    case SDLK_SPACE: paused = !paused; break;
                    // Rotation de la caméra
                    case SDLK_LEFT: angleY -= 0.1f; break;
                    case SDLK_RIGHT: angleY += 0.1f; break;
                    case SDLK_UP: angleX -= 0.1f; break;
                    case SDLK_DOWN: angleX += 0.1f; break;
                    // Reset caméra
                    case SDLK_r: angleX = 0; angleY = 0; break;
                }
            }
        }

        // --- Logique (si pas en pause) ---
        if (!paused && SDL_GetTicks() - lastGenTime > GENERATION_DELAY_MS) {
            for(int i=0; i<N; i++) {
                for(int j=0; j<N; j++) {
                    for(int k=0; k<N; k++) {
                        nextGeneration3D(grid, nextGrid, i, j, k);
                    }
                }
            }
            // Copie
            for(int i=0; i<N; i++)
                for(int j=0; j<N; j++)
                    for(int k=0; k<N; k++)
                        grid[i][j][k] = nextGrid[i][j][k];

            generation++;
            lastGenTime = SDL_GetTicks();
        }

        // --- Affichage ---
        // On affiche toujours, même en pause, pour permettre la rotation fluide
        drawCube(renderer, grid, angleX, angleY);

        // Titre
        char title[100];
        snprintf(title, 100, "3D Gen: %d | %s | Fleches pour tourner", generation, paused ? "PAUSE" : "RUN");
        SDL_SetWindowTitle(window, title);

        SDL_Delay(16); // ~60 FPS pour l'affichage (indépendant de la vitesse de simulation)
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}