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

#define S_NICK 11
#define S_BUFF 1024

typedef struct i {
    char nick[S_NICK];
} info;

info clients[100];
int ser_sock;

void command(int fd, char * buf);
void toall(char*buf);
void hand_c();

int main() {
    int w_sock, fd = 0, addr_len, res;
    struct sockaddr_in addr, w_addr;
    char buf[1024], f_buf[1024], ip[50];
    fd_set readfds, testfds;
    FILE*f_add;

    ser_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (ser_sock == -1) {
        perror("|gP| Err");
        exit(EXIT_FAILURE);
    }

    /*Lettura file addr.txt
    f_add = fopen("addr.txt", "r");
    if (f_add == NULL)perror("open");
    res = fscanf(f_add, "%s", ip);
    if (res == 0)perror("scan");*/

    //Indirizzo apertura socket
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9734);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_len = sizeof (w_addr);
    //int addr_l = sizeof (addr);

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

    signal(SIGINT, hand_c);

    FD_ZERO(&readfds);
    FD_SET(ser_sock, &readfds);

    while (1) {
        int fd;
        int nread;

        testfds = readfds;
        //getsockname(ser_sock, (struct sockaddr*) &addr, &addr_l);
        //printf("%s\n", inet_ntoa(addr.sin_addr));
        printf("- ");
        fflush(stdout);
        res = select(FD_SETSIZE, &testfds, NULL, NULL, NULL);
        if (res < 1) {
            perror("select");
            exit(1);
        }

        /*Start waiting*/

        for (fd = 0; fd < FD_SETSIZE; fd++) {
            if (FD_ISSET(fd, &testfds)) {
                /*  Aggiunta client  */
                if (fd == ser_sock) {
                    addr_len = sizeof (w_addr);
                    w_sock = accept(ser_sock, (struct sockaddr *) &w_addr, &addr_len);
                    FD_SET(w_sock, &readfds);
                    read(w_sock, clients[w_sock].nick, 30);
                    snprintf(buf, S_BUFF, "|%s| è entrato\n", clients[w_sock].nick);
                    toall(buf);
                }/*  Lavoro con client  */
                else {
                    ioctl(fd, FIONREAD, &nread);
                    /* Rimozione client  */
                    if (nread == 0) {
                        close(fd);
                        FD_CLR(fd, &readfds);
                        snprintf(buf, S_BUFF, "|%s| è uscito\n", clients[fd].nick);
                        toall(buf);
                    } else {
                        read(fd, f_buf, sizeof (f_buf));
                        if (f_buf[0] == '/') {
                            command(fd, f_buf);
                        } else {
                            snprintf(buf, sizeof (buf), "|%s| %s", clients[fd].nick, f_buf);
                            toall(buf);
                        }/*
                        for (i = 4; i < FD_SETSIZE; i++) {
                            if (i != fd || i != ser_sock) {
                                write(i, buf, sizeof (buf));
                                fflush(stdout);
                            }
                        }*/
                        buf[0] = '\0';
                        f_buf[0] = '\0';

                    }
                }
            }
        }
    }
    close(ser_sock);
}

void toall(char*buf) {
    int i = 4;
    printf("%s", buf);
    while (i < FD_SETSIZE) {
        if (i != ser_sock) {
            write(i, buf, S_BUFF);
            fflush(stdout);
            i++;
        }
    }
    buf[0] = '\0';
}

void command(int fd, char * buf) {
    char*cmd;
    char old[S_NICK];
    strtok(buf, "\n");
    cmd = strtok(buf, " ");
    if (strcmp(cmd, "/nick") == 0) {
        strcpy(old, clients[fd].nick);
        cmd = strtok(NULL, " ");
        if (strlen(cmd) > 11) {
            write(fd, "Troppo grande il nick!\n", 30);
            return;
        }
        strcpy(clients[fd].nick, cmd);
        printf("%s è ora |%s|\n", old, clients[fd].nick);
    } else {
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
