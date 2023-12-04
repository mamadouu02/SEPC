#include "ensitheora.h"
#include "synchro.h"

bool fini = true;

/* les variables pour la synchro, ici */

pthread_cond_t cond_fenetre = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_taille = PTHREAD_COND_INITIALIZER;

/* l'implantation des fonctions de synchro ici */

void envoiTailleFenetre(th_ycbcr_buffer buffer) {
    pthread_mutex_lock(&mutex);
    fini = false;
    windowsx = buffer[0].width;
    windowsy = buffer[0].height;
    fini = true;
    pthread_cond_signal(&cond_taille);
    pthread_mutex_unlock(&mutex);
}

void attendreTailleFenetre() {
    pthread_mutex_lock(&mutex);
    while (!fini) {
        pthread_cond_wait(&cond_taille, &mutex);
    }
    pthread_mutex_unlock(&mutex);
}

void signalerFenetreEtTexturePrete() {
    pthread_cond_signal(&cond_fenetre);
}

void attendreFenetreTexture() {
    pthread_cond_wait(&cond_fenetre, &mutex);
}

void debutConsommerTexture() {}

void finConsommerTexture() {}

void debutDeposerTexture() {}

void finDeposerTexture() {}
