#include "map.h"

void* catch_input(void*arg) {
    int c;  
    while(1){
        c = getch();
        *(char*)arg = c;
    }
    return NULL;
}

extern struct join_data_t* jdata;
extern struct client_data_t clients[4];
extern struct beast_t* beasts;
extern struct map_t* map; 

int main(int argc, char **argv)
{
    int fd = shm_open("/join_data", O_CREAT | O_RDWR, 0600);
    err(fd == -1, "shm_open");
    
    ftruncate(fd, sizeof(struct join_data_t));
    jdata = (struct join_data_t*)mmap(NULL, sizeof(struct join_data_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    err(jdata == NULL, "mmap");
    
    sem_init(&jdata->client_joining, 1, 1);
    sem_init(&jdata->server_sem, 1, 0);
    
    srand(time(NULL));
    
    for(int i=0; i<4; i++){
        jdata->slots[i] = 0;
    }
    
    for(int i=0; i<4; i++){
        clients[i].infofd = shm_open(infonames[i], O_CREAT | O_RDWR, 0600);
        err(clients[i].infofd == -1, "shm_open");
        ftruncate(clients[i].infofd, sizeof(struct client_info_t));
        clients[i].info = (struct client_info_t*)mmap(NULL, sizeof(struct client_info_t), PROT_READ | PROT_WRITE, MAP_SHARED, clients[i].infofd, 0);
        
        clients[i].intentfd = shm_open(intentnames[i], O_CREAT | O_RDWR, 0600);
        err(clients[i].intentfd == -1, "shm_open");
        ftruncate(clients[i].intentfd, sizeof(struct client_intent_t));
        clients[i].intent = (struct client_intent_t*)mmap(NULL, sizeof(struct client_intent_t), PROT_READ | PROT_WRITE, MAP_SHARED, clients[i].intentfd, 0);
        
        sem_init(&clients[i].intent->client_sync,1,0);
        sem_init(&clients[i].intent->spawn_sync,1,0);
    }
    
    map = load_map("map.txt");
    
    beasts = NULL;
    int num_of_beasts = 0;
    
    pthread_t input_thread;
    char input = ' ';
    pthread_create(&input_thread, NULL, catch_input, &input);
    
    initscr();
    raw();
    noecho();
    
    add_beast(map, &beasts, &num_of_beasts);
    
    while(input!='q' && input != 'Q'){
        sleep(1);
        clear();
        for(int i=0; i<num_of_beasts; i++){
            int *arg = malloc(sizeof(*arg));
                if ( arg == NULL ) {
                    exit(EXIT_FAILURE);
                }
            *arg = i;
            pthread_create(&(beasts+i)->tid, NULL, move_beast, arg);
        }
        for(int i=0; i<4; i++){
            if(jdata->slots[i] == 1){
                //BRAK ODP KLIENTA
                if(clients[i].intent->status == PENDING){
                    jdata->slots[i] = 0;
                    (*(map->fields+clients[i].info->y)+clients[i].info->x)->actor = NULL;
                    sem_post(&jdata->client_joining);
                    continue;
                }
                if(clients[i].intent->status == JOINED){
                    clients[i].info->type = HUMAN;
                    clients[i].info->client_num = i+1;
                    place_player(map,&clients[i]);
                    clients[i].info->stashed_coins = 0;
                    clients[i].info->deaths = 0;
                    clients[i].info->bush_intent = IDLE;
                    sem_post(&clients[i].intent->spawn_sync);
                }
                else{
                    handle_player_move(map, &clients[i]);
                }
                
                clients[i].intent->status = PENDING;
                clients[i].intent->intent = IDLE;
                
                
                //SEMAFOR POWIADAMIAJACY KLIENTA
                sem_post(&clients[i].intent->client_sync);
            }
        }
        for(int i=0; i<num_of_beasts; i++){
            handle_player_move(map, (beasts+i)->data);
        }
        if(input=='b' || input=='B'){
            add_beast(map, &beasts, &num_of_beasts);
            input = ' ';
        }
        if(input=='t' || input=='T' || input =='c'){
            add_treasure(map, input);
            input = ' ';
        }
        
        for(int i=0; i<4; i++){
            if(jdata->slots[i]==1){
                update_vision(map,&clients[i]);
            }
        }
        
        display_map(map);
        print_legend(map->width+10,10);
        print_client_labels(map->width+10,2);
        for(int i=0; i<4; i++){
            if(jdata->slots[i]==1){
                print_client_info(clients[i].info,map->width+27+(8*i),2);
            }
        }
        refresh();
    }
    
    
    for(int i=0; i<map->height;i++){
        free(*(map->fields+i));
    }
    free(map->fields);
    
    
    for(int i=0; i<num_of_beasts; i++){
        free((beasts+i)->data->intent);
        free((beasts+i)->data->info);
        free((beasts+i)->data);
    }
    free(beasts);
    
    
    for(int i=0; i<4 ;i++){
        munmap(clients[i].info, sizeof(struct client_info_t));
        munmap(clients[i].intent, sizeof(struct client_intent_t));
        
        close(clients[i].infofd);
        close(clients[i].intentfd);
        
        shm_unlink(infonames[i]);
        shm_unlink(intentnames[i]);
    }    

    munmap(jdata, sizeof(struct join_data_t));
    close(fd);
    shm_unlink("/join_data");
    
	return 0;
}


