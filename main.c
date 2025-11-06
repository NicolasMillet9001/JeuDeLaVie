#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL.h>

// Dimension de la matrice
#define N 30
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


// Calcule dans une matrice temporaire l'état de la case [i][j] en fonction de la génération actuelle
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

int main() {

    //Définition du teux de cellules vivantes à la génération 0
    int tauxCellulesVivantes = 50;
    printf("Quelle est le taux de cellules vivantes a l'initialisation ? (Default : 50%%)\n");
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
                                        0); // Pas de flags particuliers
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


    int generation = 1;
    int running = 1; // Variable pour contrôler la boucle principale
    SDL_Event event; // Pour gérer les événements

    // Boucle principale du jeu
    while (running) {
        // --- 1. Gestion des événements ---
        // Vérifie si l'utilisateur veut quitter
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
            // Optionnel : quitter avec la touche 'q'
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_q) {
                running = 0;
    //boucle répétée pour toutes les nouvelles générations du jeu de la vie
    do {
        // matrice qui va garder en mémoire la prochaine génération
        int matriceTemp[N][N];
        printf("Generation %d\n", generation);
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                //Copie de l'état actuel de la cellule en cas de status quo
                matriceTemp[i][j] = matrice[i][j];
                nextGeneration(matrice,matriceTemp, i, j);
            }
        }

        // --- 2. Logique du jeu (calcul de la prochaine génération) ---
        int matriceTemp[N][N];

        // Met à jour le titre de la fenêtre avec le numéro de génération
        char title[50];
        snprintf(title, 50, "Jeu de la Vie - Generation %d", generation);
        SDL_SetWindowTitle(window, title);

        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                matriceTemp[i][j] = matrice[i][j]; // Copie état actuel
                nextGeneration(matrice, matriceTemp, i, j); // Calcule N+1
            }
        }

        // --- 3. Affichage (remplace afficherMatrice) ---
        drawGrid(renderer, matriceTemp);

        // --- 4. Mise à jour de l'état ---
        // Copier la matrice temporaire dans la matrice principale
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                matrice[i][j] = matriceTemp[i][j];
            }
        }

        // --- 5. Contrôle de la boucle ---
        generation++;

        // Arrête la simulation après 100 générations (comme l'original)
        // Vous pouvez commenter cette ligne pour une simulation infinie
        if (generation >= 100) {
            running = 0;
        }

        // Pause pour ralentir la simulation (remplace system("pause"))
        SDL_Delay(GENERATION_DELAY_MS);

    } // Fin de la boucle while(running)

    }while (generation < 100);

    // --- Nettoyage de SDL ---
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

