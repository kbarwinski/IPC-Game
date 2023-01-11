#include "common.h"


struct client_data_t client;
struct join_data_t* jdata;
int fd, hasJoined;
WINDOW *menu_win;

void* catch_input(void*arg) {
    int c;  
    while(1){
        c = wgetch(menu_win);
        *(char*)arg = c;
    
        switch(c){
            case KEY_UP:
                client.intent->intent = UP;
                break;
            case KEY_LEFT:
                client.intent->intent = LEFT;
                break;
            case KEY_RIGHT:
                client.intent->intent = RIGHT;
                break;
            case KEY_DOWN:
                client.intent->intent = DOWN;
                break;
            case 'q':
            case 'Q':
            {
                
                        munmap(client.info, sizeof(struct client_info_t));
                        munmap(client.intent, sizeof(struct client_intent_t));
                        
                        close(client.infofd);
                        close(client.intentfd);
                        
                        munmap(jdata, sizeof(struct join_data_t));
                        close(fd);
                        exit(0);
                        break;
            }
        }
    }
    return NULL;
}

int main(int argc, char **argv)
{

    fd = shm_open("/join_data", O_RDWR, 0600);
    err(fd == -1, "shm_open");
    
    ftruncate(fd, sizeof(struct join_data_t));
    jdata = (struct join_data_t*)mmap(NULL, sizeof(struct join_data_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    err(jdata == NULL, "mmap");
    
    hasJoined = 0;
    
    
    initscr();
    raw();
    noecho();
    menu_win = newwin(200,200,0,0);
    keypad(menu_win,TRUE);
    
    printw("Waiting for a free server slot...\n");
    refresh();
    
    while(!hasJoined){
        sem_wait(&jdata->client_joining);
        for(int i=0; i<4; i++){
            if(jdata->slots[i] == 0){
                jdata->slots[i] = 1;
                
                printw("Joined as client %d!\n",i+1);
                refresh();
                
                sem_post(&jdata->client_joining);
                hasJoined = i+1;
                
                client.infofd = shm_open(infonames[i], O_RDWR, 0600);
                err(client.infofd == -1, "shm_open");
                ftruncate(client.infofd, sizeof(struct client_info_t));
                client.info = (struct client_info_t*)mmap(NULL, sizeof(struct client_info_t), PROT_READ, MAP_SHARED, client.infofd, 0);
                
                client.intentfd = shm_open(intentnames[i], O_RDWR, 0600);
                err(client.intentfd == -1, "shm_open");
                ftruncate(client.intentfd, sizeof(struct client_intent_t));
                client.intent = (struct client_intent_t*)mmap(NULL, sizeof(struct client_intent_t), PROT_READ | PROT_WRITE, MAP_SHARED, client.intentfd, 0);
                
                client.intent->status = JOINED;
                client.intent->intent = IDLE;
                sem_wait(&client.intent->spawn_sync);
                break;
            }
        }
    }
    
    pthread_t input_thread;
    char input = 'z';
    pthread_create(&input_thread, NULL, catch_input, &input);
    
    while(1){
        sem_wait(&(client.intent->client_sync));
        client.intent->status = ACTIVE;
        clear();
        int x = client.info->x + 5;
        int y = client.info->y + 2;
        print_client_labels(50,2);
        print_client_info(client.info,67,2);
        print_legend(50,10);
        for(int i=0; i<2*PLAYER_VISION+1; i++){
            for(int j=0; j<2*PLAYER_VISION+1; j++){
                mvprintw(y+j,x+i,"%c",client.info->vision[i][j]);
            }
        }
        refresh();
    }

    
	return 0;
}
