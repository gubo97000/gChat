#include "func.h"

int main(void) {
    pthread_t t_vis;

    //Start interface
    interface(0, NULL);
   
    //Log the server, open conf.txt
    login();
        
    //Start thread for visualizer
    pthread_create(&t_vis, NULL, visualizer, NULL);

    //Start graphic
    gtk_main();
}

