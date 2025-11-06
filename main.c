#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL.h>
#include <math.h>

#define N 400
#define CELL_SIZE 5
#define WIDTH (N * CELL_SIZE)
#define HEIGHT (N * CELL_SIZE)
#define GENERATION_DELAY_MS 100 // Délai en ms entre les générations

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

void drawGrid(SDL_Renderer *renderer, int grid[N][N]) {

    // 1. On dessine le fond en noir
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // 2. On dessine les cellules vivantes (avec dégradé de couleur)
    // La couleur est définie DANS la boucle
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (grid[i][j]) {

                // --- CALCUL DE LA COULEUR ---
                // Calcule la Teinte (Hue) en fonction de la position (i, j)
                // On crée des bandes diagonales qui balaient le spectre des couleurs
                // Vous pouvez jouer avec "0.5f" pour changer la largeur des bandes
                float hue = fmodf((float)(i + j) * 0.5f, 360.0f);

                // On garde la saturation et la valeur à 1.0 pour des couleurs vives
                RGBColor cellColor = HsvToRgb(hue, 1.0f, 1.0f);

                // Applique la couleur calculée pour CETTE cellule
                SDL_SetRenderDrawColor(renderer, cellColor.r, cellColor.g, cellColor.b, 255);

                // Crée un rectangle pour la cellule vivante
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

    // --- NOUVEAU : Menu de sélection dans la console ---
    printf("Choisissez votre mode :\n");
    printf("  1: Simulation Aleatoire\n");
    printf("  2: Mode Editeur (creer vous-meme)\n");
    printf("Votre choix : ");

    if (scanf("%d", &mode) != 1) {
        mode = 1; // Par défaut, mode aléatoire
        while (getchar() != '\n'); // Vider le buffer
    }

    //definition de la matrice de départ
    int matrice[N][N];

    if (mode == 1) {
        // --- MODE 1 : ALÉATOIRE (Code existant) ---
        printf("Quelle est le taux de cellule vivante a l'initialisation ? (Default : 50%%)\n");
        if (scanf("%i", &tauxCellulesVivantes) != 1) {
            while (getchar() != '\n');
        }
        srand(time(0)); // Initialiser l'aléatoire
        // Remplir la matrice avec des valeurs aléatoires
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                matrice[i][j] = (rand() % 100 + 1 <= tauxCellulesVivantes) ? 1 : 0;
            }
        }
    } else {
        // --- MODE 2 : EDITEUR ---
        // Remplir la matrice avec des 0
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                matrice[i][j] = 0;
            }
        }
    }

    // --- Initialisation de SDL ---
    // (Code d'initialisation SDL inchangé)
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Erreur lors de l'initialisation de la SDL : %s\n", SDL_GetError());
        return 1;
    }
    SDL_Window* window = SDL_CreateWindow("Jeu de la Vie",
                                        SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED,
                                        WIDTH, HEIGHT,
                                        0);
    if (window == NULL) {
        fprintf(stderr, "Erreur lors de la création de la fenêtre : %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1,
                                              SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        fprintf(stderr, "Erreur lors de la création du renderer : %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    // --- Fin de l'initialisation de SDL ---


    int generation = 0; // On commence à la génération 0
    int running = 1; // Variable pour contrôler la boucle principale
    int isPaused = 1; // 1 = en pause, 0 = en cours (commence en pause)
    int mousePressed = 0; // Pour gérer le dessin en glissant
    SDL_Event event; // Pour gérer les événements

    // --- Affichage de l'état initial (Generation 0) ---
    // (Affiche la grille aléatoire ou la grille vide)
    drawGrid(renderer, matrice);


    // Boucle principale du jeu
    while (running) {

        // --- 1. Gestion des événements ---
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }

            // --- GESTION CLAVIER ---
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_q: // 'q' pour quitter
                        running = 0;
                        break;
                    case SDLK_SPACE: // 'Espace' pour mettre en pause/reprendre
                        isPaused = !isPaused; // Bascule l'état de pause
                        // Si on sort de la pause à la Gen 0, on passe à la Gen 1
                        if (!isPaused && generation == 0) {
                            generation = 1;
                        }
                        break;
                }
            }

            // --- NOUVEAU : GESTION SOURIS (uniquement en mode pause/éditeur) ---
            if (isPaused) {
                if (event.type == SDL_MOUSEBUTTONDOWN) {
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        mousePressed = 1;
                        // On "clique" sur une cellule pour l'inverser
                        int j = event.button.x / CELL_SIZE;
                        int i = event.button.y / CELL_SIZE;
                        // Vérifie les bornes
                        if (i >= 0 && i < N && j >= 0 && j < N) {
                            matrice[i][j] = !matrice[i][j]; // Inverse l'état
                        }
                    }
                }
                if (event.type == SDL_MOUSEBUTTONUP) {
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        mousePressed = 0; // On relâche le clic
                    }
                }
                if (event.type == SDL_MOUSEMOTION) {
                    if (mousePressed) {
                        // Si on glisse la souris en cliquant, on "dessine"
                        int j = event.motion.x / CELL_SIZE;
                        int i = event.motion.y / CELL_SIZE;
                        if (i >= 0 && i < N && j >= 0 && j < N) {
                            matrice[i][j] = 1; // Met la cellule à "vivante"
                        }
                    }
                }
            } // Fin gestion souris

        } // Fin de la boucle d'événements


        // --- 2. Mise à jour du titre de la fenêtre (selon l'état) ---
        char title[100];
        if (isPaused) {
            // Si on est à la génération 0, on est en mode "Editeur"
            if (generation == 0) {
                 snprintf(title, 100, "Jeu de la Vie - MODE EDITEUR - Dessinez et appuyez sur Espace");
            } else {
                 snprintf(title, 100, "Jeu de la Vie - Gen %d (PAUSE) - Espace pour reprendre", generation);
            }
        } else {
            snprintf(title, 100, "Jeu de la Vie - Generation %d", generation);
        }
        SDL_SetWindowTitle(window, title);


        // --- 3. Logique du jeu ---
        if (!isPaused) {
            // --- SIMULATION EN COURS ---
            // (Code de simulation inchangé)
            int matriceTemp[N][N];
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < N; j++) {
                    matriceTemp[i][j] = matrice[i][j]; // Copie état actuel
                    nextGeneration(matrice, matriceTemp, i, j); // Calcule N+1
                }
            }

            // Affichage
            drawGrid(renderer, matriceTemp);

            // Mise à jour
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < N; j++) {
                    matrice[i][j] = matriceTemp[i][j];
                }
            }

            generation++;
            SDL_Delay(GENERATION_DELAY_MS);

        } else {
            // --- EN PAUSE / EN MODE EDITEUR ---
            // On redessine la grille pour voir les changements faits à la souris
            drawGrid(renderer, matrice);
            // Petite pause pour ne pas utiliser 100% du CPU
            SDL_Delay(100);
        }

    } // Fin de la boucle while(running)


    // --- Boucle de pause finale ---
    // (Code inchangé)
    char finalTitle[100];
    snprintf(finalTitle, 100, "Jeu de la Vie - FIN (Gen %d) - 'q' ou 'X' pour quitter", generation - 1);
    SDL_SetWindowTitle(window, finalTitle);

    int pause = 1;
    while (pause) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                pause = 0;
            }
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_q) {
                pause = 0;
            }
        }
        SDL_Delay(100);
    }
    // --- Fin de la boucle de pause ---

    // --- Nettoyage de SDL ---
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

