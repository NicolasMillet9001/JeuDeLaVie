#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL.h>
#include <math.h>
#include <float.h>

#define N 400
#define CELL_SIZE 5
#define WIDTH (N * CELL_SIZE)
#define HEIGHT (N * CELL_SIZE)
#define GENERATION_DELAY_MS 100 // Délai en ms entre les générations
#define FADE_DURATION_RATIO 0.8f

/**
 * @brief Structure pour stocker une couleur RGB
 */
typedef struct {
    Uint8 r;
    Uint8 g;
    Uint8 b;
} RGBColor;

/**
 * @brief Convertit une couleur du modèle HSV (Teinte, Saturation, Valeur) au modèle RGB.
 * @param h Teinte (0.0f à 360.0f)
 * @param s Saturation (0.0f à 1.0f)
 * @param v Valeur (0.0f à 1.0f)
 * @return Une structure RGBColor avec les valeurs (0-255)
 */
RGBColor HsvToRgb(float h, float s, float v) {
    float C = v * s;
    float X = C * (1.0f - fabsf(fmodf(h / 60.0f, 2.0f) - 1.0f));
    float m = v - C;
    float r_prime, g_prime, b_prime;

    if (h >= 0 && h < 60) {
        r_prime = C; g_prime = X; b_prime = 0;
    } else if (h >= 60 && h < 120) {
        r_prime = X; g_prime = C; b_prime = 0;
    } else if (h >= 120 && h < 180) {
        r_prime = 0; g_prime = C; b_prime = X;
    } else if (h >= 180 && h < 240) {
        r_prime = 0; g_prime = X; b_prime = C;
    } else if (h >= 240 && h < 300) {
        r_prime = X; g_prime = 0; b_prime = C;
    } else { // h >= 300 && h < 360
        r_prime = C; g_prime = 0; b_prime = X;
    }

    RGBColor color;
    color.r = (Uint8)((r_prime + m) * 255);
    color.g = (Uint8)((g_prime + m) * 255);
    color.b = (Uint8)((b_prime + m) * 255);
    return color;
}

// Notez le changement: int grid[N][N] -> float displayGrid[N][N]
void drawGrid(SDL_Renderer *renderer, float displayGrid[N][N]) {

    // 1. On dessine le fond en noir
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // 2. On dessine les cellules
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {

            // On dessine la cellule si sa luminosité est > 0
            float brightness = displayGrid[i][j];
            if (brightness > 0.001f) { // Seuil minimal pour dessiner

                // --- CALCUL DE LA COULEUR ---
                float hue = fmodf((float)(i + j) * 0.5f, 360.0f);

                // NOUVEAU: La "Value" (luminosité) est pilotée par la displayGrid
                RGBColor cellColor = HsvToRgb(hue, 1.0f, brightness);

                // Applique la couleur calculée
                SDL_SetRenderDrawColor(renderer, cellColor.r, cellColor.g, cellColor.b, 255);

                SDL_Rect cell = { j * CELL_SIZE, i * CELL_SIZE, CELL_SIZE, CELL_SIZE };
                SDL_RenderFillRect(renderer, &cell);
            }
        }
    }

    // 3. On affiche le tout à l'écran
    SDL_RenderPresent(renderer);
}


// Définie dans une matrice temporaire l'état de la case [i][j] en fonction de la génération actuelle
void nextGeneration(int matrice[N][N], int matriceNplus1[N][N], int i, int j) {
    int compteurDeCellulesAdjacentesVivantes = 0;
    int currentState = matrice[i][j];

    //Compte le nombre de cellules adjacentes vivantes
    for (int compteuri = i-1; compteuri<= i+1; compteuri++) {
        //On s'assure que l'on vérifie les cases adjacentes dans la matrice et non à l'extérieur
        if (compteuri>=0 && compteuri < N) {
            for (int compteurj = j-1; compteurj<= j+1; compteurj++) {
                //On s'assure que l'on vérifie les cases adjacentes dans la matrice et non à l'extérieur
                if (compteurj>=0 && compteurj < N) {
                    if (matrice[compteuri][compteurj] == 1) {
                        //La cellule ne se compte pas elle-même dans le compte de cellule adjacente vivante
                        if (i != compteuri || j != compteurj) {
                            compteurDeCellulesAdjacentesVivantes++;
                        }
                    }
                }
            }
        }
    }

    if (currentState == 0) {
        //Si le nombre de cellules adjacentes vivantes est de 3, la cellule deviendra vivante à la prochaine génération
        if(compteurDeCellulesAdjacentesVivantes == 3) {
            matriceNplus1[i][j] = 1;
        }
    }else {
        //Si le nombre de cellules adjacentes vivantes et trop important ou trop peu, la cellule meurt à la prochaine génération
        if (compteurDeCellulesAdjacentesVivantes < 2 || compteurDeCellulesAdjacentesVivantes > 3) {
            matriceNplus1[i][j] = 0;
        }
    }

}

int main(int argc, char* argv[]) {

    int mode = 0;
    int tauxCellulesVivantes = 50;

    // --- Menu de sélection dans la console ---
    // (Code inchangé)
    printf("Choisissez votre mode :\n");
    printf("  1: Simulation Aleatoire\n");
    printf("  2: Mode Editeur (creer vous-meme)\n");
    printf("Votre choix : ");
    if (scanf("%d", &mode) != 1) {
        mode = 1;
        while (getchar() != '\n');
    }

    // --- NOUVEAU : Deux grilles ---
    int logicGrid[N][N];     // La grille de la logique du jeu (1 ou 0)
    float displayGrid[N][N]; // La grille de l'affichage (0.0f à 1.0f)

    if (mode == 1) {
        // --- MODE 1 : ALÉATOIRE ---
        printf("Quelle est le taux de cellule vivante a l'initialisation ? (Default : 50%%)\n");
        if (scanf("%i", &tauxCellulesVivantes) != 1) {
            while (getchar() != '\n');
        }
        srand(time(0));
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                int state = (rand() % 100 + 1 <= tauxCellulesVivantes) ? 1 : 0;
                logicGrid[i][j] = state;
                displayGrid[i][j] = (float)state; // L'affichage commence à l'état logique
            }
        }
    } else {
        // --- MODE 2 : EDITEUR ---
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                logicGrid[i][j] = 0;
                displayGrid[i][j] = 0.0f;
            }
        }
    }

    // --- Initialisation de SDL ---
    // (Code inchangé)
    if (SDL_Init(SDL_INIT_VIDEO) != 0) { /* ... gestion erreur ... */ return 1; }
    SDL_Window* window = SDL_CreateWindow("Jeu de la Vie",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,WIDTH, HEIGHT,

0);
    if (window == NULL) { /* ... gestion erreur ... */ SDL_Quit(); return 1; }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) { /* ... gestion erreur ... */ SDL_DestroyWindow(window); SDL_Quit(); return 1; }
    // --- Fin de l'initialisation de SDL ---


    int generation = 0;
    int running = 1;
    int isPaused = 1;
    int mousePressed = 0;
    SDL_Event event;

    // --- NOUVEAU : Timers pour la boucle de jeu ---
    Uint32 lastGenerationTime = SDL_GetTicks(); // Timer pour la logique
    Uint32 lastFrameTime = SDL_GetTicks();      // Timer pour le delta-time (fluidité)

    // Durée du fondu en ms
    const float fadeDurationMs = (float)GENERATION_DELAY_MS * FADE_DURATION_RATIO;

    // --- Affichage de l'état initial (Generation 0) ---
    drawGrid(renderer, displayGrid); // On passe la grille d'affichage

    // Boucle principale du jeu (basée sur les frames, pas les générations)
    while (running) {

        // --- 1. Calcul du Delta Time (temps écoulé depuis la dernière frame) ---
        Uint32 currentFrameTime = SDL_GetTicks();
        float deltaTimeMs = (float)(currentFrameTime - lastFrameTime);
        lastFrameTime = currentFrameTime;

        // --- 2. Gestion des événements ---
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }

            if (event.type == SDL_KEYDOWN) {
                // (Gestion 'q' et 'Espace' inchangée)
                switch (event.key.keysym.sym) {
                    case SDLK_q: running = 0; break;
                    case SDLK_SPACE:
                        isPaused = !isPaused;
                        if (!isPaused && generation == 0) {
                            generation = 1;
                            lastGenerationTime = currentFrameTime; // Démarrer le timer de gen
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
                            logicGrid[i][j] = !logicGrid[i][j]; // Inverse état logique
                            displayGrid[i][j] = (float)logicGrid[i][j]; // Snap l'affichage
                        }
                    }
                }
                if (event.type == SDL_MOUSEBUTTONUP) { /* ... mousePressed = 0 ... */ }
                if (event.type == SDL_MOUSEMOTION) {
                    if (mousePressed) {
                        int j = event.motion.x / CELL_SIZE;
                        int i = event.motion.y / CELL_SIZE;
                        if (i >= 0 && i < N && j >= 0 && j < N) {
                            logicGrid[i][j] = 1; // Dessine
                            displayGrid[i][j] = 1.0f; // Snap l'affichage
                        }
                    }
                }
            }
        } // Fin de la boucle d'événements

        // --- 3. Mise à jour du titre ---
        // (Code inchangé)
        char title[100];
        if (isPaused) { /* ... snprintf ... */ } else { /* ... snprintf ... */ }
        SDL_SetWindowTitle(window, title);


        // --- 4. Logique du jeu ---
        if (!isPaused) {

            // --- A. Logique de Génération (basée sur le timer) ---
            if (currentFrameTime - lastGenerationTime > GENERATION_DELAY_MS) {

                int logicTemp[N][N]; // Grille temporaire pour la N+1
                for (int i = 0; i < N; i++) {
                    for (int j = 0; j < N; j++) {
                        logicTemp[i][j] = logicGrid[i][j];
                        // On passe la grille LOGIQUE à nextGeneration
                        nextGeneration(logicGrid, logicTemp, i, j);
                    }
                }
                // Copier le résultat dans la grille logique
                for (int i = 0; i < N; i++) {
                    for (int j = 0; j < N; j++) {
                        logicGrid[i][j] = logicTemp[i][j];
                    }
                }

                generation++;
                lastGenerationTime = currentFrameTime; // Réinitialiser le timer
            }

            // --- B. Logique d'Affichage (basée sur le Delta Time) ---
            // On fait "rattraper" la grille d'affichage vers la grille logique

            // Incrément de fondu pour CETTE frame
            float fadeIncrement = deltaTimeMs / fadeDurationMs;

            for (int i = 0; i < N; i++) {
                for (int j = 0; j < N; j++) {
                    if (logicGrid[i][j] == 1) {
                        // Fade-in
                        displayGrid[i][j] = fminf(1.0f, displayGrid[i][j] + fadeIncrement);
                    } else {
                        // Fade-out
                        displayGrid[i][j] = fmaxf(0.0f, displayGrid[i][j] - fadeIncrement);
                    }
                }
            }

            // --- C. Dessin ---
            drawGrid(renderer, displayGrid);

            // PAS de SDL_Delay ici, VSYNC (ou la boucle) s'en charge

        } else {
            // --- EN PAUSE / EN MODE EDITEUR ---
            // On redessine pour voir les changements de la souris
            drawGrid(renderer, displayGrid);
            SDL_Delay(100); // Petite pause pour ne pas saturer le CPU
        }

    } // Fin de la boucle while(running)


    // --- Boucle de pause finale ---
    // (Code inchangé)
    char finalTitle[100];
    snprintf(finalTitle, 100, "Jeu de la Vie - FIN (Gen %d) - 'q' ou 'X' pour quitter", generation - 1);
    SDL_SetWindowTitle(window, finalTitle);
    int pause = 1;
    while (pause) { /* ... gestion événements ... */ SDL_Delay(100); }
    // --- Fin de la boucle de pause ---

    // --- Nettoyage de SDL ---
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

