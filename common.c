#include "common.h"

const char* infonames[4] = {
    "/cl1",
    "/cl2",
    "/cl3",
    "/cl4",
};

const char* intentnames[4] = {
    "/cl1_intent",
    "/cl2_intent",
    "/cl3_intent",
    "/cl4_intent",
};

void print_legend(int x, int y){
    
    mvprintw(y-1,x+2,"* * * LEGEND * * *");
    
    mvprintw(y,x,"1234");
    mvprintw(y,x+5,"- players");
    
    mvprintw(y+1,x+3,".");
    mvprintw(y+1,x+5,"- ground");
    
    mvprintw(y+2,x+3,"|");
    mvprintw(y+2,x+5,"- wall");
    
    mvprintw(y+3,x+3,"#");
    mvprintw(y+3,x+5,"- bush");
    
    mvprintw(y+4,x+3,"A");
    mvprintw(y+4,x+5,"- camp");
    
    mvprintw(y+5,x+3,"c");
    mvprintw(y+5,x+5,"- coin");
    
    mvprintw(y+6,x+3,"t");
    mvprintw(y+6,x+5,"- small treasure");
    
    mvprintw(y+7,x+3,"T");
    mvprintw(y+7,x+5,"- large treasure");
}
void print_client_labels(int x, int y){
    mvprintw(y,x,"Player number:");
    mvprintw(y+1,x,"Player type:");
    mvprintw(y+2,x,"Current X/Y:");
    mvprintw(y+3,x,"Deaths:");
    mvprintw(y+4,x,"Gathered coins:");
    mvprintw(y+5,x,"Stashed coins:");
}
void print_client_info(struct client_info_t* client, int x, int y){
    mvprintw(y,x,"%d",client->client_num);
    mvprintw(y+1,x,"HUMAN");
    mvprintw(y+2,x,"(%d/%d)",client->x,client->y);
    mvprintw(y+3,x,"%d",client->deaths);
    mvprintw(y+4,x,"%d",client->gathered_coins);
    mvprintw(y+5,x,"%d",client->stashed_coins);
}