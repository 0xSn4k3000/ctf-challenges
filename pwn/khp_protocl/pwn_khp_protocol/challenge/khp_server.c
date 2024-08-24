#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include <pthread.h>

#include "./khp_func.h"

void ShowHelp() {
    printf("Usage:\n");
    printf("\t -l : Set ip to listen on \n");
    printf("\t -p : Set port to listen on \n");
    printf("\t -h : To show this message. \n");
}

int main(int argc, char *argv[]) {
    setvbuf(stdout, 0, 2, 0);
    setvbuf(stdin, 0, 2, 0);
    Log("Starting keys holder protocol server....");

    char PORT[8] = "8080";

    int opt;
    while ((opt = getopt(argc, argv, "p:h")) != -1) {
        switch (opt) {
            case 'p':
                strncpy(PORT, optarg, sizeof(PORT));
                break;
            case 'h':
                ShowHelp();
                break;
            default:
                ShowHelp();
                break;
        }
    }

    int SERVER, NEW_CLI;
    struct sockaddr_in Address;
    int AddrLen = sizeof(Address);

    if((SERVER = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        Log("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    opt = 1;
    if (setsockopt(SERVER, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        Log("Setting socket options failed");
        exit(EXIT_FAILURE);
    }

    Address.sin_family = AF_INET;
    Address.sin_addr.s_addr = INADDR_ANY;
    Address.sin_port = htons(atoi(PORT));

    if (bind(SERVER, (struct sockaddr *)&Address, sizeof(Address)) < 0) {
        Log("Bind failed, check if the port is in use");
        exit(EXIT_FAILURE);
    }

    if (listen(SERVER, 10) < 0) {
        Log("Listen failed");
        exit(EXIT_FAILURE);
    }


    Log("Listening on port -> %s", PORT);


    while(1) {
        if ((NEW_CLI = accept(SERVER, (struct sockaddr *)&Address, (socklen_t *)&AddrLen)) < 0) {
            Log("Accept failed");
        }
        Log("New client connected.");
        
        pthread_t tid;
        if (pthread_create(&tid, NULL, StartService, &NEW_CLI) != 0) {
            Log("Can't start new thread.");
        }

        pthread_detach(tid);
    }

    close(SERVER);
    return 0;
}