#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main() {
    int c_sock, res = -1, r = 1;
    struct sockaddr_in addr;
    char buf[1024],ip[50];
    FILE*f_add;
    while (1) {
        buf[0] = '\0';
        printf(">> gChat Visualizer <<\n");

        //intro();

        //Creazione socket
        c_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (c_sock == -1) {
            perror("|gV| Err");
            exit(EXIT_FAILURE);
        }
        f_add = fopen("addr.txt", "w+");
        fgets(ip, 50, fileno);
        strtok(ip,"\n");
        fclose(f_add);
        //Indirizzo server
        addr.sin_family = AF_INET;
        addr.sin_port = htons(9734);
        addr.sin_addr.s_addr = inet_addr(ip);

        //Connessione
        while (res == -1) {
            res = connect(c_sock, (struct sockaddr*) &addr, sizeof (addr));
            if (res == -1) {
                perror("|gV| Err");
                printf("|gV| Riprovo a connettermi in 5 sec!\n");
                sleep(5);
            }
        }
        write(c_sock, "Vis", 4);
        printf("|gV| Connesso con server!\n");

        //Working

        while (1) {
            fflush(stdin);
            read(c_sock, buf, 1024);
            if (strcmp(buf, "Chiusura server!\n") == 0) {
                r = 0;
                break;
            }
            printf("%s", buf);
            fflush(stdout);
        }

        close(c_sock);
    }
}




