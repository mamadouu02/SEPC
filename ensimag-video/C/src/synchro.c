#include "ensitheora.h"
#include "synchro.h"

/* les variables pour la synchro, ici */

bool sent = false;
bool ready = false;
int n = 0;

pthread_cond_t cond_taille = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_texture = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_cons = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_prod = PTHREAD_COND_INITIALIZER;

/* l'implantation des fonctions de synchro ici */

void envoiTailleFenetre(th_ycbcr_buffer buffer) {
    pthread_mutex_lock(&mutex);
    windowsx = buffer[0].width;
    windowsy = buffer[0].height;
    sent = true;
    pthread_cond_signal(&cond_taille);
    pthread_mutex_unlock(&mutex);
}

void attendreTailleFenetre() {
    pthread_mutex_lock(&mutex);
    while (!sent) {
        pthread_cond_wait(&cond_taille, &mutex);
    }
    sent = false;
    pthread_mutex_unlock(&mutex);
}

void signalerFenetreEtTexturePrete() {
    pthread_mutex_lock(&mutex);
    ready = true;
    pthread_cond_signal(&cond_texture);
    pthread_mutex_unlock(&mutex);
}

void attendreFenetreTexture() {
    pthread_mutex_lock(&mutex);
    while (!ready) {
        pthread_cond_wait(&cond_texture, &mutex);
    }
    ready = false;
    pthread_mutex_unlock(&mutex);
}

void debutConsommerTexture() {
    pthread_mutex_lock(&mutex);
    while (n == 0) {
        pthread_cond_wait(&cond_cons, &mutex);
    }
    pthread_mutex_unlock(&mutex);
}

void finConsommerTexture() {
    pthread_mutex_lock(&mutex);
    n--;
    pthread_cond_signal(&cond_prod);
    pthread_mutex_unlock(&mutex);
}

void debutDeposerTexture() {
    pthread_mutex_lock(&mutex);
    while (n == NBTEX) {
        pthread_cond_wait(&cond_prod, &mutex);
    }
    pthread_mutex_unlock(&mutex);
}

void finDeposerTexture() {
    pthread_mutex_lock(&mutex);
    n++;
    pthread_cond_signal(&cond_cons);
    pthread_mutex_unlock(&mutex);
}
