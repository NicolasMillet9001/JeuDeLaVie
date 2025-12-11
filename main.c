#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL.h>
#include <math.h>

// --- DIMENSIONS ---
#define N_3D 40          // Taille du cube 3D
#define N_2D 1000        // Taille de la grille 2D
#define INIT_WIDTH 800
#define INIT_HEIGHT 800
// Vitesse du déplacement automatique quand la souris est au bord
#define SCROLL_SPEED 15.0f
#define SCROLL_MARGIN 50


static const int filler[27][27] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,1,1,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,1,1,0,0,1,0,1,1,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,1,0,0,1,0,1,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,1,0,1,0,1,0,1,1,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,1,0,0,0,1,1,0,0},
    {1,1,1,1,0,0,0,0,0,1,0,1,0,0,0,0,1,0,0,0,1,0,1,1,1,0,0},
    {1,0,0,0,1,1,0,1,0,1,1,1,0,1,1,0,0,0,0,0,0,0,0,0,1,1,0},
    {1,0,0,0,0,0,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,1,0,0,1,1,0,1,0,0,1,0,0,1,0,1,1,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,0,0,1,1,1,1},
    {0,1,0,0,1,1,0,1,0,0,1,0,0,1,0,0,1,1,0,1,0,1,1,0,0,0,1},
    {1,0,0,0,0,0,1,1,0,0,0,1,0,1,0,1,0,0,0,1,1,0,0,0,0,0,1},
    {1,0,0,0,1,1,0,1,0,1,1,0,0,1,0,0,1,0,0,1,0,1,1,0,0,1,0},
    {1,1,1,1,0,0,0,0,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,1,1,0,1,0,0,1,0,0,1,0,1,1,0,0,1,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,1,0,0,0,0,0,1},
    {0,1,1,0,0,0,0,0,0,0,0,0,1,1,0,1,1,1,0,1,0,1,1,0,0,0,1},
    {0,0,1,1,1,0,1,0,0,0,1,0,0,0,0,1,0,1,0,0,0,0,0,1,1,1,1},
    {0,0,1,1,0,0,0,1,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,1,1,0,1,0,1,0,1,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,1,0,1,0,0,1,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,1,1,0,1,0,0,1,1,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,1,1,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

// ==========================================
// OUTILS DESSIN (TEXTE)
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
        {1,1,1, 1,0,0, 1,1,1, 0,0,1, 1,1,1}, // S
        {1,0,1, 1,1,1, 1,0,1, 1,0,1, 1,0,1}, // M
        {1,1,1, 0,1,0, 0,1,0, 0,1,0, 0,1,0}, // T
        {1,1,1, 1,0,1, 1,1,0, 1,0,1, 1,0,1}, // R
        {1,1,0, 1,0,1, 1,0,1, 1,0,1, 1,1,0}, // D
        {1,1,1, 1,0,0, 1,1,1, 1,0,0, 1,1,1}, // E (Edition)
    };
    int index = -1;
    if (c >= '0' && c <= '9') index = c - '0';
    if (c == 'S') index = 10; if (c == 'M') index = 11; if (c == 'T') index = 12;
    if (c == 'R') index = 13; if (c == 'D') index = 14; if (c == 'E') index = 15;
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
    for (int i = 0; buffer[i] != '\0'; i++) drawChar(renderer, x + (i * 4 * scale), y, scale, buffer[i]);
}

// ==========================================
// MOTEUR & AFFICHAGE
// ==========================================
// --- 3D ---
void drawCube(SDL_Renderer *renderer, int grid[N_3D][N_3D][N_3D], float angleX, float angleY,
              float currentScale, float camX, float camY, int winW, int winH) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    float cx = winW / 2.0f; float cy = winH / 2.0f; float offset = N_3D / 2.0f;
    for (int i = 0; i < N_3D; i++) {
        for (int j = 0; j < N_3D; j++) {
            for (int k = 0; k < N_3D; k++) {
                int age = grid[i][j][k];
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
                    int green_blue_val = 255 - (age * 5); if (green_blue_val < 0) green_blue_val = 0;
                    float depthShade = 1.0f - (z2 + N_3D*currentScale/2) / (N_3D*currentScale*2);
                    if (depthShade < 0.2f) depthShade = 0.2f; if (depthShade > 1.0f) depthShade = 1.0f;
                    SDL_SetRenderDrawColor(renderer, (Uint8)(255 * depthShade), (Uint8)(green_blue_val * depthShade), (Uint8)(green_blue_val * depthShade), 255);
                    SDL_Rect voxel = { screenX, screenY, size, size };
                    SDL_RenderFillRect(renderer, &voxel);
                }
            }
        }
    }
    SDL_RenderPresent(renderer);
}

void nextGeneration3D(int grid[N_3D][N_3D][N_3D], int nextGrid[N_3D][N_3D][N_3D]) {
    for (int i = 0; i < N_3D; i++) {
        for (int j = 0; j < N_3D; j++) {
            for (int k = 0; k < N_3D; k++) {
                int neighbors = 0;
                int currentAge = grid[i][j][k];
                for (int ni = i-1; ni <= i+1; ni++) {
                    if (ni<0 || ni>=N_3D) continue;
                    for (int nj = j-1; nj <= j+1; nj++) {
                        if (nj<0 || nj>=N_3D) continue;
                        for (int nk = k-1; nk <= k+1; nk++) {
                            if (nk<0 || nk>=N_3D) continue;
                            if (ni==i && nj==j && nk==k) continue;
                            if (grid[ni][nj][nk] > 0) neighbors++;
                        }
                    }
                }
                if (currentAge > 0) nextGrid[i][j][k] = (neighbors >= 4 && neighbors <= 5) ? currentAge + 1 : 0;
                else nextGrid[i][j][k] = (neighbors == 5) ? 1 : 0;
            }
        }
    }
}

// --- 2D INFINIE (Ou presque) ---
// Modification : Ajout du paramètre showGrid
void draw2D(SDL_Renderer *renderer, int grid[N_2D][N_2D], float currentScale, float camX, float camY, int winW, int winH, int showGrid) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    float cx = winW / 2.0f;
    float cy = winH / 2.0f;
    float offset = N_2D / 2.0f;
    int size = (int)currentScale;
    if (size < 1) size = 1;
    // Calcul des bornes visibles
    int min_i = (int)((0 - camX - cx) / size + offset) - 1;
    int max_i = (int)((winW - camX - cx) / size + offset) + 1;
    int min_j = (int)((0 - camY - cy) / size + offset) - 1;
    int max_j = (int)((winH - camY - cy) / size + offset) + 1;
    // Clamp
    if (min_i < 0) min_i = 0; if (max_i >= N_2D) max_i = N_2D - 1;
    if (min_j < 0) min_j = 0; if (max_j >= N_2D) max_j = N_2D - 1;
    // --- DESSIN DES LIGNES DE GRILLE (Si Mode Edition activé) ---
    if (showGrid && size > 2) {
        SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255); // Gris foncé
        // Lignes Verticales
        for (int i = min_i; i <= max_i; i++) {
            int screenX = (int)(cx + (i - offset) * size + camX);
            SDL_RenderDrawLine(renderer, screenX, 0, screenX, winH);
        }
        // Lignes Horizontales
        for (int j = min_j; j <= max_j; j++) {
            int screenY = (int)(cy + (j - offset) * size + camY);
            SDL_RenderDrawLine(renderer, 0, screenY, winW, screenY);
        }
    }
    // --- DESSIN DES CELLULES ---
    for (int i = min_i; i <= max_i; i++) {
        for (int j = min_j; j <= max_j; j++) {
            int age = grid[i][j];
            if (age > 0) {
                int screenX = (int)(cx + (i - offset) * size + camX);
                int screenY = (int)(cy + (j - offset) * size + camY);
                int green_blue_val = 255 - (age * 5);
                if (green_blue_val < 0) green_blue_val = 0;
                SDL_SetRenderDrawColor(renderer, 255, (Uint8)green_blue_val, (Uint8)green_blue_val, 255);
                SDL_Rect cell = { screenX, screenY, size, size };
                if (size > 4) { cell.w -= 1; cell.h -= 1; }
                SDL_RenderFillRect(renderer, &cell);
            }
        }
    }
    SDL_RenderPresent(renderer);
}

void nextGeneration2D(int grid[N_2D][N_2D], int nextGrid[N_2D][N_2D]) {
    for (int i = 1; i < N_2D - 1; i++) {
        for (int j = 1; j < N_2D - 1; j++) {
            int neighbors = 0;
            int currentAge = grid[i][j];
            if(grid[i-1][j-1]) neighbors++; if(grid[i][j-1]) neighbors++; if(grid[i+1][j-1]) neighbors++;
            if(grid[i-1][j])   neighbors++;                               if(grid[i+1][j])   neighbors++;
            if(grid[i-1][j+1]) neighbors++; if(grid[i][j+1]) neighbors++; if(grid[i+1][j+1]) neighbors++;
            if (currentAge > 0) nextGrid[i][j] = (neighbors == 2 || neighbors == 3) ? currentAge + 1 : 0;
            else nextGrid[i][j] = (neighbors == 3) ? 1 : 0;
        }
    }
}

// ==========================================
// MENU
// ==========================================
void drawMenu(SDL_Renderer* renderer, int winW, int winH, int selectedMode, int taux, int is3D) {
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);
    // Toggle 2D/3D
    SDL_Rect btnDim = {winW/2 - 80, 50, 160, 60};
    SDL_SetRenderDrawColor(renderer, 200, 200, 50, 255);
    SDL_RenderDrawRect(renderer, &btnDim);
    if (is3D) { drawChar(renderer, btnDim.x+50, btnDim.y+15, 6, '3'); drawChar(renderer, btnDim.x+80, btnDim.y+15, 6, 'D'); }
    else { drawChar(renderer, btnDim.x+50, btnDim.y+15, 6, '2'); drawChar(renderer, btnDim.x+80, btnDim.y+15, 6, 'D'); }
    // Bouton Random (Gauche)
    SDL_Rect btn1 = {winW/4 - 100, winH/3, 200, 150};
    if (selectedMode == 1) SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); else SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_RenderDrawRect(renderer, &btn1);
    for(int i=0; i<50; i++) SDL_RenderDrawPoint(renderer, btn1.x+rand()%200, btn1.y+rand()%150);
    drawChar(renderer, btn1.x+80, btn1.y+160, 5, 'R');
    // Bouton Edition (Droite)
    SDL_Rect btn2 = {3*winW/4 - 100, winH/3, 200, 150};
    if (selectedMode == 2) SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); else SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_RenderDrawRect(renderer, &btn2);
    // Icone Crayon
    SDL_Rect dot = {btn2.x+90, btn2.y+65, 20, 20}; SDL_RenderFillRect(renderer, &dot);
    drawChar(renderer, btn2.x+80, btn2.y+160, 5, 'E');
    // Slider Taux (Uniquement si Random)
    if (selectedMode == 1) {
        SDL_Rect barBg = {winW/4, 2*winH/3, winW/2, 20};
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
        SDL_RenderFillRect(renderer, &barBg);
        SDL_Rect barFill = {winW/4, 2*winH/3, (winW/2) * taux / 100, 20};
        SDL_SetRenderDrawColor(renderer, 200, 50, 50, 255);
        SDL_RenderFillRect(renderer, &barFill);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        drawChar(renderer, winW/4-40, 2*winH/3-5, 5, 'T');
        drawNumber(renderer, 3*winW/4+20, 2*winH/3-5, 5, taux);
    }
    // Bouton Start
    SDL_Rect btnStart = {winW/2 - 100, 4*winH/5, 200, 60};
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &btnStart);
    drawChar(renderer, btnStart.x+90, btnStart.y+15, 6, 'S');
    SDL_RenderPresent(renderer);
}

// ==========================================
// MAIN
// ==========================================
int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) return 1;
    SDL_Window* window = SDL_CreateWindow("Jeu de la Vie (Infinite 2D / 3D)", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, INIT_WIDTH, INIT_HEIGHT, SDL_WINDOW_RESIZABLE | SDL_WINDOW_FULLSCREEN_DESKTOP);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    int isFullscreen = 1;
    int programRunning = 1;
    // Allocation Statique
    static int grid3D[N_3D][N_3D][N_3D];
    static int nextGrid3D[N_3D][N_3D][N_3D];
    static int grid2D[N_2D][N_2D];
    static int nextGrid2D[N_2D][N_2D];
    static int initialGrid3D[N_3D][N_3D][N_3D];
    static int initialGrid2D[N_2D][N_2D];

    while (programRunning) {
        // --- PHASE MENU ---
        int inMenu = 1;
        int selectedMode = 1;
        int taux = 20;
        int is3D = 1;
        int winWidth, winHeight;
        SDL_GetWindowSize(window, &winWidth, &winHeight);
        while (inMenu && programRunning) {
            SDL_Event e;
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) programRunning = 0;
                if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_RESIZED) {
                    winWidth = e.window.data1; winHeight = e.window.data2;
                }
                if (e.type == SDL_KEYDOWN) {
                    if (e.key.keysym.sym == SDLK_ESCAPE) programRunning = 0;
                    if (e.key.keysym.sym == SDLK_LEFT) selectedMode = 1;
                    if (e.key.keysym.sym == SDLK_RIGHT) selectedMode = 2;
                    if (e.key.keysym.sym == SDLK_UP && taux < 100) taux += 5;
                    if (e.key.keysym.sym == SDLK_DOWN && taux > 5) taux -= 5;
                    if (e.key.keysym.sym == SDLK_RETURN) inMenu = 0;
                }
                if (e.type == SDL_MOUSEBUTTONDOWN) {
                    int mx = e.button.x; int my = e.button.y;
                    if (mx > winWidth/2-80 && mx < winWidth/2+80 && my > 50 && my < 110) is3D = !is3D; // Toggle
                    if (mx > winWidth/4-100 && mx < winWidth/4+100 && my > winHeight/3 && my < winHeight/3+150) selectedMode = 1; // Random
                    if (mx > 3*winWidth/4-100 && mx < 3*winWidth/4+100 && my > winHeight/3 && my < winHeight/3+150) selectedMode = 2; // Edition
                    if (mx > winWidth/2-100 && mx < winWidth/2+100 && my > 4*winHeight/5 && my < 4*winHeight/5+60) inMenu = 0; // Start
                    if (selectedMode == 1 && my > 2*winHeight/3 && my < 2*winHeight/3+20) { // Slider
                        int relX = mx - (winWidth/4);
                        if (relX >= 0 && relX <= winWidth/2) {
                            taux = (relX * 100) / (winWidth/2);
                        }
                    }
                }
            }
            drawMenu(renderer, winWidth, winHeight, selectedMode, taux, is3D);
            SDL_Delay(16);
        }
        if (!programRunning) break;
        // --- INITIALISATION ---
        srand(time(0));
        int paused = 0;
        // Reset grilles
        if (is3D) {
            for(int i=0; i<N_3D; i++) for(int j=0; j<N_3D; j++) for(int k=0; k<N_3D; k++) grid3D[i][j][k] = 0;
        } else {
            for(int i=0; i<N_2D; i++) for(int j=0; j<N_2D; j++) grid2D[i][j] = 0;
        }
        if (selectedMode == 1) { // RANDOM
            if (is3D) {
                for(int i=0; i<N_3D; i++) for(int j=0; j<N_3D; j++) for(int k=0; k<N_3D; k++) {
                    grid3D[i][j][k] = (rand() % 100 < taux) ? 1 : 0;
                    initialGrid3D[i][j][k] = grid3D[i][j][k];
                }
            } else {
                for(int i=0; i<N_2D; i++) for(int j=0; j<N_2D; j++) {
                    grid2D[i][j] = (rand() % 100 < taux) ? 1 : 0;
                    initialGrid2D[i][j] = grid2D[i][j];
                }
            }
        } else { // EDITION
            paused = 1; // On commence en pause pour éditer
            if (is3D) {
                for(int i=0; i<N_3D; i++) for(int j=0; j<N_3D; j++) for(int k=0; k<N_3D; k++) initialGrid3D[i][j][k] = 0;
            } else {
                for(int i=0; i<N_2D; i++) for(int j=0; j<N_2D; j++) initialGrid2D[i][j] = 0;
            }
        }
        int generationDelay = 50;
        float angleX = 0.0f; float angleY = 0.0f;
        float currentScale = is3D ? 10.0f : 20.0f; // Zoom par défaut
        float camX = 0.0f; float camY = 0.0f;
        int generation = 0;
        Uint32 lastGenTime = SDL_GetTicks();
        SDL_GetWindowSize(window, &winWidth, &winHeight);
        int isPainting = 0; // Pour dessiner en maintenant la souris
        // --- BOUCLE JEU ---
        int gameRunning = 1;
        while (gameRunning && programRunning) {
            SDL_Event event;
            // --- EDGE SCROLLING (2D) ---
            if (is3D == 0) {
                int mx, my;
                SDL_GetMouseState(&mx, &my);
                if (mx < SCROLL_MARGIN) camX += SCROLL_SPEED;
                if (mx > winWidth - SCROLL_MARGIN) camX -= SCROLL_SPEED;
                if (my < SCROLL_MARGIN) camY += SCROLL_SPEED;
                if (my > winHeight - SCROLL_MARGIN) camY -= SCROLL_SPEED;
            }
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) programRunning = 0;
                if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    winWidth = event.window.data1; winHeight = event.window.data2;
                }
                // --- EDITION 2D A LA SOURIS ---
                if (!is3D && selectedMode == 2) {
                    if (event.type == SDL_MOUSEBUTTONDOWN) isPainting = 1;
                    if (event.type == SDL_MOUSEBUTTONUP) isPainting = 0;
                }
                if (event.type == SDL_KEYDOWN) {
                    switch(event.key.keysym.sym) {
                        case SDLK_ESCAPE: programRunning = 0; break;
                        case SDLK_BACKSPACE: gameRunning = 0; break;
                        case SDLK_SPACE: paused = !paused; break;
                        case SDLK_F11:
                            isFullscreen = !isFullscreen;
                            SDL_SetWindowFullscreen(window, isFullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
                            SDL_GetWindowSize(window, &winWidth, &winHeight);
                            break;
                        case SDLK_e: generationDelay = (generationDelay > 1) ? generationDelay - 10 : 1; break;
                        case SDLK_a: generationDelay += 10; break;
                        case SDLK_LEFT: angleY -= 0.1f; break; case SDLK_RIGHT: angleY += 0.1f; break;
                        case SDLK_UP: angleX -= 0.1f; break; case SDLK_DOWN: angleX += 0.1f; break;
                        case SDLK_EQUALS: angleX=0; angleY=0; currentScale=10.0f; camX=0; camY=0; break;
                        case SDLK_l: camX -= 20.0f; break; case SDLK_j: camX += 20.0f; break;
                        case SDLK_i: camY += 20.0f; break; case SDLK_k: camY -= 20.0f; break;
                        case SDLK_PLUS: case SDLK_KP_PLUS: currentScale += 1.0f; break;
                        case SDLK_MINUS: case SDLK_KP_MINUS: if(currentScale > 1) currentScale -= 1.0f; break;
                        // --- COLLAGE DU MOTIF (Touche P) ---
                        case SDLK_4:
                            // On ne place le motif que si on est en 2D et en mode Edition
                            if (!is3D && selectedMode == 2) {
                                int mx, my;
                                SDL_GetMouseState(&mx, &my);
                                // Recalcul des coordonnées grille
                                float cx = winWidth / 2.0f;
                                float cy = winHeight / 2.0f;
                                float offset = N_2D / 2.0f;
                                int size = (int)currentScale; if (size < 1) size = 1;
                                int startI = (int)((mx - camX - cx) / size + offset);
                                int startJ = (int)((my - camY - cy) / size + offset);
                                for (int pi = 0; pi < 27; pi++) {
                                    for (int pj = 0; pj < 27; pj++) {
                                        int targetI = startI + pi;
                                        int targetJ = startJ + pj;
                                        if (targetI >= 0 && targetI < N_2D && targetJ >= 0 && targetJ < N_2D) {
                                            if (filler[pj][pi]) {
                                                grid2D[targetI][targetJ] = 1;
                                            }
                                        }
                                    }
                                }
                            }
                            break;
                        // --- REINITIALISATION (Touche R) ---
                        case SDLK_r:
                            if (is3D) {
                                for(int i=0; i<N_3D; i++) for(int j=0; j<N_3D; j++) for(int k=0; k<N_3D; k++) grid3D[i][j][k] = initialGrid3D[i][j][k];
                            } else {
                                for(int i=0; i<N_2D; i++) for(int j=0; j<N_2D; j++) grid2D[i][j] = initialGrid2D[i][j];
                            }
                            generation = 0;
                            angleX = 0.0f; angleY = 0.0f; currentScale = is3D ? 10.0f : 20.0f; camX = 0; camY = 0;
                            break;
                    }
                }
            }
            // --- LOGIQUE PEINTURE 2D (Hors boucle événement pour fluidité) ---
            if (isPainting && !is3D && selectedMode == 2) {
                int mx, my;
                SDL_GetMouseState(&mx, &my);
                float cx = winWidth / 2.0f;
                float cy = winHeight / 2.0f;
                float offset = N_2D / 2.0f;
                int size = (int)currentScale; if (size < 1) size = 1;
                // Formule inverse : Trouve i et j à partir de la position souris
                int i = (int)((mx - camX - cx) / size + offset);
                int j = (int)((my - camY - cy) / size + offset);
                if (i >= 0 && i < N_2D && j >= 0 && j < N_2D) {
                    grid2D[i][j] = 1; // Active la cellule
                }
            }
            if (!paused && SDL_GetTicks() - lastGenTime > generationDelay) {
                if (is3D) {
                    nextGeneration3D(grid3D, nextGrid3D);
                    for(int i=0; i<N_3D; i++) for(int j=0; j<N_3D; j++) for(int k=0; k<N_3D; k++) grid3D[i][j][k] = nextGrid3D[i][j][k];
                } else {
                    nextGeneration2D(grid2D, nextGrid2D);
                    for(int i=0; i<N_2D; i++) for(int j=0; j<N_2D; j++) grid2D[i][j] = nextGrid2D[i][j];
                }
                generation++;
                lastGenTime = SDL_GetTicks();
            }
            if (is3D) {
                drawCube(renderer, grid3D, angleX, angleY, currentScale, camX, camY, winWidth, winHeight);
            } else {
                // On passe (selectedMode == 2) pour afficher la grille si on est en édition
                draw2D(renderer, grid2D, currentScale, camX, camY, winWidth, winHeight, (selectedMode == 2));
            }
            char title[100];
            // Ajout de l'info "[P] PASTE" dans le titre si en mode édition 2D
            if (!is3D && selectedMode == 2) {
                 snprintf(title, 100, "2D EDITION | Gen: %d | [BACK] MENU | [P] PASTE | [SPACE] RUN | [R] RESET", generation);
            } else {
                 snprintf(title, 100, "%s | Gen: %d | [BACK] MENU | %s | [R] RESET", is3D?"3D":"2D", generation, paused?"PAUSE":"RUN");
            }
            SDL_SetWindowTitle(window, title);
            SDL_Delay(16);
        }
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
