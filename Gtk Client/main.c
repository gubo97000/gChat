#include "func.h"

int main(void) {
   
    pthread_t t_vis, t2;

    //Start interface
    interface(0, NULL);
   
    //Log the server, open conf.txt
    login();
    
    //Start thread for work
    //pthread_create(&t2, NULL, (void*) work, NULL);
    
    //Start thread for visualizer
    pthread_create(&t_vis, NULL, visualizer, NULL);

    gtk_main();
}

