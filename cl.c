#define _GNU_SOURCE

#include <stdio.h>
#include <stdio_ext.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <ctype.h>
#include <pthread.h>
#include <signal.h>

#define S_BUFF 1024
#define S_NICK 11
#define S_HOST 20

typedef struct i {
    char host[S_HOST];
    char nick[S_NICK];
} conf_t;

//File conf.txt initialize 
int fconf_init();
//Load conf.txt
int sconf_up(FILE*f_add, conf_t*conf);
//Check if string is empty 
int is_empty(const char *s);
//Check if server closed
void*thread_check(void*c_sock);
//Check if a command is empty
int check(char buf[S_BUFF]);

int connected = 1;

int main() {
    int c_sock, res = -1, r = 1, not_empty = 0;
    struct sockaddr_in addr;
    struct hostent *inf;
    char buf[1024], ip[50];
    FILE*f_add;
    conf_t conf;
    pthread_t t1;

    while (r) {
        printf(">> gChat Client <<\n");
        connected = 1;

        //Create socket
        c_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (c_sock == -1) {
            perror("|gC| Err");
            exit(EXIT_FAILURE);
        }

        //Open conf.txt o creazione se non esiste
        f_add = fopen("conf.txt", "r");
        if (f_add == NULL) {
            perror("openr");
            fconf_init();
            f_add = fopen("conf.txt", "r");
        }

        //Load conf.txt, check host, connect
        while (res == -1) {
            errno = 0;
            while (1) {
                //Caricamento dati da conf.txt a struct conf
                sconf_up(f_add, &conf);
                //Ricerca host by name
                inf = gethostbyname(conf.host);
                if (inf == NULL) {
                    printf("Non trovo l'host!\n");
                    sleep(2);
                } else break;
            }
            //Print host with ip
            printf("|gC| Connessione a -%s- su ", conf.host);
            char **addrs;
            addrs = inf -> h_addr_list;
            while (*addrs) {
                printf(" %s", inet_ntoa(*(struct in_addr *) *addrs));
                addrs++;
            }
            printf("\n");

            //Load server address
            addr.sin_family = AF_INET;
            addr.sin_port = htons(9734);
            addr.sin_addr = *(struct in_addr*) *inf->h_addr_list;

            //Connect
            res = connect(c_sock, (struct sockaddr*) &addr, sizeof (addr));
            if (res == -1) {
                perror("|gC| Conn");
                printf("|gC| Riprovo a connettermi in 5 sec!\n");
                sleep(5);
            }
        }

        write(c_sock, conf.nick, S_NICK);
        printf("|gC| Connesso con server!\n");

        /*//Creating fd for non blocking read (Alt.)
        val = fcntl(c_sock, F_GETFL, 0);
        fcntl(c_sock, F_SETFL, val | O_NONBLOCK);*/

        //Check server closed by thread
        pthread_create(&t1, NULL, thread_check, (void*) &c_sock);

        //Working
        while (connected) {
            not_empty = 1;
            /*//Check if server closed (Alt.)
              read(c_sock,buf, sizeof(buf));
               if (strncmp(buf, "Chiusura server!\n", 16) == 0) {
                  break;
              }*/
            printf(">> ");
            if (fgets(buf, S_BUFF, stdin) == NULL) {
                perror("|gC| fgets");
                exit(EXIT_FAILURE);
            }
            //Check if the command is empty
            if (strncmp(buf, "/", 1) == 0) {
                not_empty = check(buf);
            }
            //Check /end
            if (strncmp(buf, "/end", 4) == 0) {
                r = 0;
                break;
            }
            //Write
            if (!is_empty(buf) && connected == 1 && not_empty == 1) {
                res = write(c_sock, buf, 1024);
                if (res == -1) {
                    perror("|gC| write");
                    exit(EXIT_FAILURE);
                }
            }
            fflush(stdin);
        }

        printf("|gC| Disconnesso dal server\n");
    }
    close(c_sock);
    pthread_cancel(t1);
}

int fconf_init() {
    FILE*f_add;
    char buf[S_HOST], nick[S_NICK];

    f_add = fopen("conf.txt", "w+");
    if (f_add == NULL) {
        perror("openw");
    }
    printf("|gC| Creazione ./conf.txt\n");

    //Scelta host, controllo lunghezza/eliminazione '\n'
    while (1) {
        char *pos;
        printf("|gC| Inserisci host: ");
        fgets(buf, S_HOST, stdin);
        if (buf[0] == '\n') {
            printf("Non c'è host!\n");
        } else if ((pos = strchr(buf, '\n')) != NULL) {
            *pos = '\0';
            break;
        } else {
            __fpurge(stdin);
            printf("Nome host troppo grande!\n");
        }
    }
    fprintf(f_add, "#insert host in the next line(no space after':')\nhost:%s\n", buf);
    fflush(f_add);

    //Scelta nick, controllo lunghezza/eliminazione '\n'
    while (1) {
        char *pos;
        printf("|gC| Scegli un nick: ");
        fgets(nick, S_NICK, stdin);
        if (nick[0] == '\n' || is_empty(nick) == 1) {
            printf("Nick vuoto!\n");
        } else if ((pos = strchr(nick, '\n')) != NULL) {
            *pos = '\0';
            break;
        } else {
            __fpurge(stdin);
            printf("Il nick è troppo grande!\n");
        }
    }
    fprintf(f_add, "#insert nick in the next line(no space after':')\nnick:%s\n", nick);
    fflush(f_add);
    fclose(f_add);
}

int sconf_up(FILE*f_add, conf_t*conf) {
    char*pos, *res;
    char buf[S_BUFF];
    int check[2] = {0};

    do {
        res = fgets(buf, S_BUFF, f_add);
        if (buf[0] == '\n' || buf[0] == '#') {
            continue;
        }
        pos = strtok(buf, ":");
        if (strcmp(pos, "host") == 0 && !check[0]) {
            pos = strtok(NULL, "\n");
            strcpy(conf->host, pos);
            check[0] = 1;
            continue;
        }
        if (strcmp(pos, "nick") == 0 && !check[1]) {
            pos = strtok(NULL, "\n");
            strcpy(conf->nick, pos);
            check[1] = 1;
            continue;
        }

    } while (res != 0);
    rewind(f_add);
}

int is_empty(const char *s) {
    while (*s != '\n') {
        if (isspace(*s) == 0)
            return 0;
        s++;
    }
    return 1;
}

void*thread_check(void*c_sock) {
    char buf[S_BUFF];
    while (1) {
        read(*(int*) c_sock, buf, S_BUFF);
        if (strcmp(buf, "Chiusura server!\n") == 0) {
            printf("%s", buf);
            connected = 0;
            buf[0] = '\0';
            raise(SIGINT);
            pthread_yield();
        }
    }
}

int check(char* chk) {
    char buf[S_BUFF];
    strcpy(buf, chk);

    chk = strtok(buf, "\n");
    chk = strtok(buf, " ");
    chk = strtok(NULL, " ");
    if (chk == NULL) {
        return -1;
    }
    if (is_empty(chk) == 1) {
        return -1;
    }
    return 1; /*Not empty*/
}