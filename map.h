#if !defined(_MAP_H_)
#define _MAP_H_

#include "common.h"

#define MAX_DIMENSION 128
#define BEAST_VISION 6

enum terrain_type{
    GROUND,
    WALL,
    BUSH,
    CAMP,
};

enum coin_type{
    SMALL,
    MEDIUM,
    LARGE,
    DROPPED,
    NIL,
};

struct coin_t{
    enum coin_type type;
    int coin_amount;
};

struct field_t{
    enum terrain_type terrain;
    struct coin_t coin;
    struct client_data_t* actor;
    int x;
    int y;
};

struct map_t{
    int width;
    int height;
    struct field_t** fields;
};

struct beast_t{
    struct client_data_t *data;
    pthread_t tid;
};

void update_vision(struct map_t* map, struct client_data_t* client);
void add_treasure(struct map_t* map, char input);
void add_beast(struct map_t* map, struct beast_t **beasts, int *num);
void* move_beast(void* arg);
struct map_t* load_map(char* fname);
void display_map(struct map_t* map);
struct field_t* random_empty_field(struct map_t* map);
void calc_coords(int *x, int *y, enum intent_type intent);
void place_player(struct map_t* map, struct client_data_t* client_data);
void handle_player_move(struct map_t *map, struct client_data_t* client_data);
#endif