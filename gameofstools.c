#include <MLV/MLV_all.h>
#include "gameofstools.h"

Agent *
ajouteChateau(AListe clan, char couleur, int x, int y)
{
	Agent *chateau;

	/* Création du chateau */
	chateau = malloc(sizeof(Agent));
	chateau->clan = couleur;
	chateau->genre = CHATEAU;
	chateau->posx = x;
	chateau->posy = y;
	chateau->aprec = NULL;
	chateau->asuiv = NULL;
	chateau->vprec = NULL;
	chateau->vsuiv = NULL;

	/* Ajout du chateau */
	clan = chateau;

	return chateau;
}

Agent *
ajouteAgent(Agent *chateau, char genre, Case plateau[NBLIG][NBCOL])
{
	Agent *agent, *newagent;
	int i, j;

	agent = chateau;

	/* On ajoute le baron juste après le dernier baron */
	while (agent->asuiv != NULL && agent->asuiv->genre != genre) {
		agent = agent->asuiv;
	}

	/* On est sur le premier agent de genre "genre" */
	newagent = malloc(sizeof(Agent));
	newagent->clan = chateau->clan;
	newagent->genre = genre;
	/* Case libre voisine */
	newagent->posx = -1;
	newagent->posy = -1;
	for (i = chateau->posx - 1 ; i <= chateau->posx + 1; i++) {
		if (i < 0 || i >= NBLIG) {
			continue;
		}
		for (j = chateau->posy - 1 ; j <= chateau->posy + 1; j++) {
			if (j < 0 || j >= NBCOL) {
				continue;
			}
			if (plateau[i][j].chateau == NULL && plateau[i][j].habitant == NULL) {
				newagent->posx = i;
				newagent->posy = j;
				break;
			}
		}
		if (newagent->posx != -1) {
			break;
		}
	}

	/* Insertion dans la liste chaînée */
	newagent->asuiv = agent->asuiv;
	agent->asuiv = newagent;

	/* Ajout sur le plateau */
	plateau[newagent->posx][newagent->posy].habitant = newagent;

	return newagent;
}

void affichePlateau(Case plateau[NBLIG][NBCOL])
{
	int i, j;

	/* Une première ligne horizontale */
	for (j = 0; j < NBCOL; j++) {
		printf("------");
	}
	printf("-\n");

	for (i = 0; i < NBLIG; i++) {
		for (j = 0; j < NBCOL; j++) {
			printf("|");
			if (plateau[i][j].chateau != NULL) {
				printf("%cC", plateau[i][j].chateau->clan);
			}
			else {
				printf("  ");
			}
			if (plateau[i][j].habitant != NULL) {
				printf("XXX");
			}
			else {
				printf("   ");
			}
		}
		/* Dernière colonne */
		printf("|\n");
		/* Ligne horizontale inférieure */
		for (j = 0; j < NBCOL; j++) {
			printf("------");
		}
		printf("-\n");
	}
}

int main(int argc, char *argv[])
{
	Monde monde;
	Agent *chateau, *baron, *manant;
	int i, j;

	/* Mise en place */
	monde.tour = 0;
	monde.tresorRouge = 50;
	monde.tresorBleu = 50;

	/* Toutes les cases sont neutres et vides */
	for (i = 0; i < NBLIG; i++) {
		for (j = 0; j < NBCOL; j++) {
			monde.plateau[i][j].clan = LIBRE;
			monde.plateau[i][j].chateau = NULL;
			monde.plateau[i][j].habitant = NULL;
		}
	}

	/* Initialisation des listes d'agents */
	chateau = ajouteChateau(monde.rouge, ROUGE, 0, 0);
	monde.plateau[0][0].chateau = chateau;
	baron = ajouteAgent(chateau, BARON, monde.plateau);
	manant = ajouteAgent(chateau, MANANT, monde.plateau);
	chateau = ajouteChateau(monde.bleu, BLEU, NBLIG-1, NBCOL-1);
	monde.plateau[NBLIG-1][NBCOL-1].chateau = chateau;
	baron = ajouteAgent(chateau, BARON, monde.plateau);
	manant = ajouteAgent(chateau, MANANT, monde.plateau);

	affichePlateau(monde.plateau);

	/* Tours de jeu */
	/* Tirage au sort bleu/rouge */
	/* Production chateaux */
	/* Déplacement combattants */
	/* Production */
	/* Déplacement manants */

	/* Fin du jeu */

}
