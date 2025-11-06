#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL.h>

#define N 80
#define CELL_SIZE 10
#define WIDTH (N * CELL_SIZE)
#define HEIGHT (N * CELL_SIZE)
#define GENERATION_DELAY_MS 100 // Délai en ms entre les générations

void drawGrid(SDL_Renderer *renderer, int grid[N][N]) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (grid[i][j]) {
                SDL_Rect cell = { j * CELL_SIZE, i * CELL_SIZE, CELL_SIZE, CELL_SIZE };
                SDL_RenderFillRect(renderer, &cell);
            }
        }
    }
    SDL_RenderPresent(renderer);
}

void afficherMatrice(int matrice[N][N]) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (matrice[i][j] == 0)
                printf(" .  ");
            else if (matrice[i][j] == 1)
                printf(" #  ");
            else
                printf(" ?  "); // Au cas où il y a d'autres valeurs
        }
        printf("\n");
    }
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
    //Définition du teux de cellules vivantes à la génération 0
    int tauxCellulesVivantes = 50;
    printf("Quelle est le taux de cellule vivante a l'initialisation ? (Default : 50%%)\n");
    if (scanf("%i", &tauxCellulesVivantes) != 1) {
        // On vide le buffer pour éviter les boucles infinies
        while (getchar() != '\n');
    }

    //Pour avoir de vraies valeurs aléatoires
    srand(time(0));

    //definition de la matrice de départ
    int matrice[N][N];
    // Remplir la matrice avec des valeurs aléatoires
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            matrice[i][j] = (rand() % 100 + 1 <= tauxCellulesVivantes) ? 1 : 0;
        }
    }

    // --- Initialisation de SDL ---
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
    SDL_Event event; // Pour gérer les événements

    // --- Affichage de l'état initial (Generation 0) ---
    // On dessine la grille une première fois avant la boucle
    drawGrid(renderer, matrice);


    // Boucle principale du jeu
    while (running) {

        // --- 1. Gestion des événements ---
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
            // Gestion des touches clavier
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
        } // Fin de la boucle d'événements


        // --- 2. Mise à jour du titre de la fenêtre (selon l'état) ---
        char title[100];
        if (isPaused) {
            snprintf(title, 100, "Jeu de la Vie - Gen %d (PAUSE) - Espace pour démarrer", generation);
        } else {
            snprintf(title, 100, "Jeu de la Vie - Generation %d", generation);
        }
        SDL_SetWindowTitle(window, title);


        // --- 3. Logique du jeu (uniquement si on N'EST PAS en pause) ---
        if (!isPaused) {

            // --- Calcul de la génération N+1 ---
            int matriceTemp[N][N];
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < N; j++) {
                    matriceTemp[i][j] = matrice[i][j]; // Copie état actuel
                    nextGeneration(matrice, matriceTemp, i, j); // Calcule N+1
                }
            }

            // --- 4. Affichage ---
            drawGrid(renderer, matriceTemp);

            // --- 5. Mise à jour de l'état ---
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < N; j++) {
                    matrice[i][j] = matriceTemp[i][j];
                }
            }

            generation++; // On passe à la génération suivante

            // Pause de la simulation
            SDL_Delay(GENERATION_DELAY_MS);

        } else {
            // Si on est en pause, on met quand même un petit délai
            // pour ne pas que la boucle "while(running)" consomme 100% du CPU
            SDL_Delay(100);
        }

    } // Fin de la boucle while(running)


    // --- C'est ici que va le code que vous aviez collé ---
    // --- Boucle de pause finale ---

    // Met à jour le titre une dernière fois
    char finalTitle[100];
    snprintf(finalTitle, 100, "Jeu de la Vie - FIN (Gen %d) - 'q' ou 'X' pour quitter", generation - 1);
    SDL_SetWindowTitle(window, finalTitle);

    int pause = 1;
    while (pause) {
        // On vérifie juste les événements de fermeture
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                pause = 0;
            }
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_q) {
                pause = 0;
            }
        }
        // Petite pause pour ne pas utiliser 100% du CPU
        SDL_Delay(100);
    }
    // --- Fin de la boucle de pause ---


    // --- Nettoyage de SDL ---
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

