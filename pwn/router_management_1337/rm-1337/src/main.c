#include "../include/headers.h"

int main() {
    InitMethods();    

    if(!Config()){
        Log("ERROR", "Can't parse config file, exiting...");
        exit(1);
    } else {
        Log("INFO", "Network configuration -> %s:%d", HOST, PORT);
    }
    
    GettingDeviceInfo();

    AddSeccompRules();
    InitSockets();
    
    while(1) {
        if ((NEW_CLI = accept(SERVER, (struct sockaddr *)&Address, (socklen_t *)&AddrLen)) < 0) {
            Log("ERROR", "Accept failed");
            continue;
        }
        Log("INFO", "New client connected.");

        pthread_t tid;
        if (pthread_create(&tid, NULL, HandleClient, (void *)&NEW_CLI) != 0) {
            Log("ERROR", "Can't start new thread.");
            close(NEW_CLI);
            continue;
        }

        pthread_detach(tid);
        continue;
    }

    Log("INFO", "FOR SOME REASONE THE WHILE BREAK");
    SocketsCleaning();
    
    return 0;
}