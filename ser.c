#include <stdio.h>
#include <stdio_ext.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <string.h>
#include <sys/time.h>
#include <signal.h>
#include <pthread.h>
#include <ctype.h>

#define S_NICK 11
#define S_HOST 20
#define S_BUFF 1024
#define N_CLNT 100

typedef struct i {
    char nick[S_NICK]; /*Nickname*/
    int adm; /*Admin?*/
} info;

typedef struct l {
    char adps[S_BUFF]; /*Admin password*/
} conf_t;

typedef struct c {
    char name[S_BUFF]; /*Private chat name*/
    int occupied; /*If the private room was closed*/
    fd_set set; /*fd_set for the chat room*/
    fd_set tset; /*Test set*/
} chat_t;

info clients[N_CLNT];
chat_t priv[N_CLNT];
pthread_t thr[N_CLNT];

int ser_sock;
conf_t conf;
struct timeval timeout;

//Lettore comandi /cmd
void command(int fd, char * buf, chat_t* pri);
//Send buf to all fd from 4 and print
void toall(char*buf);
//Send buf to all fd in fd_set
void toroom(char*buf, chat_t * pri);
//Handler SIGINT
void hand_c();
//Initialize file ser_conf.txt
int fconf_init();
//Read ser_conf.txt save data in struct conf_t
int sconf_up(FILE*f_add, conf_t*conf);
//Move from a set to another
int fd_move(int fd, chat_t* src, chat_t* dst);
//Function for thread (Private room)
void*priv_room(void*room);
//Set room for visualizer
int set_vis(int fd, chat_t* pri);

int main() {
    int w_sock, addr_len, res, fd = 0, nread;
    struct sockaddr_in addr, w_addr;
    char buf[1024], f_buf[1024];

    FILE*f_add;
    timeout.tv_sec = 2;
    timeout.tv_usec = 2;
    //Inizializzazione priv[0]
    strcpy(priv[0].name, "Hall");
    priv[0].set;
    priv[0].occupied = 1;

    ser_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (ser_sock == -1) {
        perror("|gP| Err");
        exit(EXIT_FAILURE);
    }

    //Apertura conf.txt o creazione se non esiste
    f_add = fopen("ser_conf.txt", "r");
    if (f_add == NULL) {
        perror("openr");
        fconf_init();
        f_add = fopen("ser_conf.txt", "r");
    }
    //Caricamento dati da conf.txt a struct conf
    sconf_up(f_add, &conf);

    //Indirizzo apertura socket
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9734);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_len = sizeof (w_addr);

    res = bind(ser_sock, (struct sockaddr*) &addr, sizeof (addr));
    if (res == -1) {
        perror("|gP| Bind");
        exit(EXIT_FAILURE);
    }

    res = listen(ser_sock, 5);
    if (res == -1) {
        perror("|gP| Listen");
        exit(EXIT_FAILURE);
    }

    //Gestione segnale SIGINT
    signal(SIGINT, hand_c);

    //Work
    while (1) {
        printf("- ");
        fflush(stdout);

        FD_ZERO(&priv[0].set);
        FD_SET(ser_sock, &priv[0].set);

        /*Start waiting*/
        do {
            FD_ZERO(&priv[0].tset);
            priv[0].tset = priv[0].set;
            res = select(FD_SETSIZE, &(priv[0].tset), NULL, NULL, &timeout);
            if (res == -1) {
                perror("select");
                fflush(stdout);
                exit(1);
            }
            timeout.tv_sec = 1;
            timeout.tv_usec = 2;
        } while (res == 0);
        /*Something happend*/
        for (fd = 0; fd < FD_SETSIZE; fd++) {
            if (FD_ISSET(fd, &(priv[0].tset))) {
                /*  Aggiunta client  */
                if (fd == ser_sock) {
                    addr_len = sizeof (w_addr);
                    w_sock = accept(ser_sock, (struct sockaddr *) &w_addr, &addr_len);

                    FD_SET(w_sock, &priv[0].set);

                    read(w_sock, clients[w_sock].nick, S_NICK);
                    clients[w_sock].adm = 0;

                    //Visualizer
                    if ((strcmp(clients[w_sock].nick, "Visualizer")) == 0) {
                        if (set_vis(w_sock, &priv[0]) == -1) {
                            write(w_sock, "Room not found!\n", S_BUFF);
                        } else {
                            write(w_sock, "Room found!\n", S_BUFF);
                        }
                    } /*Input client*/ else {
                        /*snprintf(buf, S_BUFF, "|%s| \n", clients[w_sock].nick);
                        toroom(buf, &priv[0]);*/
                        strcpy(buf, "/prch plaza\n");
                        command(w_sock, buf, &priv[0]);

                    }

                }

            }
        }
    }

    close(ser_sock);
}

void toroom(char*buf, chat_t* pri) {
    int i = 4, r = 0;

    dprintf(1, "%s, %s", pri->name, buf);
    while (i < FD_SETSIZE) {
        r = FD_ISSET(i, &pri->set);
        if (i != 2 && i != ser_sock && r != 0) {
            write(i, buf, S_BUFF);
        }
        i++;
    }
    buf[0] = '\0';
}

void toall(char*buf) {
    int i = 4;
    printf("%s", buf);
    while (i < FD_SETSIZE) {
        if (i != ser_sock) {
            write(i, buf, S_BUFF);
            fflush(stdout);
        }
        i++;
    }
    buf[0] = '\0';
}

void command(int fd, char * buf, chat_t *pri) {
    char*cmd;
    char old[S_NICK], sup[S_BUFF];
    int t, i = 0, r = 1;
    strtok(buf, "\n");
    cmd = strtok(buf, " ");
    //Nick cmd
    if (strcmp(cmd, "/nick") == 0) {
        strcpy(old, clients[fd].nick);
        cmd = strtok(NULL, " ");
        if (strlen(cmd) > 11) {
            write(fd, "Troppo grande il nick!\n", 30);
            return;
        }
        strcpy(clients[fd].nick, cmd);
        snprintf(buf, S_BUFF, "%s è ora |%s|\n", old, clients[fd].nick);
        toroom(buf, pri);
    }
    //Admin cmd
    if (strcmp(cmd, "/admn") == 0) {

        cmd = strtok(NULL, " ");
        t = strcmp(cmd, conf.adps);
        if (t != 0) {
            write(fd, "Non puoi diventare admin\n", 30);
            return;
        }
        clients[fd].adm = 1;
        snprintf(buf, S_BUFF, "|%s| è ora admin\n", clients[fd].nick);
        toroom(buf, pri);
    }
    //Broadcast cmd
    if (strcmp(cmd, "/cast") == 0) {
        if (clients[fd].adm == 1) {
            cmd = strtok(NULL, "\n");
            snprintf(sup, S_BUFF, "|%s| AVVISO: %s\n", clients[fd].nick, cmd);
            toall(sup);
        }
    }
    //Private room cmd
    if (strcmp(cmd, "/prch") == 0) {
        cmd = strtok(NULL, " ");
        //Check if room exist
        for (i = 0; i < N_CLNT; i++) {
            t = strcmp(cmd, priv[i].name);
            if (t == 0) {
                fd_move(fd, pri, &priv[i]);
                r = 0;
                snprintf(buf, S_BUFF, "|%s| entrato in %s \n", clients[fd].nick, priv[i].name);
                toroom(buf, pri);
                break;
            }
        }
        if (r) {
            //Check free room and create
            for (i = 0; i < N_CLNT; i++) {
                if (priv[i].occupied == 0) {
                    priv[i].occupied = 1;
                    strcpy(priv[i].name, cmd);
                    FD_ZERO(&priv[i].set);
                    fd_move(fd, pri, &priv[i]);
                    pthread_create(&thr[i], NULL, priv_room, (void*) &priv[i]);
                    snprintf(buf, S_BUFF, "|%s| ha creato ed è in %s \n", clients[fd].nick, priv[i].name);
                    toroom(buf, pri);
                    return;
                }
            }
            snprintf(buf, S_BUFF, "Tutto occupato!\n");
            toroom(buf, pri);
        }

    } else {/*Wrong cmd*/
        write(fd, "Comando non riconosciuto\n", 30);
        return;
    }
}

void hand_c() {
    char buf[S_BUFF];
    strcpy(buf, "Chiusura server!\n");
    toall(buf);
    signal(SIGINT, SIG_DFL);
    raise(SIGINT);
}

int fconf_init() {
    FILE*f_add;

    f_add = fopen("ser_conf.txt", "w+");
    if (f_add == NULL) {
        perror("openw");
    }
    printf("|gC| Creazione ./ser_conf.txt\n");
    printf("|gC| Modifica ./ser_conf.txt e riavvia il server\n");
    fprintf(f_add, "#insert admin password in the next line(no space after':')\nadps:");
    fflush(f_add);
    fclose(f_add);
    exit(EXIT_SUCCESS);
}

int sconf_up(FILE*f_add, conf_t*conf) {
    int i = 0, check = 0;
    char *pos, *res;
    char buf[S_BUFF];
    char* sett[1] = {"adps"};
    for (i = 0; i < 1; i++) {
        rewind(f_add);
        check = 0;
        while (res != 0) {
            res = fgets(buf, S_BUFF, f_add);
            if (buf[0] != '\n' && buf[0] != '#') {
                pos = strtok(buf, ":");
                if (strcmp(pos, sett[i]) == 0) {
                    pos = strtok(NULL, "\n");
                    strcpy(conf->adps, pos);
                    res = 0;
                    check = 1;
                }
            }
        }
        if (check == 0) {
            printf("Non ho trovato %s, controlla ser_conf.txt\n", sett[i]);
            break;
        }
    }
}

int fd_move(int fd, chat_t* src, chat_t* dst) {
    FD_SET(fd, &(dst->set));
    FD_SET(fd, &(dst->tset));
    FD_CLR(fd, &(src->set));
    FD_CLR(fd, &(src->tset));
}

int set_vis(int fd, chat_t* pri) {
    char*pos;
    char buf[S_BUFF];
    int t = 0, i = 0;
    read(fd, buf, S_BUFF);
    pos = strtok(buf, "\n");

    //Check if room exist
    for (i = 0; i < N_CLNT; i++) {
        t = strcmp(pos, priv[i].name);
        if (t == 0) {
            fd_move(fd, pri, &priv[i]);
            return 0;
        }
    }
    return -1;
}

void*priv_room(void*room) {
    chat_t* pri = (chat_t*) room;
    timeout.tv_sec = 1;

    int fd, nread, res;
    char buf[S_BUFF], f_buf[S_BUFF];

    while (1) {
        FD_ZERO(&(pri->tset));

        printf("!%s! ", pri->name);
        fflush(stdout);

        /*Start waiting*/
        do {
            FD_ZERO(&pri->tset);
            pri->tset = pri->set;
            res = select(FD_SETSIZE, &(pri->tset), NULL, NULL, &timeout);
            if (res == -1) {
                perror("select");
                fflush(stdout);
                exit(1);
            }
            timeout.tv_sec = 1;
            timeout.tv_usec = 2;
        } while (res == 0);

        for (fd = 0; fd < FD_SETSIZE; fd++) {

            if (FD_ISSET(fd, &(pri->tset))) {

                /* Client action */
                ioctl(fd, FIONREAD, &nread);

                /* Rimozione client  */
                if (nread == 0) {
                    if ((strcmp(clients[fd].nick, "Visualizer")) != 0) {
                        snprintf(buf, S_BUFF, "|%s| è uscito\n", clients[fd].nick);
                        toroom(buf, pri);
                    }
                    close(fd);
                    FD_CLR(fd, &(pri->set));

                }/*Client sent something*/
                else {
                    read(fd, f_buf, sizeof (f_buf));
                    //Commad
                    if (f_buf[0] == '/') {
                        command(fd, f_buf, pri);
                    }//Plain text 
                    else {
                        snprintf(buf, sizeof (buf), "|%s| %s", clients[fd].nick, f_buf);
                        toroom(buf, pri);
                    }
                    buf[0] = '\0';
                    f_buf[0] = '\0';

                }
            }
        }
    }
}


