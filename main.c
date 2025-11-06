#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Dimension de la matrice
#define N 30


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
    afficherMatrice(matrice);


    int generation = 1;
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
        afficherMatrice(matriceTemp);

        //Copier la matrice N+1 dans la matrice actuelle après la fin des calculs
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                matrice[i][j] = matriceTemp[i][j];
            }
        }

        //Pause pour voir la nouvelle génération dans la console
        system("pause");
        generation++;

    }while (generation < 100);


    return 0;
}

