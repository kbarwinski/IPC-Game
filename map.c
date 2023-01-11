#include "map.h"

struct coin_t no_coin = {
    .type = NIL,
    .coin_amount = 0,
};

struct coin_t small = {
    .type = SMALL,
    .coin_amount = 1,
};

struct coin_t medium = {
    .type = MEDIUM,
    .coin_amount = 5,
};

struct coin_t large = {
    .type = LARGE,
    .coin_amount = 50,
};

char terrain_chars[4] = {'.','|','#','A'};
char coin_chars[4] = {'c','t','T','('};

struct join_data_t* jdata;
struct client_data_t clients[4];
struct beast_t* beasts;
struct map_t* map; 

struct map_t* load_map(char* fname){
    FILE *mapfile  = fopen(fname, "r");
    
    if(mapfile == NULL){
        return NULL;
    }
    
    int width, height;
    
    char dimensionbuff[20];
    fgets(dimensionbuff,20,mapfile);
    
    sscanf(dimensionbuff,"%dW %dH",&width,&height);
    
    struct field_t** map = (struct field_t**)malloc(sizeof(struct field_t*) * height);

    char buff[MAX_DIMENSION+1];
    
    for(int i=0; i<height; i++){
        *(map+i) = (struct field_t*)malloc(sizeof(struct field_t) * width + 1);
        
        fgets(buff,MAX_DIMENSION+3,mapfile);

        for(int j=0; j<width; j++){
            struct field_t *field = (*(map+i)+j);
            
            field->terrain = GROUND;
            field->coin = no_coin;
            field->actor = NULL;
            
            field->x = j;
            field->y = i;
            
            switch(buff[j]){
                case '|':
                    field->terrain = WALL;
                break;
                case '#':   
                    field->terrain = BUSH;
                break;
                case 'A':
                    field->terrain = CAMP;
                break;
                case 'c':
                    field->coin = small;
                break;
                case 't':
                    field->coin = medium;
                break;
                case 'T':
                    field->coin = large;
                break;
                default:
                break;
            }
        }
    }
    
    fclose(mapfile);
    
    struct map_t *res = (struct map_t*)malloc(sizeof(struct map_t));
    res->height = height;
    res->width = width;
    res->fields = map;
    
    return res;
}

void display_map(struct map_t* map){
            printw("\n");
    for(int i=0; i<map->height; i++){
        for(int j=0; j<map->width; j++){
            
            struct field_t* field = *(map->fields+i)+j;
            
            if(field->actor!=NULL){
                if(field->actor->info->type == HUMAN)
                    printw("%c",'0' + field->actor->info->client_num);
                else
                    printw("B");
            }
            else if(field->coin.type != NIL){
                printw("%c",coin_chars[field->coin.type]);
            }
            else{
                printw("%c",terrain_chars[field->terrain]);
            }
        }
        printw("\n");
    }
}


struct field_t* random_empty_field(struct map_t* map){

    while(1){
        int randx = rand() % map->width;
        int randy = rand() % map->height;
        struct field_t* res = *(map->fields+randy)+randx;
        if(res->terrain == GROUND && res->actor == NULL)
            return res;
    }
}

void add_treasure(struct map_t* map, char input){
    struct field_t* random_field = random_empty_field(map);
    struct coin_t to_add;
    switch(input){
    case 'c':
        to_add = small;
        break;
    case 't':
        to_add = medium;
        break;
    case 'T':
        to_add = large;
        break;
    }
    random_field->coin = to_add;
}

void place_player(struct map_t* map, struct client_data_t* client_data){
    struct field_t* random_field = random_empty_field(map);
    struct client_info_t* client = client_data->info;
    client->x = random_field->x;
    client->y = random_field->y;
    client->gathered_coins = 0;

    random_field->actor = client_data;
}


void calc_coords(int *x, int *y, enum intent_type intent){
    switch(intent){
        case IDLE:
            return;
        case UP:
            *y-=1;
            break;
        case DOWN:
            *y+=1;
            break;
        case LEFT:
            *x-=1; 
            break;
        case RIGHT:
            *x+=1;
            break;
    }
}

void handle_player_move(struct map_t* map, struct client_data_t* client_data){
    
    struct client_info_t* client = client_data->info;
    enum intent_type intent = client_data->intent->intent;
    
    int x = client->x;
    int y = client->y;
    
    calc_coords(&x, &y, intent);

    if(x>=map->width || x<0 || y<0 || y>=map->height)
        return;
    
    struct field_t *current, *target;
    current = *(map->fields+client->y)+client->x;
    target = *(map->fields+y)+x;
    
    if(target->terrain == WALL || intent == IDLE)
        return;
        
    if(target->terrain == BUSH){
        if(client->bush_intent != intent){
            client->bush_intent = intent;
            return;
        }
    }
    
    client->bush_intent = IDLE;
    if(target->actor != NULL){
        struct client_info_t *standing = target->actor->info;
        
        int targetx = standing->x;
        int targety = standing->y;
        
        calc_coords(&targetx, &targety, target->actor->intent->intent);
        
        if((targetx == client->x && targety == client->y) || (targetx == x && targety == y)){
        struct coin_t dropped = {
            .type = DROPPED,
            .coin_amount = client->gathered_coins + standing->gathered_coins + target->coin.coin_amount,
        };
        standing->gathered_coins = 0;
        client->gathered_coins = 0;
        
        if(dropped.coin_amount!=0)
            target->coin = dropped;
            
        if(target->actor->info->type!=BEAST){
            place_player(map,target->actor);
            target->actor->intent->intent = IDLE;
            target->actor->info->deaths += 1;
            target->actor = NULL;
        }
        if(current->actor->info->type!=BEAST){
            place_player(map,current->actor);
            client_data->intent->intent = IDLE;
            current->actor->info->deaths += 1;
            current->actor = NULL;
            return;
        }
        }
    }
    
    if(target->actor == NULL || target->actor->info->type!=BEAST || current->actor->info->type!=BEAST){
        target->actor = client_data;
        target->actor->info->x = x;
        target->actor->info->y = y;
    }

    
    if(target->coin.type!=NIL && target->actor->info->type != BEAST){
        target->actor->info->gathered_coins += target->coin.coin_amount;
        target->coin = no_coin;
    }
    
    if(target->terrain == CAMP){
        target->actor->info->stashed_coins += target->actor->info->gathered_coins;
        target->actor->info->gathered_coins = 0;
    }
    
    if(current->actor == client_data)
        current->actor = NULL;
}


void add_beast(struct map_t* map, struct beast_t **beasts, int *num){
    
    int num_of_beasts = *num;
    
    *beasts = (struct beast_t*)realloc(*beasts, sizeof(struct beast_t) * (num_of_beasts+1));
    
    
    struct client_data_t *data = (struct client_data_t*)malloc(sizeof(struct client_data_t));
    
    struct client_info_t *info = (struct client_info_t*)malloc(sizeof(struct client_info_t));
    struct client_intent_t *intent = (struct client_intent_t*)malloc(sizeof(struct client_intent_t));
    
    
    (*beasts+num_of_beasts)->data = data;
    
    (*beasts+num_of_beasts)->data->infofd = -1;
    (*beasts+num_of_beasts)->data->intentfd = -1;
    (*beasts+num_of_beasts)->data->intent = intent;
    (*beasts+num_of_beasts)->data->info = info;

    struct client_data_t *beast_data = (*beasts+num_of_beasts)->data; 
    place_player(map,beast_data);
    
    beast_data->info->type = BEAST;
    beast_data->info->bush_intent = IDLE;
    
    *num += 1;
}


int check_for_walls(struct map_t *map, int x1, int y1, int x2, int y2){
    int dx, dy, decide, temp;
    dx = abs(x2 - x1);
    dy = abs(y2 - y1);
    
    if(dx>dy)
        decide = 0;
    else{
        temp = dx;
        dx = dy;
        dy = temp;
        
        temp = x1;
        x1 = y1;
        y1 = temp;
        
        temp = x2;
        x2 = y2;
        y2 = temp;
        
        decide = 1;
    }
    
    int pk = 2 * dy - dx;
      for (int i = 0; i <= dx; i++) {
        if(decide == 0){
            if((*(map->fields+y1)+x1)->terrain == WALL)
                return 1;
        }
        if(decide == 1){
            if((*(map->fields+x1)+y1)->terrain == WALL)
                return 1;     
        }
        x1 < x2 ? x1++ : x1--;
        if (pk < 0) {
            if (decide == 0) {
                pk = pk + 2 * dy;
            }
            else {
                pk = pk + 2 * dy;
            }
        }
        else {
            y1 < y2 ? y1++ : y1--;
            pk = pk + 2 * dy - 2 * dx;
        }
    }
    return 0;
}

void* move_beast(void* arg){
    
    int *slots = jdata->slots;
    struct client_data_t *beast_data = (beasts+*(int*)arg)->data;
    struct client_data_t *player_data = clients;
    
    int chase_index = -1;
    int x = -1;
    int y = -1;
    int min = map->width + map->height;
    int beast_x = beast_data->info->x;
    int beast_y = beast_data->info->y;
    for(int i=0; i<4; i++){
        if(*(slots+i)==1){
            x = (player_data+i)->info->x;
            y = (player_data+i)->info->y;
            if(abs(x-beast_x)<BEAST_VISION && abs(y-beast_y)<BEAST_VISION){
                if(check_for_walls(map,x,y,beast_x,beast_y) == 0){
                    if(chase_index!=-1 && abs(x-beast_x)+abs(y-beast_y)>min)
                        continue;
                    chase_index = i;
                    min = abs(x-beast_x) + abs(y-beast_y);
                }
            }
        }
    }
    if(chase_index==-1){
        int intentcode = rand() % 4;
        beast_data->intent->intent = intentcode;
    }
    else{
        if(x>beast_x)
            beast_data->intent->intent = RIGHT;
        else if(y>beast_y)
            beast_data->intent->intent = DOWN;
        else if (x<beast_x)
            beast_data->intent->intent = LEFT;
        else
            beast_data->intent->intent = UP;
    }
    free(arg);
    return NULL;
}

void update_vision(struct map_t* map, struct client_data_t* client){
    int x = client->info->x;
    int y = client->info->y;
    
    int m = 0;
    int n = 0;
    
    
    for(int i=x-PLAYER_VISION; i<x+PLAYER_VISION; i++){
        for(int j=y-PLAYER_VISION; j<y+PLAYER_VISION; j++){
            char res = ' ';
            if(i<0 || j<0 || i>=map->width || j>=map->height){
                res = ' ';
            }
            else{
                struct field_t* field = *(map->fields+j)+i;
                if(field->actor!=NULL){
                    if(field->actor->info->type == HUMAN)
                        res = '0' + field->actor->info->client_num;
                    else
                        res = 'B';
                }
                else if(field->coin.type != NIL){
                    res = coin_chars[field->coin.type];
                }
                else{
                    res = terrain_chars[field->terrain];
                }
            }
            client->info->vision[m][n] = res;
            n++;
        }
        n=0;
        m++;
    }
}