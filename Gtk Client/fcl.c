#include "func.h"

int connected = 1;
int c_sock, res = -1, r = 1, not_empty = 0;
struct sockaddr_in addr;
struct hostent *inf;
char buf[S_BUFF], rcv[S_BUFF];
FILE*f_add;
conf_t conf;
pthread_t t1;

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

int genuine(const char*buf) {
    if ('/' == *buf) {
        buf += 5;
        if (is_empty(buf)) return 0;
    }

    if (is_empty(buf)) return 0;

    return 1;
}

int is_empty(const char *s) {
    while (*s != '\0') {
        if (isspace(*s) == 0)
            return 0;
        s++;
    }
    return 1;
}

void*visualizer() {
    //mark = gtk_text_buffer_get_mark((GtkTextBuffer*) buffer, "scroll");
    while (1) {
        read(c_sock, rcv, S_BUFF);
        //Check message type 
        if (*AL == rcv[0]) {
            gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, &rcv[1], -1,
                    "alert", NULL);
        }//Alert format
        else if (*SE == rcv[0]) {
            gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, &rcv[1], -1,
                    "service", NULL);
        }//Service format
        else if (*RO == rcv[0]) {
            gtk_label_set_text(GTK_LABEL(room), &rcv[1]);
        }//Room_name_change
        else if (*NI == rcv[0]) {
            gtk_label_set_text(GTK_LABEL(nick), &rcv[1]);
        }//Nick_name_change
        else {
            gtk_text_buffer_insert(buffer, &iter, rcv, -1);
        }//Text format

        if (strcmp(rcv, "Chiusura server!\n") == 0) {
            buf[0] = '\0';
            raise(SIGINT);
        }//Server chiuso

        // gtk_text_buffer_move_mark(buffer, mark, &iter);
        // gtk_text_view_scroll_mark_onscreen((GtkTextView*) view, mark);
    }
}

void login() {

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
                printf("|gC| Non trovo l'host!\n");
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
    gtk_label_set_text(GTK_LABEL(nick), conf.nick);
}

//Interface response

void button_clicked(GtkWidget *button, gpointer data) {
    gtk_signal_emit_by_name(GTK_OBJECT(message_entry), "activate");
}

void press_enter(GtkWidget *message_entry, gpointer data) {
    const char *output = gtk_entry_get_text(GTK_ENTRY((GtkWidget *) data));
    snprintf(buf, S_BUFF, "%s\n", output);
    //Write
    if (genuine(buf)) {
        res = write(c_sock, buf, 1024);
        if (res == -1) {
            perror("|gC| write");
            exit(EXIT_FAILURE);
        }
    }
    //Clean entry
    gtk_entry_set_text(GTK_ENTRY(message_entry), "");
}

