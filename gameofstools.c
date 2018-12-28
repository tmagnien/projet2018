#include <MLV/MLV_all.h>
#include <stdlib.h>
#include <stdio.h>
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
	chateau->temps = -1;
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
	int i, j, posx, posy;

	agent = chateau;

	/* On vérifie si une case libre existe autour du chateau */
	posx = -1;
	posy = -1;
	for (i = chateau->posx - 1 ; i <= chateau->posx + 1; i++) {
		if (i < 0 || i >= NBLIG) {
			continue;
		}
		for (j = chateau->posy - 1 ; j <= chateau->posy + 1; j++) {
			if (j < 0 || j >= NBCOL) {
				continue;
			}
			if (plateau[i][j].chateau == NULL && plateau[i][j].habitant == NULL) {
				posx = i;
				posy = j;
				break;
			}
		}
		if (posx != -1) {
			break;
		}
	}

	/* Pas de place */
	if (posx == -1 || posy == -1) {
		return NULL;
	}

	/* On ajoute l'agent juste avant le premier élément de type "genre" */
	while (agent->asuiv != NULL && agent->asuiv->genre != genre) {
		agent = agent->asuiv;
	}

	/* On est sur le premier agent de genre "genre" */
	newagent = malloc(sizeof(Agent));
	newagent->clan = chateau->clan;
	newagent->genre = genre;

	/* Case libre voisine */
	newagent->posx = posx;
	newagent->posy = posy;

	/* Destination = position actuelle */
	newagent->destx = newagent->posx;
	newagent->desty = newagent->posy;

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
				printf("%cC000", plateau[i][j].chateau->clan);
			}
			else if (plateau[i][j].habitant != NULL) {
				printf("%c ", plateau[i][j].habitant->clan);
				if (plateau[i][j].habitant->genre == BARON) {
					printf("100");
				}
				else if (plateau[i][j].habitant->genre == GUERRIER) {
					printf("010");
				}
				else if (plateau[i][j].habitant->genre == MANANT) {
					printf("001");
				}
			}
			else if (plateau[i][j].clan != LIBRE) {
				printf("  %c  ", plateau[i][j].clan);
			}
			else {
				printf("     ");
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

int produireAgent(AListe chateau, int *tresor, char genre, int temps, int cout)
{
	/* Le chateau est-il déjà en train de produire ? */
	if (chateau->temps >= 0) {
		printf("Production en cours.\n");
		return 0;
	}

	/* On vérifie que le trésor du clan est suffisant */
	if (cout > *tresor) {
		printf("Trésor insuffisant.\n");
		return 0;
	}

	/* On produit */
	*tresor -= cout;
	chateau->produit = genre;
	chateau->temps = temps;

	return 1;
}

int produireBaron(AListe chateau, int *tresor)
{
	return produireAgent(chateau, tresor, BARON, TBARON, CBARON);
}

int produireGuerrier(AListe chateau, int *tresor)
{
	return produireAgent(chateau, tresor, GUERRIER, TGUERRIER, CGUERRIER);
}

int produireManant(AListe chateau, int *tresor)
{
	return produireAgent(chateau, tresor, MANANT, TMANANT, CMANANT);
}

int productionChateau(AListe chateau, Case plateau[NBLIG][NBCOL])
{
	/* Production en cours ? */
	if (chateau->produit < 0) {
		return 0;
	}

	/* On décrémente d'un tour */
	if (chateau->temps > 0) {
		--chateau->temps;
		return 0;
	}

	/* Si le temps est écoulé, on produit l'agent */
	if (chateau->temps == 0) {
		/* Si on a pu produire l'agent, la production est finie */
		if (ajouteAgent(chateau, chateau->produit, plateau) != NULL) {
			chateau->temps = -1;
			return 1;
		}
	}

	return 0;
}

int deplacementCombattants(AListe chateau, Case plateau[NBLIG][NBCOL])
{
	int choix, deltax, deltay, newposx, newposy;
	Agent *agent, *suiv;

	agent = chateau;
	while(agent != NULL && agent->asuiv != NULL) {
		suiv = agent->asuiv;
		/* Seuls les combattants se déplacent pour le moment */
		if (suiv->genre == CHATEAU || suiv->genre == MANANT) {
			agent = suiv;
			continue;
		}
		/* Déplacement en cours ? */
		if (suiv->posx != suiv->destx || suiv->posy != suiv->desty) {
			/* Déplacer l'agent */
			if (suiv->destx > suiv->posx) {
				deltax = 1;
				newposx = suiv->posx + 1;
			}
			else if (suiv->destx < suiv->posx) {
				deltax = -1;
				newposx = suiv->posx - 1;
			}
			else {
				deltax = 0;
				newposx = suiv->posx;
			}
			if (suiv->desty > suiv->posy) {
				deltay = 1;
				newposy = suiv->posy + 1;
			}
			else if (suiv->desty < suiv->posy) {
				deltay = -1;
				newposy = suiv->posy - 1;
			}
			else {
				deltay = 0;
				newposy = suiv->posy;
			}
			/* Gestion case destination occupée */
			if (plateau[suiv->posx + deltax][suiv->posy + deltay].habitant != NULL) {
				/* On essaie de se déplacer sur un seul axe, au cas où */
				if (plateau[suiv->posx][suiv->posy + deltay].habitant == NULL) {
					newposx = suiv->posx;
					newposy = suiv->posy + deltay;
				}
				else if (plateau[suiv->posx + deltax][suiv->posy].habitant == NULL) {
					newposx = suiv->posx + deltax;
					newposy = suiv->posy;
				}
				else {
					/* Tant pis, on ne bouge pas à ce tour là */
					agent = suiv;
					continue;
				}
			}
			/* Mise à jour plateau et coordonnées */
			plateau[suiv->posx][suiv->posy].habitant = NULL;
			suiv->posx = newposx;
			suiv->posy = newposy;
			plateau[suiv->posx][suiv->posy].habitant = suiv;
			/* Suivant */
			agent = suiv;
			continue;
		}
		/* Choix de la destination */
		printf("Agent %c en position (%d,%d), votre choix :\n", suiv->genre, suiv->posx, suiv->posy);
		printf("1 . Destruction\n2 . Déplacement\n");
		scanf("%d", &choix);
		switch (choix) {
			case 1:
				/* Destruction */
				/* Retrait du plateau */
				plateau[suiv->posx][suiv->posy].habitant = NULL;
				/* Suppression de la liste et libération de la mémoire */
				agent->asuiv = suiv->asuiv;
				free(suiv);
				suiv = agent->asuiv;
				break;
			case 2:
				/* Déplacement */
				printf("Destination x,y :\n");
				scanf("%d,%d", &newposx, &newposy);
				/* On ramène à des coordonnées valides */
				if (newposx < 0) {
					newposx = 0;
				}
				if (newposx > NBLIG - 1) {
					newposx = NBLIG - 1;
				}
				if (newposy < 0) {
					newposy = 0;
				}
				if (newposy > NBCOL - 1) {
					newposy = NBCOL - 1;
				}
				suiv->destx = newposx;
				suiv->desty = newposy;
				/* Gestion du sur-place pour les guerriers */
				if (suiv->genre == GUERRIER && suiv->destx == suiv->posx && suiv->desty == suiv->posy) {
					/* Revendication de la case */
					plateau[suiv->posx][suiv->posy].clan = suiv->clan;
				}
				break;
		}
		agent = suiv;
	}
}

int deplacementManants(AListe chateau, Case plateau[NBLIG][NBCOL], int *tresor)
{
	int choix, deltax, deltay, newposx, newposy;
	Agent *agent, *suiv, *dernier_guerrier;

	agent = chateau;
	while(agent != NULL && agent->asuiv != NULL) {
		suiv = agent->asuiv;
		/* Seuls les manants se déplacent pour le moment */
		if (suiv->genre != MANANT) {
			agent = suiv;
			continue;
		}
		/* Manant immobile ? */
		if (suiv->destx == -1 && suiv->desty == -1) {
			/* Récolte */
			*tresor++;
			agent = suiv;
			continue;
		}
		/* Déplacement en cours ? */
		if (suiv->posx != suiv->destx || suiv->posy != suiv->desty) {
			/* Déplacer l'agent */
			if (suiv->destx > suiv->posx) {
				deltax = 1;
				newposx = suiv->posx + 1;
			}
			else if (suiv->destx < suiv->posx) {
				deltax = -1;
				newposx = suiv->posx - 1;
			}
			else {
				deltax = 0;
				newposx = suiv->posx;
			}
			if (suiv->desty > suiv->posy) {
				deltay = 1;
				newposy = suiv->posy + 1;
			}
			else if (suiv->desty < suiv->posy) {
				deltay = -1;
				newposy = suiv->posy - 1;
			}
			else {
				deltay = 0;
				newposy = suiv->posy;
			}
			/* Gestion case destination occupée */
			if (plateau[suiv->posx + deltax][suiv->posy + deltay].habitant != NULL) {
				/* On essaie de se déplacer sur un seul axe, au cas où */
				if (plateau[suiv->posx][suiv->posy + deltay].habitant == NULL) {
					newposx = suiv->posx;
					newposy = suiv->posy + deltay;
				}
				else if (plateau[suiv->posx + deltax][suiv->posy].habitant == NULL) {
					newposx = suiv->posx + deltax;
					newposy = suiv->posy;
				}
				else {
					/* Tant pis, on ne bouge pas à ce tour là */
					agent = suiv;
					continue;
				}
			}
			/* Mise à jour plateau et coordonnées */
			plateau[suiv->posx][suiv->posy].habitant = NULL;
			suiv->posx = newposx;
			suiv->posy = newposy;
			plateau[suiv->posx][suiv->posy].habitant = suiv;
			/* Suivant */
			agent = suiv;
			continue;
		}
		/* Choix de la destination */
		printf("Agent %c en position (%d,%d), votre choix :\n", suiv->genre, suiv->posx, suiv->posy);
		printf("1 . Destruction\n2 . Déplacement\n");
		scanf("%d", &choix);
		switch (choix) {
			case 1:
				/* Destruction */
				/* Retrait du plateau */
				plateau[suiv->posx][suiv->posy].habitant = NULL;
				/* Suppression de la liste et libération de la mémoire */
				agent->asuiv = suiv->asuiv;
				free(suiv);
				suiv = agent->asuiv;
				break;
			case 2:
				/* Déplacement */
				printf("Destination x,y :\n");
				scanf("%d,%d", &newposx, &newposy);
				/* On ramène à des coordonnées valides */
				if (newposx < 0) {
					newposx = 0;
				}
				if (newposx > NBLIG - 1) {
					newposx = NBLIG - 1;
				}
				if (newposy < 0) {
					newposy = 0;
				}
				if (newposy > NBCOL - 1) {
					newposy = NBCOL - 1;
				}
				suiv->destx = newposx;
				suiv->desty = newposy;
				/* Gestion du sur-place pour les manants */
				if (plateau[suiv->posx][suiv->posy].clan == suiv->clan && suiv->destx == suiv->posx && suiv->desty == suiv->posy) {
					/* Ne rien faire, production ou se transformer en guerrier */
					printf("Sur-place, votre choix :\n");
					printf("1 . Ne rien faire\n2 . Produire\n3 . Se transformer en guerrier\n");
					scanf("%d", &choix);
					switch (choix) {
						case 1:
							/* Ne rien faire */
							break;
						case 2:
							/* Récolte */
							*tresor++;
							/* Définitivement immobile */
							suiv->destx = -1;
							suiv->desty = -1;
							break;
						case 3:
							/* Transformation en guerrier */
							suiv->genre = GUERRIER;
							/* Déplacement en fin de liste des guerriers */
							dernier_guerrier = chateau;
							/* On insère avant le premier manant ou en fin de liste */
							while (dernier_guerrier->asuiv != NULL && dernier_guerrier->asuiv->genre != MANANT) {
								dernier_guerrier = dernier_guerrier->asuiv;
							}
							/* On insère après l'élément courant */
							agent->asuiv = suiv->asuiv;
							suiv->asuiv = dernier_guerrier->asuiv;
							dernier_guerrier->asuiv = suiv;
							break;
					}
				}
				break;
		}
		agent = suiv;
	}
}

int sauvegardeMonde(Monde *monde, AListe clan)
{
	char nom_fichier[80];
	FILE *fp;

	/* Demande du nom de fichier */
	printf("Nom du fichier de sauvegarde : ");
	scanf("%s", &nom_fichier);

	/* Ouverture du fichier */
	fp = fopen(nom_fichier, "w");
	if (fp == NULL) {
		printf("Impossible de créer le fichier %s\n", nom_fichier);
		return 0;
	}

	fwrite(&clan->clan, sizeof(clan->clan), 1, fp);
	fwrite(" ", sizeof(char), 1, fp);
	fwrite(&monde->tour, sizeof(monde->tour), 1, fp);
	fwrite(" ", sizeof(char), 1, fp);
	if (clan == monde->rouge) {
		fwrite(&monde->tresorRouge, sizeof(monde->tresorRouge), 1, fp);
		fwrite(" ", sizeof(char), 1, fp);
		fwrite(&monde->tresorBleu, sizeof(monde->tresorBleu), 1, fp);
	}
	else {
		fwrite(&monde->tresorBleu, sizeof(monde->tresorBleu), 1, fp);
		fwrite(" ", sizeof(char), 1, fp);
		fwrite(&monde->tresorRouge, sizeof(monde->tresorRouge), 1, fp);
	}
	fwrite("\n", sizeof(char), 1, fp);
	fclose(fp);
}

int chargementMonde(Monde *monde)
{
}

int tourDeJeuClan(Monde *monde, AListe clan, int *tresor, int sauvegarde_chargement)
{
	int i, j, choix;

	/* Affichage plateau */
	affichePlateau(monde->plateau);
	printf("Tour %d du joueur ", monde->tour);
	printf(clan == monde->rouge ? "Rouge\n" : "Bleu\n");
	printf("Trésor : ");
	printf("%d\n", *tresor);

	/* Affichage du clan */
	printf("\nClan :\n");
	Agent *agent = clan;
	while (agent != NULL) {
		printf("- %c (%d,%d)\n", agent->genre, agent->posx, agent->posy);
		agent = agent->asuiv;
	}

	/* Affichage des cases occupées */
	printf("\nCases occupées :\n");
	for (i = 0; i < NBLIG; i++) {
		for (j = 0; j < NBCOL; j++) {
			if (monde->plateau[i][j].chateau != NULL) {
				printf("(%d,%d) %c C\n", i, j, monde->plateau[i][j].chateau->clan);
			}
			if (monde->plateau[i][j].habitant != NULL) {
				printf("(%d,%d) %c %c\n", i, j, monde->plateau[i][j].habitant->clan, monde->plateau[i][j].habitant->genre);
			}
		}
	}

	/* Production chateau */
	printf("\nChateau %s en (%d,%d), quel ordre ?\n", clan == monde->rouge ? "Rouge" : "Bleu", clan->posx, clan->posy);
	printf("\n1 . Attendre\n2 . Produire Baron\n3 . Produire Guerrier\n4 . Produire Manant\n5 . Fin de partie\n");
	if (sauvegarde_chargement) {
		printf("6 . Sauvegarde\n7 . Chargement\n");
	}
	scanf("%d", &choix);
	switch (choix) {
		case 1:
			/* Attendre */
			break;
		case 2:
			/* Produire Baron */
			produireBaron(clan, tresor);
			break;
		case 3:
			/* Produire Guerrier */
			produireGuerrier(clan, tresor);
			break;
		case 4:
			/* Produire Manant */
			produireManant(clan, tresor);
			break;
		case 5:
			/* Fin de partie */
			return 0;
		case 6:
			/* Sauvegarde */
			if (sauvegarde_chargement) {
				sauvegardeMonde(monde, clan);
			}
			break;
		case 7:
			/* Chargement */
			if (sauvegarde_chargement) {
				chargementMonde(monde);
				/* Valeur spécifique pour repartir depuis la sauvegarde */
				return -1;
			}
			break;
	}
	
	/* Déplacement combattants */
	deplacementCombattants(clan, monde->plateau);

	/* Déplacement et production manants */
	deplacementManants(clan, monde->plateau, tresor);

	return 1;
}

int main(int argc, char *argv[])
{
	Monde monde;
	Agent *chateau, *baron, *manant;
	AListe clan;
	int i, j, choix, sauvegarde_chargement;

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
	monde.rouge = chateau;
	monde.plateau[0][0].chateau = chateau;
	baron = ajouteAgent(chateau, BARON, monde.plateau);
	manant = ajouteAgent(chateau, MANANT, monde.plateau);
	chateau = ajouteChateau(monde.bleu, BLEU, NBLIG-1, NBCOL-1);
	monde.bleu = chateau;
	monde.plateau[NBLIG-1][NBCOL-1].chateau = chateau;
	baron = ajouteAgent(chateau, BARON, monde.plateau);
	manant = ajouteAgent(chateau, MANANT, monde.plateau);

	/* Initialisation random */
	srandom(time(NULL));

	/* Tours de jeu */
	while (1) {
		/* Demande de sauvegarde ou de chargement tous les 5 tours */
		if (monde.tour % 5 == 0) {
			sauvegarde_chargement = 1;
		}
		else {
			sauvegarde_chargement = 0;
		}

		/* Un tour de jeu en plus */
		monde.tour++;

		/* Production des chateaux */
		productionChateau(monde.rouge, monde.plateau);
		productionChateau(monde.bleu, monde.plateau);

		/* Tirage au sort bleu/rouge */
		if (random() % 2 == 0) {
			/* Rouge */
			if (!tourDeJeuClan(&monde, monde.rouge, &monde.tresorRouge, sauvegarde_chargement)) {
				return -1;
			}
			/* Bleu */
			if (!tourDeJeuClan(&monde, monde.bleu, &monde.tresorBleu, 0)) {
				return -1;
			}
		}
		else {
			/* Bleu */
			if (!tourDeJeuClan(&monde, monde.bleu, &monde.tresorBleu, sauvegarde_chargement)) {
				return -1;
			}
			/* Rouge */
			if (!tourDeJeuClan(&monde, monde.rouge, &monde.tresorRouge, 0)) {
				return -1;
			}
		}
	}

	/* Fin du jeu */

}
