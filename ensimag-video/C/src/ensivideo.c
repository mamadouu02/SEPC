#include <SDL2/SDL.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>

#include "oggstream.h"
#include "stream_common.h"

pthread_t theora_thread, vorbis_thread;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[]) {
  int res;

  if (argc != 2) {
    fprintf(stderr, "Usage: %s FILE", argv[0]);
    exit(EXIT_FAILURE);
  }
  assert(argc == 2);

  // Initialisation de la SDL
  res = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS);
  atexit(SDL_Quit);
  assert(res == 0);

  // Your code HERE
  // start the two stream readers (theoraStreamReader and vorbisStreamReader)
  // each in a thread
  pthread_create(&theora_thread, NULL, theoraStreamReader, &argv[1]);
  pthread_create(&vorbis_thread, NULL, vorbisStreamReader, &argv[1]);
  
  // wait for vorbis thread
  pthread_join(vorbis_thread, NULL);

  // 1 seconde of sound in advance, thus wait 1 seconde
  // before leaving
  sleep(1);

  // Wait for theora and theora2sdl threads
  pthread_cancel(theora_thread);
  pthread_cancel(theora2sdl_thread);
  pthread_join(theora_thread, NULL);
  pthread_join(theora2sdl_thread, NULL);

  // TODO
  /* liberer des choses ? */
  pthread_cancel(vorbis_thread);

  exit(EXIT_SUCCESS);
}
