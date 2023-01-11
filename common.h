#if !defined(_COMMON_H_)
#define _COMMON_H_

#include <stdio.h>
#include <semaphore.h> //sem*
#include <sys/mman.h> // mmap, munmap, shm_open, shm_unlink
#include <fcntl.h> // O_*
#include <stdlib.h> // exit
#include <unistd.h> // close, ftruncate
#include <string.h> // strcasecmp
#include <time.h> // time
#include <pthread.h> //threads
#include <ncurses.h>

const char* infonames[4];
const char* intentnames[4];
#define PLAYER_VISION 3

struct join_data_t{
    sem_t client_joining;
    sem_t server_sem;
    int slots[4];
};

struct server_data_t{
    sem_t client_joined; // sekcja krytyczna
    sem_t show_round;
    unsigned long round;
};

struct client_data_t{
    int infofd;
    int intentfd;
    struct client_info_t* info;
    struct client_intent_t* intent;
};


enum client_type{
    HUMAN,
    BEAST,
};

enum client_status{
    PENDING,
    ACTIVE,
    JOINED,
};

enum intent_type{
    UP,
    LEFT,
    RIGHT,
    DOWN,
    IDLE,
};

struct client_info_t{
    int x;
    int y;
    int client_num;
    enum client_type type;
    int gathered_coins;
    int stashed_coins;
    int deaths;
    enum intent_type bush_intent;
    char vision[2*PLAYER_VISION+2][2*PLAYER_VISION+2];
};

struct client_intent_t{
    enum intent_type intent;
    enum client_status status;
    sem_t client_sync;
    sem_t spawn_sync;
};

static void err(int c, const char* msg){
    if (!c)
        return;
    perror(msg);
    exit(1);
}

void print_client_labels(int x, int y);
void print_legend(int x, int y);
void print_client_info(struct client_info_t* client, int x, int y);

#endif // _COMMON_H_