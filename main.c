#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL.h>
#include <math.h>

#define N 40
#define INIT_WIDTH 800
#define INIT_HEIGHT 800

/**
 * @brief Projette un point 3D sur un écran 2D
 */
void drawCube(SDL_Renderer *renderer, int grid[N][N][N], float angleX, float angleY,
              float currentScale, float camX, float camY, int winW, int winH) {

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    float cx = winW / 2.0f;
    float cy = winH / 2.0f;
    float offset = N / 2.0f;

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            for (int k = 0; k < N; k++) {

                int age = grid[i][j][k];
                if (age > 0) {
                    // --- 1. CALCULS 3D ---
                    float x = (i - offset) * currentScale;
                    float y = (j - offset) * currentScale;
                    float z = (k - offset) * currentScale;

                    float x1 = x * cosf(angleY) - z * sinf(angleY);
                    float z1 = x * sinf(angleY) + z * cosf(angleY);

                    float y2 = y * cosf(angleX) - z1 * sinf(angleX);
                    float z2 = y * sinf(angleX) + z1 * cosf(angleX);
                    float x2 = x1;

                    float dist = 400.0f;
                    if (dist - z2 < 1.0f) continue;

                    float z_factor = dist / (dist - z2);

                    int screenX = (int)(cx + x2 * z_factor + camX);
                    int screenY = (int)(cy + y2 * z_factor + camY);

                    // --- 2. COULEUR (Age + Profondeur) ---
                    int green_blue_val = 255 - (age * 5);
                    if (green_blue_val < 0) green_blue_val = 0;

                    float depthShade = 1.0f - (z2 + N*currentScale/2) / (N*currentScale*2);
                    if (depthShade < 0.2f) depthShade = 0.2f;
                    if (depthShade > 1.0f) depthShade = 1.0f;

                    Uint8 r = (Uint8)(255 * depthShade);
                    Uint8 g = (Uint8)(green_blue_val * depthShade);
                    Uint8 b = (Uint8)(green_blue_val * depthShade);

                    SDL_SetRenderDrawColor(renderer, r, g, b, 255);

                    int size = (int)(currentScale * 0.8f * z_factor);
                    if (size < 1) size = 1;

                    SDL_Rect voxel = { screenX, screenY, size, size };
                    SDL_RenderFillRect(renderer, &voxel);
                }
            }
        }
    }
    SDL_RenderPresent(renderer);
}

void nextGeneration3D(int grid[N][N][N], int nextGrid[N][N][N], int x, int y, int z) {
    int neighbors = 0;
    int currentAge = grid[x][y][z];

    for (int i = x - 1; i <= x + 1; i++) {
        if (i < 0 || i >= N) continue;
        for (int j = y - 1; j <= y + 1; j++) {
            if (j < 0 || j >= N) continue;
            for (int k = z - 1; k <= z + 1; k++) {
                if (k < 0 || k >= N) continue;
                if (i == x && j == y && k == z) continue;
                if (grid[i][j][k] > 0) neighbors++;
            }
        }
    }

    if (currentAge > 0) {
        if (neighbors >= 4 && neighbors <= 5) nextGrid[x][y][z] = currentAge + 1;
        else nextGrid[x][y][z] = 0;
    } else {
        if (neighbors == 5) nextGrid[x][y][z] = 1;
        else nextGrid[x][y][z] = 0;
    }
}

int main(int argc, char* argv[]) {

    int mode = 0;
    int taux = 20;

    printf("--- JEU DE LA VIE 3D (Vitesse Variable) ---\n");
    printf("CONTROLES :\n");
    printf("  [Espace]  : Pause/Play\n");
    printf("  [E]       : ACCELERER (Reduit le delai)\n");
    printf("  [A]       : RALENTIR (Augmente le delai)\n");
    printf("  [F11]     : Plein Ecran\n");
    printf("  [Fleches] : Rotation\n");
    printf("  [I/K/J/L] : Deplacement\n");
    printf("  [+ / -]   : Zoom\n\n");

    printf("Choix (1: Aleatoire, 2: Glider): ");
    if (scanf("%d", &mode) != 1) mode = 1;
    while(getchar() != '\n');

    static int grid[N][N][N];
    static int nextGrid[N][N][N];

    if (mode == 1) {
        printf("Taux (%%): ");
        scanf("%d", &taux);
        srand(time(0));
        for(int i=0; i<N; i++)
            for(int j=0; j<N; j++)
                for(int k=0; k<N; k++)
                    grid[i][j][k] = (rand() % 100 < taux) ? 1 : 0;
    } else {
        for(int i=0; i<N; i++)
            for(int j=0; j<N; j++)
                for(int k=0; k<N; k++)
                    grid[i][j][k] = 0;
        int c = N/2;
        grid[c][c][c]=1; grid[c+1][c][c]=1; grid[c][c+1][c]=1; grid[c][c][c+1]=1; grid[c-1][c][c]=1;
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0) return 1;

    SDL_Window* window = SDL_CreateWindow("Jeu 3D - Vitesse A/E",
                                        SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED,
                                        INIT_WIDTH, INIT_HEIGHT,
                                        SDL_WINDOW_RESIZABLE);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    int winWidth = INIT_WIDTH;
    int winHeight = INIT_HEIGHT;

    int running = 1;
    int paused = 1;
    int generation = 0;
    int isFullscreen = 0;

    int generationDelay = 50; // Délai initial

    float angleX = 0.0f;
    float angleY = 0.0f;
    float currentScale = 10.0f;
    float camX = 0.0f;
    float camY = 0.0f;

    Uint32 lastGenTime = SDL_GetTicks();

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;

            if (event.type == SDL_WINDOWEVENT) {
                if (event.window.event == SDL_WINDOWEVENT_RESIZED ||
                    event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    winWidth = event.window.data1;
                    winHeight = event.window.data2;
                }
            }

            if (event.type == SDL_KEYDOWN) {
                switch(event.key.keysym.sym) {
                    case SDLK_q: running = 0; break;
                    case SDLK_SPACE: paused = !paused; break;

                    case SDLK_F11:
                    case SDLK_f:
                        isFullscreen = !isFullscreen;
                        SDL_SetWindowFullscreen(window, isFullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
                        SDL_GetWindowSize(window, &winWidth, &winHeight);
                        break;

                    // --- GESTION DE LA VITESSE (E / A) ---
                    case SDLK_e: // Plus vite (E)
                        generationDelay -= 10;
                        if (generationDelay < 1) generationDelay = 1;
                        break;
                    case SDLK_a: // Moins vite (A)
                        generationDelay += 10;
                        break;

                    // Caméra & Zoom
                    case SDLK_LEFT: angleY -= 0.1f; break;
                    case SDLK_RIGHT: angleY += 0.1f; break;
                    case SDLK_UP: angleX -= 0.1f; break;
                    case SDLK_DOWN: angleX += 0.1f; break;
                    case SDLK_r:
                        angleX = 0; angleY = 0;
                        currentScale = 10.0f;
                        camX = 0; camY = 0;
                        generationDelay = 50;
                        break;

                    case SDLK_l: camX -= 20.0f; break;
                    case SDLK_j: camX += 20.0f; break;
                    case SDLK_i: camY += 20.0f; break;
                    case SDLK_k: camY -= 20.0f; break;

                    case SDLK_KP_PLUS:
                    case SDLK_PLUS:
                        currentScale += 1.0f;
                        break;
                    case SDLK_KP_MINUS:
                    case SDLK_MINUS:
                        currentScale -= 1.0f;
                        if (currentScale < 1.0f) currentScale = 1.0f;
                        break;
                }
            }
        }

        if (!paused && SDL_GetTicks() - lastGenTime > generationDelay) {
            for(int i=0; i<N; i++)
                for(int j=0; j<N; j++)
                    for(int k=0; k<N; k++)
                        nextGeneration3D(grid, nextGrid, i, j, k);

            for(int i=0; i<N; i++)
                for(int j=0; j<N; j++)
                    for(int k=0; k<N; k++)
                        grid[i][j][k] = nextGrid[i][j][k];

            generation++;
            lastGenTime = SDL_GetTicks();
        }

        drawCube(renderer, grid, angleX, angleY, currentScale, camX, camY, winWidth, winHeight);

        char title[100];
        snprintf(title, 100, "Gen: %d | Delai: %dms | Zoom: %.1f", generation, generationDelay, currentScale);
        SDL_SetWindowTitle(window, title);

        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}