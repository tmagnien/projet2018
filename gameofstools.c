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
	if (newagent->asuiv != NULL) {
		newagent->asuiv->aprec = newagent;
	}
	newagent->aprec = agent;

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

void affichePlateau(Case plateau[NBLIG][NBCOL], int tour, char *clan, int tresor)
{
	int i, j;

	MLV_clear_window(MLV_COLOR_BLACK);

	for (i = 0; i < NBLIG; i++) {
		for (j = 0; j < NBCOL; j++) {
			afficheCase(plateau[i][j], i, j);
		}
	}
	/* Affichage informations */
	MLV_draw_text(CASE*(NBCOL + 1), CASE, "Tour %d du joueur %s", MLV_COLOR_WHITE, tour, clan);
	MLV_draw_text(CASE*(NBCOL + 1), CASE*2, "Trésor : %d", MLV_COLOR_WHITE, tresor);
	MLV_actualise_window();
}

int produireAgent(AListe chateau, int *tresor, char genre, int temps, int cout)
{
	/* Le chateau est-il déjà en train de produire ? */
	if (chateau->produit != LIBRE) {
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
	Agent *new_agent;

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

Agent *
resoudreCombat(Agent *agent, Agent *adversaire)
{
	int result_agent, result_adversaire;

	/* Tirage de 100 */
	/* Attaquant = baron ou guerrier */
	switch(agent->genre) {
		case BARON:
			result_agent = ((random() % 100) + 1) * CBARON;
			break;
		case GUERRIER:
			result_agent = ((random() % 100) + 1) * CGUERRIER;
			break;
	}
	/* Adversaire = chateau, baron, guerrier ou manant */
	switch(adversaire->genre) {
		case CHATEAU:
			result_adversaire = ((random() % 100) + 1) * CCHATEAU;
			break;
		case BARON:
			result_adversaire = ((random() % 100) + 1) * CBARON;
			break;
		case GUERRIER:
			result_adversaire = ((random() % 100) + 1) * CGUERRIER;
			break;
		case MANANT:
			result_adversaire = ((random() % 100) + 1) * CMANANT;
			break;
	}

	/* Comparaison */
	if (result_agent > result_adversaire) {
		return agent;
	}
	else {
		return adversaire;
	}
}

void changerAllegeance(Agent *manant, Agent *vainqueur)
{
	Agent *dernier_manant;

	/* Changer la couleur du clan du manant */
	manant->clan = vainqueur->clan;

	/* Trouver le dernier manant du clan vainqueur */
	dernier_manant = vainqueur;
	while (dernier_manant->asuiv != NULL) {
		dernier_manant = dernier_manant->asuiv;
	}

	/* Ajouter le manant */
	dernier_manant->asuiv = manant;
	manant->aprec = dernier_manant;
}

int deplacementCombattants(AListe chateau, Case plateau[NBLIG][NBCOL], int *tresor)
{
	int choix, deltax, deltay, newposx, newposy, x, y;
	char choix_chateau;
	Agent *agent, *suiv, *perdant;
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
			if (plateau[suiv->posx + deltax][suiv->posy + deltay].habitant != NULL || plateau[suiv->posx + deltax][suiv->posy + deltay].chateau != NULL) {
				/* La case destination est-elle occupée par un ennemi ? */
				if (plateau[suiv->posx][suiv->posy + deltay].habitant != NULL && plateau[suiv->posx][suiv->posy + deltay].habitant->clan != suiv->clan) {
					/* Combat contre un autre agent */
					perdant = resoudreCombat(suiv, plateau[suiv->posx][suiv->posy + deltay].habitant);
					if (perdant == suiv) {
						/* Attaquant a perdu => destruction et retrait de la liste chaînée */
						if (perdant->aprec != NULL) {
							perdant->aprec->asuiv = perdant->asuiv;
						}
						if (perdant->asuiv != NULL) {
							perdant->asuiv->aprec = perdant->aprec;
						}
						free(perdant);
						/* Plus rien à déplacer */
						continue;
					}
					else {
						/* Attaquant a gagné => destruction du perdant et déplacement sur la case cible */
						if (perdant->aprec != NULL) {
							perdant->aprec->asuiv = perdant->asuiv;
						}
						if (perdant->asuiv != NULL) {
							perdant->asuiv->aprec = perdant->aprec;
						}
						free(perdant);
					}
				}
				else if (plateau[suiv->posx][suiv->posy + deltay].chateau != NULL && plateau[suiv->posx][suiv->posy + deltay].chateau->clan != suiv->clan) {
					/* Combat contre un chateau */
					perdant = resoudreCombat(suiv, plateau[suiv->posx][suiv->posy + deltay].chateau);
					if (perdant == suiv) {
						/* Attaquant a perdu => destruction et retrait de la liste chaînée */
						if (perdant->aprec != NULL) {
							perdant->aprec->asuiv = perdant->asuiv;
						}
						if (perdant->asuiv != NULL) {
							perdant->asuiv->aprec = perdant->aprec;
						}
						free(perdant);
						/* Plus rien à déplacer */
						continue;
					}
					else {
						/* Attaquant a gagné */
						/* Destruction des barons et guerriers et changement d'allégeance des manants */
						Agent *ag, *agtemp;
						ag = perdant;
						while (ag != NULL) {
							switch (ag->genre) {
								case BARON:
								case GUERRIER:
									/* Destruction */
									agtemp = ag->asuiv;
									if (ag->aprec != NULL) {
										ag->aprec->asuiv = ag->asuiv;
									}
									if (ag->asuiv != NULL) {
										ag->asuiv->aprec = ag->aprec;
									}
									free(ag);
									ag = agtemp;
									break;
								case MANANT:
									/* Changement d'allégeance */
									changerAllegeance(ag, suiv);
									break;
							}
						}
						/* Destruction du chateau */
						if (perdant->vprec != NULL) {
							perdant->vprec->vsuiv = perdant->vsuiv;
						}
						if (perdant->vsuiv != NULL) {
							perdant->vsuiv->vprec = perdant->vprec;
						}
						free(perdant);
					}
				}
				else {
					/* On essaie de se déplacer sur un seul axe, au cas où */
					if (plateau[suiv->posx][suiv->posy + deltay].habitant == NULL && plateau[suiv->posx][suiv->posy + deltay].chateau == NULL) {
						newposx = suiv->posx;
						newposy = suiv->posy + deltay;
					}
					else if (plateau[suiv->posx + deltax][suiv->posy].habitant == NULL && plateau[suiv->posx + deltax][suiv->posy].chateau == NULL) {
						newposx = suiv->posx + deltax;
						newposy = suiv->posy;
					}
					else {
						/* Tant pis, on ne bouge pas à ce tour là */
						agent = suiv;
						continue;
					}
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
		MLV_draw_text(CASE*(NBCOL +1), CASE*3, "Agent %c en (%d,%d)", MLV_COLOR_WHITE, suiv->genre, suiv->posx, suiv->posy);
		MLV_draw_text_box(CASE*(NBCOL+1), CASE*5, CASE*5, CASE, "Destruction", 0, MLV_COLOR_WHITE, MLV_COLOR_WHITE, MLV_COLOR_BLACK, MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);
		MLV_draw_text_box(CASE*(NBCOL+1), CASE*6, CASE*5, CASE, "Déplacement", 0, MLV_COLOR_WHITE, MLV_COLOR_WHITE, MLV_COLOR_BLACK, MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);
		/*MLV_draw_text_box(CASE*(NBCOL+1), CASE*9, CASE*5, CASE, "Fin de partie", 0, MLV_COLOR_WHITE, MLV_COLOR_WHITE, MLV_COLOR_BLACK, MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);
		MLV_draw_text_box(CASE*(NBCOL+1), CASE*11, CASE*5, CASE, "Sauvegarde", 0, MLV_COLOR_WHITE, MLV_COLOR_WHITE, MLV_COLOR_BLACK, MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);
		MLV_draw_text_box(CASE*(NBCOL+1), CASE*12, CASE*5, CASE, "Chargement", 0, MLV_COLOR_WHITE, MLV_COLOR_WHITE, MLV_COLOR_BLACK, MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);*/
		MLV_actualise_window();
		choix = 0;
		while (choix == 0) {
			MLV_wait_mouse(&x, &y);
			if (x < CASE*(NBCOL+1) || x > CASE*(NBCOL+1) + CASE*5) {
				continue;
			}
			if (y >= CASE*5 && y < CASE*6) {
				choix = 1;
			}
			else if (y >= CASE*6 && y < CASE*7) {
				choix = 2;
			}
		}
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
				MLV_draw_text_box(CASE*(NBCOL+1), CASE*8, CASE*9, CASE, "Cliquez sur la destination", 0, MLV_COLOR_WHITE, MLV_COLOR_WHITE, MLV_COLOR_BLACK, MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);
				MLV_actualise_window();
				x = -1;
				y = -1;
				while (x < 0 || y < 0 || x > CASE*NBCOL || y > CASE*NBLIG) {
					MLV_wait_mouse(&x, &y);
				}
				/* Coordonnées x vs y */
				newposx = (y / CASE);
				newposy = (x / CASE);
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
					MLV_draw_text_box(CASE*(NBCOL+1), CASE*10, CASE*11, CASE, "Construire un nouveau chateau", 0, MLV_COLOR_WHITE, MLV_COLOR_WHITE, MLV_COLOR_BLACK, MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);
					MLV_draw_text_box(CASE*(NBCOL+1), CASE*11, CASE*12, CASE, "Ne rien faire", 0, MLV_COLOR_WHITE, MLV_COLOR_WHITE, MLV_COLOR_BLACK, MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);
					MLV_actualise_window();
					choix_chateau = 0;
					while (choix_chateau == 0) {
						MLV_wait_mouse(&x, &y);
						if (x < CASE*(NBCOL+1) || x > CASE*(NBCOL+1) + CASE*5) {
							continue;
						}
						if (y >= CASE*10 && y < CASE*11) {
							choix_chateau = 1;
						}
						else if (y >= CASE*11 && y < CASE*12) {
							choix_chateau = 2;
						}
					}
					if (choix_chateau == 1) {
						new_chateau = ajouteChateau(chateau->vsuiv, suiv->clan, suiv->posx, suiv->posy);
						new_chateau->vprec = chateau;
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
	int choix, deltax, deltay, newposx, newposy, x, y;
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
		MLV_draw_text(CASE*(NBCOL +1), CASE*3, "Agent %c en (%d,%d)", MLV_COLOR_WHITE, suiv->genre, suiv->posx, suiv->posy);
		MLV_draw_text_box(CASE*(NBCOL+1), CASE*5, CASE*5, CASE, "Destruction", 0, MLV_COLOR_WHITE, MLV_COLOR_WHITE, MLV_COLOR_BLACK, MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);
		MLV_draw_text_box(CASE*(NBCOL+1), CASE*6, CASE*5, CASE, "Déplacement", 0, MLV_COLOR_WHITE, MLV_COLOR_WHITE, MLV_COLOR_BLACK, MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);
		/*MLV_draw_text_box(CASE*(NBCOL+1), CASE*9, CASE*5, CASE, "Fin de partie", 0, MLV_COLOR_WHITE, MLV_COLOR_WHITE, MLV_COLOR_BLACK, MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);
		MLV_draw_text_box(CASE*(NBCOL+1), CASE*11, CASE*5, CASE, "Sauvegarde", 0, MLV_COLOR_WHITE, MLV_COLOR_WHITE, MLV_COLOR_BLACK, MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);
		MLV_draw_text_box(CASE*(NBCOL+1), CASE*12, CASE*5, CASE, "Chargement", 0, MLV_COLOR_WHITE, MLV_COLOR_WHITE, MLV_COLOR_BLACK, MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);*/
		MLV_actualise_window();
		choix = 0;
		while (choix == 0) {
			MLV_wait_mouse(&x, &y);
			if (x < CASE*(NBCOL+1) || x > CASE*(NBCOL+1) + CASE*5) {
				continue;
			}
			if (y >= CASE*5 && y < CASE*6) {
				choix = 1;
			}
			else if (y >= CASE*6 && y < CASE*7) {
				choix = 2;
			}
		}
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
				MLV_draw_text_box(CASE*(NBCOL+1), CASE*8, CASE*9, CASE, "Cliquez sur la destination", 0, MLV_COLOR_WHITE, MLV_COLOR_WHITE, MLV_COLOR_BLACK, MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);
				MLV_actualise_window();
				x = -1;
				y = -1;
				while (x < 0 || y < 0 || x > CASE*NBCOL || y > CASE*NBLIG) {
					MLV_wait_mouse(&x, &y);
				}
				/* Coordonnées x vs y */
				newposx = (y / CASE);
				newposy = (x / CASE);
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
				if (suiv->destx == suiv->posx && suiv->desty == suiv->posy) {
					/* Ne rien faire, production ou se transformer en guerrier */
					if (plateau[suiv->posx][suiv->posy].clan == suiv->clan) {
						MLV_draw_text_box(CASE*(NBCOL+1), CASE*10, CASE*11, CASE, "Produire", 0, MLV_COLOR_WHITE, MLV_COLOR_WHITE, MLV_COLOR_BLACK, MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);
					}
					MLV_draw_text_box(CASE*(NBCOL+1), CASE*11, CASE*12, CASE, "Transformation guerrier", 0, MLV_COLOR_WHITE, MLV_COLOR_WHITE, MLV_COLOR_BLACK, MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);
					MLV_draw_text_box(CASE*(NBCOL+1), CASE*12, CASE*13, CASE, "Ne rien faire", 0, MLV_COLOR_WHITE, MLV_COLOR_WHITE, MLV_COLOR_BLACK, MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);
					MLV_actualise_window();
					choix = 0;
					while (choix == 0) {
						MLV_wait_mouse(&x, &y);
						if (x < CASE*(NBCOL+1) || x > CASE*(NBCOL+1) + CASE*5) {
							continue;
						}
						if (plateau[suiv->posx][suiv->posy].clan == suiv->clan && y >= CASE*10 && y < CASE*11) {
							choix = 1;
						}
						else if (y >= CASE*11 && y < CASE*12) {
							choix = 2;
						}
						else if (y >= CASE*12 && y < CASE*13) {
							choix = 3;
						}
					}
					switch (choix) {
						case 1:
							/* Récolte */
							*tresor++;
							/* Définitivement immobile */
							suiv->destx = -1;
							suiv->desty = -1;
							break;
						case 2:
							/* Transformation en guerrier */
							suiv->genre = GUERRIER;
							/* Déplacement en fin de liste des guerriers */
							dernier_guerrier = chateau;
							/* On insère avant le premier manant ou en fin de liste */
							while (dernier_guerrier->asuiv != NULL && dernier_guerrier->asuiv->genre != MANANT) {
								dernier_guerrier = dernier_guerrier->asuiv;
							}
							/* On insère après le dernier guerrier */
							agent->asuiv = suiv->asuiv;
							if (suiv->asuiv != NULL) {
								suiv->asuiv->aprec = agent;
							}
							suiv->asuiv = dernier_guerrier->asuiv;
							dernier_guerrier->asuiv = suiv;
							suiv->aprec = dernier_guerrier;
							agent = agent->asuiv;
							continue;
							break;
						case 3:
							/* Ne rien faire */
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

	/* Initialisation cases */
	for (i = 0; i < NBLIG; i++) {
		for (j = 0; j < NBCOL; j++) {
			monde->plateau[i][j].chateau = NULL;
			monde->plateau[i][j].habitant = NULL;
			monde->plateau[i][j].clan = LIBRE;
		}
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
					agentRouge = chateauRouge;
				}
				else {
					agent = calloc(1, sizeof(Agent));
					sscanf(ligne, "%c%c%c %d %d %d %d %d", &agent->clan, &agent->genre, &agent->produit, &agent->temps, &agent->posx, &agent->posy, &agent->destx, &agent->desty);
					if (agent->genre == CHATEAU) {
						chateauRouge->vsuiv = agent;
						agent->vprec = chateauRouge;
						chateauRouge = chateauRouge->vsuiv;
						agentRouge = chateauRouge;
					}
					else {
						agentRouge->asuiv = agent;
						agent->aprec = agentRouge;
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
					chateauBleu = monde->bleu;
					agentBleu = chateauBleu;
				}
				else {
					agent = calloc(1, sizeof(Agent));
					sscanf(ligne, "%c%c%c %d %d %d %d %d", &agent->clan, &agent->genre, &agent->produit, &agent->temps, &agent->posx, &agent->posy, &agent->destx, &agent->desty);
					if (agent->genre == CHATEAU) {
						chateauBleu->vsuiv = agent;
						agent->vprec = chateauBleu;
						chateauBleu = chateauBleu->vsuiv;
						agentBleu = chateauBleu;
					}
					else {
						agentBleu->asuiv = agent;
						agent->aprec = agentBleu;
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
	choix = 0;
	while (choix == 0) {
		MLV_wait_mouse(&x, &y);
		if (x < CASE*(NBCOL+1) || x > CASE*(NBCOL+1) + CASE*5) {
			continue;
		}
		if (y >= CASE*5 && y < CASE*6) {
			choix = 1;
		}
		else if (y >= CASE*6 && y < CASE*7) {
			choix = 2;
		}
		else if (y >= CASE*7 && y < CASE*8) {
			choix = 3;
		}
		else if (y >= CASE*8 && y < CASE*9) {
			choix = 4;
		}
		else if (y >= CASE*9 && y < CASE*10) {
			choix = 5;
		}
		else if (y >= CASE*11 && y < CASE*12) {
			choix = 6;
		}
		else if (y >= CASE*12 && y < CASE*13) {
			choix = 7;
		}
	}

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
	affichePlateau(monde->plateau, monde->tour, clan == monde->rouge ? "Rouge" : "Bleu", *tresor);

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
		affichePlateau(monde->plateau, monde->tour, clan == monde->rouge ? "Rouge" : "Bleu", *tresor);
		deplacementCombattants(chateau, monde->plateau, tresor);
		chateau = chateau->vsuiv;
	}

	/* Déplacement et production manants */
	chateau = clan;
	while (chateau != NULL) {
		affichePlateau(monde->plateau, monde->tour, clan == monde->rouge ? "Rouge" : "Bleu", *tresor);
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
	chateau->vprec = monde->rouge;
	monde->rouge = chateau;
	monde->plateau[0][0].chateau = chateau;
	baron = ajouteAgent(chateau, BARON, monde->plateau);
	manant = ajouteAgent(chateau, MANANT, monde->plateau);
	chateau = ajouteChateau(monde->bleu, BLEU, NBLIG-1, NBCOL-1);
	chateau->vprec = monde->bleu;
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
