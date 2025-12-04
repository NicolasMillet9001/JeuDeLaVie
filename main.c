#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL.h>
#include <math.h>

#define N 40
#define INIT_WIDTH 800
#define INIT_HEIGHT 800

// ==========================================
// OUTILS DESSIN & TEXTE (PIXEL ART)
// ==========================================

void drawChar(SDL_Renderer* renderer, int x, int y, int scale, char c) {
    int patterns[][15] = {
        {1,1,1, 1,0,1, 1,0,1, 1,0,1, 1,1,1}, // 0
        {0,1,0, 0,1,0, 0,1,0, 0,1,0, 0,1,0}, // 1
        {1,1,1, 0,0,1, 1,1,1, 1,0,0, 1,1,1}, // 2
        {1,1,1, 0,0,1, 1,1,1, 0,0,1, 1,1,1}, // 3
        {1,0,1, 1,0,1, 1,1,1, 0,0,1, 0,0,1}, // 4
        {1,1,1, 1,0,0, 1,1,1, 0,0,1, 1,1,1}, // 5
        {1,1,1, 1,0,0, 1,1,1, 1,0,1, 1,1,1}, // 6
        {1,1,1, 0,0,1, 0,0,1, 0,0,1, 0,0,1}, // 7
        {1,1,1, 1,0,1, 1,1,1, 1,0,1, 1,1,1}, // 8
        {1,1,1, 1,0,1, 1,1,1, 0,0,1, 1,1,1}, // 9
        {1,1,1, 1,0,0, 1,1,1, 0,0,1, 1,1,1}, // S (Start)
        {1,0,1, 1,1,1, 1,0,1, 1,0,1, 1,0,1}, // M (Mode)
        {1,1,1, 0,1,0, 0,1,0, 0,1,0, 0,1,0}, // T (Taux)
        {1,1,1, 1,0,1, 1,1,0, 1,0,1, 1,0,1}, // R (Random)
        {1,1,1, 1,0,0, 1,0,1, 1,0,1, 1,1,1}, // G (Glider)
    };

    int index = -1;
    if (c >= '0' && c <= '9') index = c - '0';
    if (c == 'S') index = 10;
    if (c == 'M') index = 11;
    if (c == 'T') index = 12;
    if (c == 'R') index = 13;
    if (c == 'G') index = 14;

    if (index >= 0) {
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 3; j++) {
                if (patterns[index][i * 3 + j]) {
                    SDL_Rect pixel = {x + j * scale, y + i * scale, scale, scale};
                    SDL_RenderFillRect(renderer, &pixel);
                }
            }
        }
    }
}

void drawNumber(SDL_Renderer* renderer, int x, int y, int scale, int number) {
    char buffer[10];
    sprintf(buffer, "%d", number);
    for (int i = 0; buffer[i] != '\0'; i++) {
        drawChar(renderer, x + (i * 4 * scale), y, scale, buffer[i]);
    }
}

// ==========================================
// FONCTIONS JEU
// ==========================================

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
                // On affiche QUE les cellules vivantes
                if (age > 0) {
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
                    int size = (int)(currentScale * 0.8f * z_factor);
                    if (size < 1) size = 1;

                    // CELLULE VIVANTE (Blanc -> Rouge)
                    int green_blue_val = 255 - (age * 5);
                    if (green_blue_val < 0) green_blue_val = 0;

                    float depthShade = 1.0f - (z2 + N*currentScale/2) / (N*currentScale*2);
                    if (depthShade < 0.2f) depthShade = 0.2f;
                    if (depthShade > 1.0f) depthShade = 1.0f;

                    Uint8 r = (Uint8)(255 * depthShade);
                    Uint8 g = (Uint8)(green_blue_val * depthShade);
                    Uint8 b = (Uint8)(green_blue_val * depthShade);

                    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
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

// ==========================================
// FONCTIONS DU MENU GRAPHIQUE
// ==========================================
void drawMenu(SDL_Renderer* renderer, int winW, int winH, int selectedMode, int taux) {
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);

    // --- BOUTON MODE 1 (RANDOM) ---
    SDL_Rect btn1 = {winW/4 - 100, winH/3, 200, 150};
    if (selectedMode == 1) SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    else SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_RenderDrawRect(renderer, &btn1);

    for(int i=0; i<50; i++) {
        SDL_RenderDrawPoint(renderer, btn1.x + rand()%200, btn1.y + rand()%150);
    }
    drawChar(renderer, btn1.x + 80, btn1.y + 160, 5, 'R');


    // --- BOUTON MODE 2 (GLIDER) ---
    SDL_Rect btn2 = {3*winW/4 - 100, winH/3, 200, 150};
    if (selectedMode == 2) SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    else SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_RenderDrawRect(renderer, &btn2);

    // Icone Glider
    SDL_Rect gPiece = {btn2.x + 80, btn2.y + 60, 40, 40};
    SDL_RenderFillRect(renderer, &gPiece);
    // Texte "G"
    drawChar(renderer, btn2.x + 80, btn2.y + 160, 5, 'G');


    // --- SLIDER TAUX (Si mode Random) ---
    if (selectedMode == 1) {
        SDL_Rect barBg = {winW/4, 2*winH/3, winW/2, 20};
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
        SDL_RenderFillRect(renderer, &barBg);

        SDL_Rect barFill = {winW/4, 2*winH/3, (winW/2) * taux / 100, 20};
        SDL_SetRenderDrawColor(renderer, 200, 50, 50, 255);
        SDL_RenderFillRect(renderer, &barFill);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        drawChar(renderer, winW/4 - 40, 2*winH/3 - 5, 5, 'T');
        drawNumber(renderer, 3*winW/4 + 20, 2*winH/3 - 5, 5, taux);
    }

    // --- BOUTON START ---
    SDL_Rect btnStart = {winW/2 - 100, 4*winH/5, 200, 60};
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &btnStart);
    drawChar(renderer, btnStart.x + 90, btnStart.y + 15, 6, 'S');

    SDL_RenderPresent(renderer);
}


int main(int argc, char* argv[]) {

    if (SDL_Init(SDL_INIT_VIDEO) != 0) return 1;

    SDL_Window* window = SDL_CreateWindow("Jeu 3D - Menu Graphique",
                                        SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED,
                                        INIT_WIDTH, INIT_HEIGHT,
                                        SDL_WINDOW_RESIZABLE | SDL_WINDOW_FULLSCREEN_DESKTOP);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    int isFullscreen = 1;
    int programRunning = 1;

    static int grid[N][N][N];
    static int nextGrid[N][N][N];

    // --- BOUCLE GLOBALE ---
    while (programRunning) {

        // --- 1. PHASE MENU ---
        int inMenu = 1;
        int selectedMode = 1;
        int taux = 20;
        int winWidth, winHeight;
        SDL_GetWindowSize(window, &winWidth, &winHeight);

        while (inMenu && programRunning) {
            SDL_Event e;
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) programRunning = 0;

                if (e.type == SDL_WINDOWEVENT && (e.window.event == SDL_WINDOWEVENT_RESIZED)) {
                    winWidth = e.window.data1;
                    winHeight = e.window.data2;
                }

                if (e.type == SDL_KEYDOWN) {
                    if (e.key.keysym.sym == SDLK_ESCAPE) programRunning = 0;
                    if (e.key.keysym.sym == SDLK_LEFT) selectedMode = 1;
                    if (e.key.keysym.sym == SDLK_RIGHT) selectedMode = 2;
                    if (e.key.keysym.sym == SDLK_UP && taux < 100) taux += 5;
                    if (e.key.keysym.sym == SDLK_DOWN && taux > 5) taux -= 5;
                    if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_KP_ENTER) inMenu = 0;
                }

                if (e.type == SDL_MOUSEBUTTONDOWN) {
                    int mx = e.button.x;
                    int my = e.button.y;

                    if (mx > winWidth/4 - 100 && mx < winWidth/4 + 100 && my > winHeight/3 && my < winHeight/3 + 150)
                        selectedMode = 1;
                    if (mx > 3*winWidth/4 - 100 && mx < 3*winWidth/4 + 100 && my > winHeight/3 && my < winHeight/3 + 150)
                        selectedMode = 2;
                    if (mx > winWidth/2 - 100 && mx < winWidth/2 + 100 && my > 4*winHeight/5 && my < 4*winHeight/5 + 60)
                        inMenu = 0;

                    if (selectedMode == 1 && my > 2*winHeight/3 && my < 2*winHeight/3 + 20) {
                        int relX = mx - (winWidth/4);
                        if (relX >= 0 && relX <= winWidth/2) {
                            taux = (relX * 100) / (winWidth/2);
                        }
                    }
                }
            }

            drawMenu(renderer, winWidth, winHeight, selectedMode, taux);
            SDL_Delay(16);
        }

        if (!programRunning) break;

        // --- 2. INITIALISATION JEU ---
        int paused = 1;

        if (selectedMode == 1) {
            // MODE RANDOM
            srand(time(0));
            for(int i=0; i<N; i++) for(int j=0; j<N; j++) for(int k=0; k<N; k++)
                grid[i][j][k] = (rand() % 100 < taux) ? 1 : 0;
        } else {
            // MODE GLIDER
            for(int i=0; i<N; i++) for(int j=0; j<N; j++) for(int k=0; k<N; k++) grid[i][j][k] = 0;
            int c = N/2;
            grid[c][c][c]=1; grid[c+1][c][c]=1; grid[c][c+1][c]=1; grid[c][c][c+1]=1; grid[c-1][c][c]=1;
        }

        int generationDelay = 50;
        float angleX = 0.0f;
        float angleY = 0.0f;
        float currentScale = 10.0f;
        float camX = 0.0f; float camY = 0.0f;
        int generation = 0;

        Uint32 lastGenTime = SDL_GetTicks();
        SDL_GetWindowSize(window, &winWidth, &winHeight);

        // --- 3. BOUCLE JEU ---
        int gameRunning = 1;
        while (gameRunning && programRunning) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) programRunning = 0;

                if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    winWidth = event.window.data1;
                    winHeight = event.window.data2;
                }

                if (event.type == SDL_KEYDOWN) {
                    switch(event.key.keysym.sym) {
                        case SDLK_ESCAPE: programRunning = 0; break;
                        case SDLK_BACKSPACE: gameRunning = 0; break; // Retour Menu
                        case SDLK_SPACE: paused = !paused; break;
                        case SDLK_F11:
                            isFullscreen = !isFullscreen;
                            SDL_SetWindowFullscreen(window, isFullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
                            SDL_GetWindowSize(window, &winWidth, &winHeight);
                            break;
                        case SDLK_e: generationDelay = (generationDelay > 1) ? generationDelay - 10 : 1; break;
                        case SDLK_a: generationDelay += 10; break;
                        case SDLK_LEFT: angleY -= 0.1f; break;
                        case SDLK_RIGHT: angleY += 0.1f; break;
                        case SDLK_UP: angleX -= 0.1f; break;
                        case SDLK_DOWN: angleX += 0.1f; break;
                        case SDLK_r: angleX=0; angleY=0; currentScale=10.0f; camX=0; camY=0; break;
                        case SDLK_l: camX -= 20.0f; break;
                        case SDLK_j: camX += 20.0f; break;
                        case SDLK_i: camY += 20.0f; break;
                        case SDLK_k: camY -= 20.0f; break;
                        case SDLK_PLUS: case SDLK_KP_PLUS: currentScale += 1.0f; break;
                        case SDLK_MINUS: case SDLK_KP_MINUS: if(currentScale > 1) currentScale -= 1.0f; break;
                    }
                }
            }

            if (!paused && SDL_GetTicks() - lastGenTime > generationDelay) {
                for(int i=0; i<N; i++) for(int j=0; j<N; j++) for(int k=0; k<N; k++) nextGeneration3D(grid, nextGrid, i, j, k);
                for(int i=0; i<N; i++) for(int j=0; j<N; j++) for(int k=0; k<N; k++) grid[i][j][k] = nextGrid[i][j][k];
                generation++;
                lastGenTime = SDL_GetTicks();
            }

            drawCube(renderer, grid, angleX, angleY, currentScale, camX, camY, winWidth, winHeight);

            char title[100];
            snprintf(title, 100, "Gen: %d | [BACK] MENU | [SPACE] PAUSE", generation);
            SDL_SetWindowTitle(window, title);

            SDL_Delay(16);
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}