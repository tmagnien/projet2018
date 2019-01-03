#include <MLV/MLV_all.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "gameofstools.h"

#define CASE 30

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
	chateau->produit = LIBRE;
	chateau->temps = 0;
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

void afficheCase(Case c, int i, int j)
{
	MLV_Color couleur;

	if (c.clan == ROUGE) {
		couleur = MLV_COLOR_RED;
	}
	else if (c.clan == BLEU) {
		couleur = MLV_COLOR_BLUE;
	}
	else {
		couleur = MLV_COLOR_WHITE;
	}
	MLV_draw_rectangle(j*CASE, i*CASE, CASE, CASE, couleur);
	if (c.chateau) {
		if (c.chateau->clan == ROUGE) {
			couleur = MLV_COLOR_RED;
		}
		else {
			couleur = MLV_COLOR_BLUE;
		}
		MLV_draw_text(j*CASE, i*CASE, "C", couleur);
	}
	if (c.habitant) {
		if (c.habitant->clan == ROUGE) {
			couleur = MLV_COLOR_RED;
		}
		else {
			couleur = MLV_COLOR_BLUE;
		}
		switch (c.habitant->genre) {
			case BARON:
				MLV_draw_text(j*CASE + CASE/2, i*CASE, "b", couleur);
				break;
			case GUERRIER:
				MLV_draw_text(j*CASE, i*CASE + CASE/2, "g", couleur);
				break;
			case MANANT:
				MLV_draw_text(j*CASE + CASE/2, i*CASE + CASE/2, "m", couleur);
				break;
		}
	}
}

void affichePlateau(Case plateau[NBLIG][NBCOL])
{
	int i, j;

	for (i = 0; i < NBLIG; i++) {
		for (j = 0; j < NBCOL; j++) {
			afficheCase(plateau[i][j], i, j);
		}
	}
	MLV_actualise_window();
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
	if (chateau->produit == LIBRE) {
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
			chateau->produit = LIBRE;
			return 1;
		}
	}

	return 0;
}

int deplacementCombattants(AListe chateau, Case plateau[NBLIG][NBCOL], int *tresor)
{
	int choix, deltax, deltay, newposx, newposy;
	char choix_chateau;
	Agent *agent, *suiv;
	AListe new_chateau;

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
				/* Gestion du sur-place pour les barons */
				if (suiv->genre == BARON && suiv->destx == suiv->posx && suiv->desty == suiv->posy && *tresor >= CCHATEAU) {
					/* Revendication de la case */
					printf("Construction d'un nouveau chateau (o/n) ?\n");
					scanf(" %c", &choix_chateau);
					if (choix_chateau == 'o' || choix_chateau == 'O') {
						new_chateau = ajouteChateau(chateau->vsuiv, suiv->clan, suiv->posx, suiv->posy);
						plateau[suiv->posx][suiv->posy].chateau = new_chateau;
					}
				}
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
	AListe chateau;
	Agent *agent;
	int i, j;

	/* Demande du nom de fichier */
	printf("Nom du fichier de sauvegarde : ");
	scanf("%s", nom_fichier);

	/* Ouverture du fichier */
	fp = fopen(nom_fichier, "w");
	if (fp == NULL) {
		printf("Impossible de créer le fichier %s\n", nom_fichier);
		return 0;
	}

	/* Première ligne */
	fprintf(fp, "%c %d %d %d\n", clan->clan, monde->tour, clan == monde->rouge ? monde->tresorRouge : monde->tresorBleu, clan == monde->rouge ? monde->tresorBleu : monde->tresorRouge);

	/* Chaque ligne représente les données d'un agent */
	/* L'odre n'a pas d'importance, commençons par le rouge */
	chateau = monde->rouge;
	while (chateau != NULL) {
		agent = chateau;
		while (agent != NULL) {
			fprintf(fp, "%c%c", agent->clan, agent->genre);
			if (agent->genre == CHATEAU && agent->produit != LIBRE) {
				fprintf(fp, "%c %d", agent->produit, agent->temps); 
			}
			else {
				fprintf(fp, "- 0");
			}
			/* Maintenant les coordonnées actuelles et destination */
			fprintf(fp, " %d %d %d %d\n", agent->posx, agent->posy, agent->destx, agent->desty);
			agent = agent->asuiv;
		}
		chateau = chateau->vsuiv;
	}
	/* Même chose pour le bleu */
	chateau = monde->bleu;
	while (chateau != NULL) {
		agent = chateau;
		while (agent != NULL) {
			fprintf(fp, "%c%c", agent->clan, agent->genre);
			if (agent->genre == CHATEAU && agent->produit != LIBRE) {
				fprintf(fp, "%c %d", agent->produit, agent->temps); 
			}
			else {
				fprintf(fp, "- 0");
			}
			/* Maintenant les coordonnées actuelles et destination */
			fprintf(fp, " %d %d %d %d\n", agent->posx, agent->posy, agent->destx, agent->desty);
			agent = agent->asuiv;
		}
		chateau = chateau->vsuiv;
	}

	/* Maintenant on sauvegarde les lignes du plateau */
	for (i = 0; i < NBLIG; i++) {
		for (j = 0; j < NBCOL; j++) {
			fprintf(fp, "%c", monde->plateau[i][j].clan);
		}
		fprintf(fp, "\n");
	}

	fclose(fp);
}

int chargementMonde(Monde *monde, AListe clan)
{
	char nom_fichier[80];
	char ligne[NBCOL+2];
	FILE *fp;
	Agent *agentRouge, *agentBleu, *chateauRouge, *chateauBleu, *agent;
	int tresor1, tresor2;
	int i, j;

	if (monde == NULL || clan == NULL) {
		/* Evitons une segfault */
		printf("Erreur : variables NULL\n");
		return 0;
	}
	/* On commence avec un monde vide */
	monde->rouge = NULL;
	monde->bleu = NULL;

	/* Demande du nom de fichier */
	printf("Nom du fichier de sauvegarde : ");
	scanf("%s", nom_fichier);

	/* Ouverture du fichier */
	fp = fopen(nom_fichier, "r");
	if (fp == NULL) {
		printf("Impossible de lire le fichier %s\n", nom_fichier);
		return 0;
	}

	/* Première ligne */
	fgets(ligne, NBCOL, fp);
	printf("%s\n", ligne);
	sscanf(ligne, "%c %d %d %d\n", &clan->clan, &monde->tour, &tresor1, &tresor2);
	if (clan->clan == ROUGE) {
		monde->tresorRouge = tresor1;
		monde->tresorBleu = tresor2;
	}
	else {
		monde->tresorRouge = tresor2;
		monde->tresorBleu = tresor1;
	}

	/* Chaque ligne représente les données d'un agent ou le tableau de jeu */
	/* Numéro de la ligne du plateau en cours de lecture */
	i = 0;
	while (fgets(ligne, NBCOL + 2, fp) != NULL) {
		printf("Ligne %d : %s", i, ligne);
		printf("Longueur de ligne : %ld\n", strlen(ligne));
		/* On teste la longueur de la ligne pour savoir si on est toujours en train de lire des agents */
		if (strlen(ligne) == (NBCOL+1)) {
			printf("On lit une ligne\n");
			/* On lit une ligne tu plateau */
			for (j = 0; j < NBCOL; j++) {
				monde->plateau[i][j].clan = ligne[j];
			}
			i++;
		}
		else {
			/* On lit les informations d'un agent */
			printf("On lit un agent\n");
			if (ligne[0] == ROUGE) {
				if (monde->rouge == NULL) {
					/* On crée le clan rouge */
					monde->rouge = calloc(1, sizeof(Agent));
					chateauRouge = monde->rouge;
					sscanf(ligne, "%c%c%c %d %d %d %d %d", &chateauRouge->clan, &chateauRouge->genre, &chateauRouge->produit, &chateauRouge->temps, &chateauRouge->posx, &chateauRouge->posy, &chateauRouge->destx, &chateauRouge->desty);
					printf("Agent %c%c%c\n", chateauRouge->clan, chateauRouge->genre, chateauRouge->produit);
					agentRouge = chateauRouge;
				}
				else {
					agent = calloc(1, sizeof(Agent));
					sscanf(ligne, "%c%c%c %d %d %d %d %d", &agent->clan, &agent->genre, &agent->produit, &agent->temps, &agent->posx, &agent->posy, &agent->destx, &agent->desty);
					printf("Agent %c%c%c\n", agent->clan, agent->genre, agent->produit);
					if (agent->genre == CHATEAU) {
						printf("Ajout chateau\n");
						chateauRouge->vsuiv = agent;
						chateauRouge = chateauRouge->vsuiv;
						agentRouge = chateauRouge;
					}
					else {
						agentRouge->asuiv = agent;
						agentRouge = agentRouge->asuiv;
					}
				}
				/* Ajoutons l'information dans le plateau */
				if (agentRouge->genre == CHATEAU) {
					monde->plateau[agentRouge->posx][agentRouge->posy].chateau = agentRouge;
				}
				else {
					monde->plateau[agentRouge->posx][agentRouge->posy].habitant = agentRouge;
				}
			}
			else {
				if (monde->bleu == NULL) {
					/* On crée le clan bleu */
					monde->bleu = calloc(1, sizeof(Agent));
					agentBleu = monde->bleu;
					sscanf(ligne, "%c%c%c %d %d %d %d %d", &agentBleu->clan, &agentBleu->genre, &agentBleu->produit, &agentBleu->temps, &agentBleu->posx, &agentBleu->posy, &agentBleu->destx, &agentBleu->desty);
				}
				else {
					agent = calloc(1, sizeof(Agent));
					sscanf(ligne, "%c%c%c %d %d %d %d %d", &agent->clan, &agent->genre, &agent->produit, &agent->temps, &agent->posx, &agent->posy, &agent->destx, &agent->desty);
					if (agent->genre == CHATEAU) {
						agentBleu->vsuiv = agent;
						agentBleu = agentBleu->vsuiv;
					}
					else {
						agentBleu->asuiv = agent;
						agentBleu = agentBleu->asuiv;
					}
				}
				/* Ajoutons l'information dans le plateau */
				if (agentBleu->genre == CHATEAU) {
					monde->plateau[agentBleu->posx][agentBleu->posy].chateau = agentBleu;
				}
				else {
					monde->plateau[agentBleu->posx][agentBleu->posy].habitant = agentBleu;
				}
			}
		}
	}

	fclose(fp);
	printf("Fin du chargement\n");
	return 1;
}

int choixProductionChateau(Agent *chateau, Monde *monde, int *tresor, AListe clan, int sauvegarde_chargement)
{
	int choix;
	int x, y;

	MLV_draw_text(CASE*(NBCOL +1), CASE*3, "Chateau %s en (%d,%d)", MLV_COLOR_WHITE, chateau->clan == ROUGE ? "Rouge" : "Bleu", chateau->posx, chateau->posy);
	MLV_draw_text_box(CASE*(NBCOL+1), CASE*5, CASE*5, CASE, "Attendre", 0, MLV_COLOR_WHITE, MLV_COLOR_WHITE, MLV_COLOR_BLACK, MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);
	MLV_draw_text_box(CASE*(NBCOL+1), CASE*6, CASE*5, CASE, "Produire baron", 0, MLV_COLOR_WHITE, MLV_COLOR_WHITE, MLV_COLOR_BLACK, MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);
	MLV_draw_text_box(CASE*(NBCOL+1), CASE*7, CASE*5, CASE, "Produire guerrier", 0, MLV_COLOR_WHITE, MLV_COLOR_WHITE, MLV_COLOR_BLACK, MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);
	MLV_draw_text_box(CASE*(NBCOL+1), CASE*8, CASE*5, CASE, "Produire manant", 0, MLV_COLOR_WHITE, MLV_COLOR_WHITE, MLV_COLOR_BLACK, MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);
	MLV_draw_text_box(CASE*(NBCOL+1), CASE*9, CASE*5, CASE, "Fin de partie", 0, MLV_COLOR_WHITE, MLV_COLOR_WHITE, MLV_COLOR_BLACK, MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);
	MLV_draw_text_box(CASE*(NBCOL+1), CASE*11, CASE*5, CASE, "Sauvegarde", 0, MLV_COLOR_WHITE, MLV_COLOR_WHITE, MLV_COLOR_BLACK, MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);
	MLV_draw_text_box(CASE*(NBCOL+1), CASE*12, CASE*5, CASE, "Chargement", 0, MLV_COLOR_WHITE, MLV_COLOR_WHITE, MLV_COLOR_BLACK, MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);
	MLV_actualise_window();
	MLV_wait_mouse(&x, &y);
	switch (choix) {
		case 1:
			/* Attendre */
			break;
		case 2:
			/* Produire Baron */
			produireBaron(chateau, tresor);
			break;
		case 3:
			/* Produire Guerrier */
			produireGuerrier(chateau, tresor);
			break;
		case 4:
			/* Produire Manant */
			produireManant(chateau, tresor);
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
				/* Valeur spécifique pour repartir depuis la sauvegarde */
				return -1;
			}
			break;
	}
	return 1;
}

int tourDeJeuClan(Monde *monde, AListe clan, int *tresor, int sauvegarde_chargement)
{
	int i, j, choix, ret;
	Agent *agent, *chateau;

	/* Affichage plateau */
	affichePlateau(monde->plateau);

	/* Affichage informations */
	MLV_draw_text(CASE*(NBCOL + 1), CASE, "Tour %d du joueur %s", MLV_COLOR_WHITE, monde->tour, clan == monde->rouge ? "Rouge" : "Bleu");
	MLV_draw_text(CASE*(NBCOL + 1), CASE*2, "Trésor : %d", MLV_COLOR_WHITE, *tresor);
	MLV_actualise_window();

	/* Production chateau */
	chateau = clan;
	while (chateau != NULL) {
		/* Production du chateau */
		ret = choixProductionChateau(chateau, monde, tresor, clan, sauvegarde_chargement);
		if (ret <= 0) {
			return ret;
		}
		chateau = chateau->vsuiv;
	}
	
	/* Déplacement et production combattants */
	chateau = clan;
	while (chateau != NULL) {
		deplacementCombattants(chateau, monde->plateau, tresor);
		chateau = chateau->vsuiv;
	}

	/* Déplacement et production manants */
	chateau = clan;
	while (chateau != NULL) {
		deplacementManants(chateau, monde->plateau, tresor);
		chateau = chateau->vsuiv;
	}

	return 1;
}

int main(int argc, char *argv[])
{
	Monde *monde, *newmonde;
	Agent *chateau, *baron, *manant;
	AListe clan, newclan;
	int i, j, choix, sauvegarde_chargement, ret, tour;

	/* Mise en place */
	monde = malloc(sizeof(Monde));
	monde->tour = 0;
	monde->tresorRouge = 50;
	monde->tresorBleu = 50;

	/* Toutes les cases sont neutres et vides */
	for (i = 0; i < NBLIG; i++) {
		for (j = 0; j < NBCOL; j++) {
			monde->plateau[i][j].clan = LIBRE;
			monde->plateau[i][j].chateau = NULL;
			monde->plateau[i][j].habitant = NULL;
		}
	}

	/* Initialisation des listes d'agents */
	chateau = ajouteChateau(monde->rouge, ROUGE, 0, 0);
	monde->rouge = chateau;
	monde->plateau[0][0].chateau = chateau;
	baron = ajouteAgent(chateau, BARON, monde->plateau);
	manant = ajouteAgent(chateau, MANANT, monde->plateau);
	chateau = ajouteChateau(monde->bleu, BLEU, NBLIG-1, NBCOL-1);
	monde->bleu = chateau;
	monde->plateau[NBLIG-1][NBCOL-1].chateau = chateau;
	baron = ajouteAgent(chateau, BARON, monde->plateau);
	manant = ajouteAgent(chateau, MANANT, monde->plateau);

	/* Initialisation random */
	srandom(time(NULL));

	/* Pour le chargement */
	newclan = NULL;

	/* Fenêtre graphique */
	MLV_create_window("Game of stools", "Game of stools", CASE*(NBCOL + 10), CASE*(NBLIG+5));

	/* Tours de jeu */
	while (1) {
		/* Un tour de jeu en plus */
		monde->tour++;

		/* Affichage du plateau */
		affichePlateau(monde->plateau);

		/* Production des chateaux */
		productionChateau(monde->rouge, monde->plateau);
		productionChateau(monde->bleu, monde->plateau);

		/* Tirage au sort bleu/rouge (forcé si chargement) */
		if (newclan != NULL) {
			if (newclan->clan == ROUGE) {
				tour = 0;
			}
			else {
				tour = 1;
			}
		}
		else {
			tour = random() % 2;
		}
		if (tour == 0) {
			/* Rouge */
			ret = tourDeJeuClan(monde, monde->rouge, &monde->tresorRouge, sauvegarde_chargement);
			if (ret == 0) {
				/* Erreur */
				return -1;
			}
			if (ret == -1) {
				/* Chargement ==> on passe directement au tour suivant */
				newmonde = malloc(sizeof(Monde));
				newclan = malloc(sizeof(AListe));
				if (chargementMonde(newmonde, newclan) != 0) {
					monde = newmonde;
					continue;
				}
			}
			/* Bleu */
			ret = tourDeJeuClan(monde, monde->bleu, &monde->tresorBleu, 0);
			if (ret == 0) {
				/* Erreur */
				return -1;
			}
		}
		else {
			/* Bleu */
			ret = tourDeJeuClan(monde, monde->bleu, &monde->tresorBleu, sauvegarde_chargement);
			if (ret == 0) {
				/* Erreur */
				return -1;
			}
			if (ret == -1) {
				/* Chargement ==> on passe directement au tour suivant */
				newmonde = malloc(sizeof(Monde));
				newclan = malloc(sizeof(AListe));
				if (chargementMonde(newmonde, newclan) != 0) {
					monde = newmonde;
					continue;
				}
			}
			/* Rouge */
			ret = tourDeJeuClan(monde, monde->rouge, &monde->tresorRouge, 0);
			if (ret == 0) {
				/* Erreur */
				return -1;
			}
		}
	}

	/* Fin du jeu */

}
