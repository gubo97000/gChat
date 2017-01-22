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

#define S_BUFF 1024
#define S_NICK 11
#define S_HOST 20

typedef struct i {
    char host[S_HOST];
    char nick[S_NICK];
} conf_t;

int intro();
int fconf_init();
int sconf_up(FILE*f_add, conf_t*conf);
int is_empty(char *s);

int main() {
    int c_sock, res = -1, m = 0;
    struct sockaddr_in addr;
    struct hostent *inf;
    char buf[S_BUFF], sup[S_BUFF];
    char*room;
    FILE*f_add;
    conf_t conf;

    while (1) {
        buf[0] = '\0';
        printf(">> gChat Visualizer <<\n");

        //Apertura conf.txt o creazione se non esiste
        f_add = fopen("conf.txt", "r");
        if (f_add == NULL) {
            perror("openr");
            fconf_init();
        }

        //Input room and check 
        while (1) {
            printf("In che stanza vuoi entrare?: ");
            fgets(buf, S_BUFF, stdin);
            res = is_empty(buf);
            if (res != 1) {
                break;
            } else {
                strcpy(buf, "plaza\n");
                printf("Default\n");
                break;
            }
        }

        //Open conf.txt e connessione
        while (1) {
            //Creazione socket
            c_sock = socket(AF_INET, SOCK_STREAM, 0);
            if (c_sock == -1) {
                perror("|gC| Err");
                exit(EXIT_FAILURE);
            }

            //Connessione al server
            while (1) {
                //Caricamento conf e check host
                while (1) {
                    //Caricamento dati da conf.txt a struct conf
                    sconf_up(f_add, &conf);
                    //Ricerca host by name
                    inf = gethostbyname(conf.host);
                    if (inf == NULL) {
                        printf("Non trovo l'host!");
                        sleep(2);
                    } else {
                        break;
                    }
                }

                //Stampa di host e ip host
                printf("|gV| Connessione a -%s- su ", conf.host);
                char **addrs;
                addrs = inf -> h_addr_list;
                while (*addrs) {
                    printf(" %s", inet_ntoa(*(struct in_addr *) *addrs));
                    addrs++;
                }
                printf("\n");

                //Indirizzo server
                addr.sin_family = AF_INET;
                addr.sin_port = htons(9734);
                addr.sin_addr = *(struct in_addr*) *inf->h_addr_list;

                //Connessione
                res = connect(c_sock, (struct sockaddr*) &addr, sizeof (addr));
                if (res == -1) {
                    perror("|gC| Conn");
                    printf("|gC| Riprovo a connettermi in 5 sec!\n");
                    errno = 0;
                    sleep(5);
                } else {
                    printf("|gV| Connesso con server!\n");
                    break;
                }
            }
            fclose(f_add);


            strcpy(conf.nick, "Visualizer");

            //Riconoscimento stanza
            //printf("|gV| Hello... ");
            res = write(c_sock, conf.nick, S_NICK);
            if (res == -1) {
                perror("write");
            }
            //Invio nome room
            printf("|gV| Ask for room '%s' \n", buf);
            res = write(c_sock, buf, S_BUFF);
            if (res == -1) {
                perror("write");
            }
            printf("|gV| Il server dice: ");
            //Risposta server
            res = read(c_sock, buf, S_BUFF);
            if (res == -1) {
                perror("read");
            }

            if (strcmp(buf, "Room not found!\n") != 0) {
                printf("%s", buf);
                break;
            } else {
                printf("%s", buf);
                exit(EXIT_FAILURE);
            }
        }
        printf("|gV| Connesso alla stanza!\n");
        fflush(stdout);

        //Working
        while (1) {
            buf[0] = '\0';
            read(c_sock, buf, 1024);
            if (strcmp(buf, "Chiusura server!\n") == 0) {
                break;
            }
            printf("%s", buf);
            fflush(stdout);
        }
    }

    close(c_sock);
}

int intro() {
    char input[256];
    printf("|gC| 0) Piazza\n");
    printf("|gC| 1) Connessione privata\n");
    printf("> ");
    while (1) {
        scanf("%s", input);

        if (strcmp(input, "0") == 0)
            intro();
        else if (strcmp(input, "1") == 0)
            intro();
        else if (strcmp(input, "3") == 0)
            intro();

        else
            printf("Allowed commands: 0, 1, 3.\n");
    }
}

int fconf_init() {
    FILE*f_add;
    char buf[S_HOST], nick[S_NICK];
    ;

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
    /*
        //Scelta nick, controllo lunghezza/eliminazione '\n'
        while (1) {
            char *pos;
            printf("|gC| Scegli un nick: ");
            fgets(nick, S_NICK, stdin);
            if (nick[0] == '\n') {
                printf("Non c'è nick!\n");
            } else if ((pos = strchr(nick, '\n')) != NULL) {
     *pos = '\0';
                break;
            } else {
                __fpurge(stdin);
                printf("Il nick è troppo grande!\n");
            }
        }
        fprintf(f_add, "#insert nick in the next line(no space after':')\nnick:%s\n", nick);
     */
    fflush(f_add);
    rewind(f_add);
}

int sconf_up(FILE*f_add, conf_t * conf) {
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

int is_empty(char *s) {
    int res = 0, i = 0;
    char buf[S_BUFF];
    strcpy(buf, s);
    while (buf[i] != '\0') {

        if (buf[i] == ' ' || buf[i] == '\n')
            i++;
        else return 0;
    }
    return 1;
}

