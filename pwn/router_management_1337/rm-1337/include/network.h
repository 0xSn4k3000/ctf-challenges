#ifndef LOADED
#define LOADED

    #include "headers.h"

#endif



char HOST[100];
int PORT;

int SERVER, NEW_CLI;
struct sockaddr_in Address;
int AddrLen = sizeof(Address);

bool InitSockets() {
    Log("INFO", "Trying to initialize sockets.");
    if((SERVER = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        Log("ERROR", "Socket creation failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(SERVER, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        Log("ERROR", "Setting socket options failed");
        exit(EXIT_FAILURE);
    }

    if (inet_pton(AF_INET, HOST, &Address.sin_addr) <= 0) {
        Log("ERROR", "Failed to set the HOST, using 0.0.0.0");
        Address.sin_addr.s_addr = INADDR_ANY;
    }

    Address.sin_family = AF_INET;
    Address.sin_port = htons(PORT);


    if (bind(SERVER, (struct sockaddr *)&Address, sizeof(Address)) < 0) {
        Log("ERROR", "Bind failed, check if the port is in use");
        exit(EXIT_FAILURE);
    }

    if (listen(SERVER, 10) < 0) {
        Log("ERROR", "Listen failed");
        exit(EXIT_FAILURE);
    }

    Log("INFO", "Listening on port -> %d", PORT);
}

int Read(int cli, char *buffer) {
    int bytesRecv;
    if( (bytesRecv = recv(cli, buffer , BUFFER - 2, 0)) < 0){
		Log("ERROR", "recv failed");
	}
    buffer[bytesRecv + 1] = 0x00;
    return bytesRecv;
}


void Send(int cli, const char *format, ...) {
    char final[1024];
    memset(final, sizeof(final), 0);

    va_list args;
    va_start(args, format);
    vsnprintf(final, sizeof(final), format, args);
    send(cli, final, strlen(final), 0);
    
    va_end(args);    
}

void ThreadExit(int signal) {
    pthread_exit(NULL);
}

void *HandleClient(void *arg) {
    signal(SIGPIPE, ThreadExit);

    int CliSock = *(int *)arg;
    
    char CMD[BUFFER];
    char *PARSED[3];
    char *RES;

    int err = 0;
    socklen_t len = sizeof(err);

    while(!err) {
        if(getsockopt(CliSock, SOL_SOCKET, SO_ERROR, &err, &len) != 0){
            Log("ERROR", "getsockopt failed to get client status.");
        }

        memset(CMD, 0, strlen(CMD));

        if((Read(CliSock, CMD) <= 0)) {
            close(CliSock);
            break;
        }

        parseCommand(CMD, PARSED);
    
        if(PARSED != NULL) {
            if(strcmp(PARSED[1], "GetInformation") == 0) {
                
                if(Methods[GetInformationIndex].Enabled) {
                    GetInformation(&RES);
                    Send(CliSock, "[OK] \n\r%s \n\r", RES);
                    free(RES);
                } else {
                    Send(CliSock, "[ERROR] \n\rMethodDisabled->%s \n\r", PARSED[1]);
                }

            } else if (strcmp(PARSED[1], "ListAllowedMethods") == 0) {
                
                if(Methods[ListAllowedMethodsIndex].Enabled) {
                     ListAllowedMethods(&RES);
                    Send(CliSock, "[OK] \n\r%s \n\r", RES);
                    free(RES);
                } else {
                    Send(CliSock, "[ERROR] \n\rMethodDisabled->%s \n\r", PARSED[1]);
                }
               
            } else if (strcmp(PARSED[1], "ListAllMethods") == 0) {

                if(Methods[ListAllMethodsIndex].Enabled) {
                    ListAllMethods(&RES);
                    Send(CliSock, "[OK] \n\r%s \n\r", RES);
                    free(RES);
                } else {
                    Send(CliSock, "[ERROR] \n\rMethodDisabled->%s \n\r", PARSED[1]);
                }
            
            } else if (strcmp(PARSED[1], "GetParameterValue") == 0) {

                if(Methods[GetParameterValueIndex].Enabled) {
                    GetParameterValue(&RES, PARSED[2]);
                    Send(CliSock, "[OK] \n\r%s \n\r" , RES);
                    free(RES);
                } else {
                    Send(CliSock, "[ERROR] \n\rMethodDisabled->%s \n\r", PARSED[1]);
                }

            } else if (strcmp(PARSED[1], "SetParameterValue") == 0) {

                if(Methods[SetParameterValueIndex].Enabled) {
                    if(SetParameterValue(PARSED[2], CliSock)){
                        Send(CliSock, "[OK] \n\r");
                    } else {
                        Send(CliSock, "[ERROR] \n\r");
                    }
                } else {
                    Send(CliSock, "[ERROR] \n\rMethodDisabled->%s \n\r", PARSED[1]);
                }

            } else {
                Send(CliSock, "[ERROR] \n\rIncorrectCommand->%s \n\r", PARSED[1]); 
            }
        } else {
            Send(CliSock, "[ERROR] \n\rIncorrectCommand->%s \n\r", PARSED[1]); 
        }

        for(int i = 0; i < 4; i++) {
            if(PARSED[i]){
                memset(PARSED[i], 0, strlen(PARSED[i]));
            }
        }
    }

    Log("INFO", "Client disconnected.");
    close(CliSock);
    ThreadExit(0);
}

void SocketsCleaning() {
    if (SERVER) {
        close(SERVER);
    } 

    if (NEW_CLI) {
        close(NEW_CLI);
    }
    Log("INFO", "Cleaning up...");
}