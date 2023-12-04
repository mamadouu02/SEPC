#ifndef SYNCHRO_H
#define SYNCHRO_H

#include "ensitheora.h"
#include <stdbool.h>

extern bool fini;

/* Les extern des variables pour la synchro ici */

extern bool sent;
extern bool ready;
extern int n;

extern pthread_mutex_t mutex;
extern pthread_cond_t cond_taille;
extern pthread_cond_t cond_fenetre;
extern pthread_cond_t cond_cons;
extern pthread_cond_t cond_prod;

/* Fonctions de synchro à implanter */

void envoiTailleFenetre(th_ycbcr_buffer buffer);
void attendreTailleFenetre();

void attendreFenetreTexture();
void signalerFenetreEtTexturePrete();

void debutConsommerTexture();
void finConsommerTexture();

void debutDeposerTexture();
void finDeposerTexture();

#endif
