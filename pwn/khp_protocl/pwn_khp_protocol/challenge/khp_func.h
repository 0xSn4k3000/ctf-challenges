#include <malloc.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define CMD_BUFFER 256
#define KEY_BUFFER 84

int cli, AVAIL_ID;
char *KEYS_DB_FILE = "users.keys";
char *KEYS_BUF = 0;

char *IN_MEM_KEYS[10];

char *CURRENT_USER, *CURRENT_ROLE;
char *CURRENT_PROFILE;

void Log(char *logs, ...) {
    time_t currentTime;
    struct tm *timeInfo;
    char asciiTime[26];

    time(&currentTime);
    timeInfo = localtime(&currentTime);
    asctime_r(timeInfo, asciiTime);
    
    asciiTime[24] = '\0';

    va_list args;
    va_start(args, logs);
    printf("[ %s ] : ", asciiTime);
    vprintf(logs, args);
    printf("\n");
    va_end(args);

}

void Send(const char *text, ...) {
    char final[512];
    va_list args;
    va_start(args, text);
    vsnprintf(final, sizeof(final), text, args);
    send(cli, final, strlen(final), 0);
    va_end(args);
}

void Read(char *buffer) {
    int bytesRecv;
    if( (bytesRecv = recv(cli, buffer , CMD_BUFFER , 0)) < 0){
		Log("recv failed");
	}
    buffer[bytesRecv + 1] = '\0';
}

int LoadKeysDB() {
    int file = open(KEYS_DB_FILE, O_RDONLY);
    struct stat file_stat;

    if (file == -1) {
        Log("Failed to open the keys file");
        return 0;
    }

    if (fstat(file, &file_stat) == -1) {
        Log("Error getting file information");
        close(file);
        return 0;
    }

    off_t size = file_stat.st_size;

    KEYS_BUF = (char *)malloc(size + 1);
    if(KEYS_BUF == NULL) {
        Log("Failed to allocate memory");
        close(file);
        return 0;
    }

    size_t bytesRead = read(file, KEYS_BUF, size);
    if (bytesRead != size) {
        Log("Failed to read the file");
        free(KEYS_BUF);
        close(file);
        return 0;
    }

    KEYS_BUF[size] = '\0';
    close(file);
    Log("Database loaded successfuly");
    return 1;
}

int GetFreeId() {
    int i, id;
    for(i = 1; i <= 10 ; i++) {
        if (IN_MEM_KEYS[i] == 0) {
            return i;
        }
    }
    return 0;
}

void RegisterNewKey(char *cmd) {
    char *token, *USER, *ROLE, *KEY;
    int size;

    token = strtok(cmd, " ");

    token = strtok(NULL, ":");
    USER = token;

    token = strtok(NULL, " ");
    ROLE = token;

    token = strtok(NULL, ";");
    KEY = token;
  
    AVAIL_ID = GetFreeId();
    if(AVAIL_ID == 0) {
        Send("No more available keys, please delete one or get an enterprise version.\n");
    } else {
        IN_MEM_KEYS[AVAIL_ID] = (char *)malloc(KEY_BUFFER);

        size = malloc_usable_size(IN_MEM_KEYS[AVAIL_ID]);
        size++;

        sprintf(IN_MEM_KEYS[AVAIL_ID], "%s:%s %s;", USER, ROLE, KEY);

        IN_MEM_KEYS[AVAIL_ID][strlen(IN_MEM_KEYS[AVAIL_ID])] = '\x00';

        Log("New user registered -> %s", USER);
        Log("New key registered with the id -> %d", AVAIL_ID);
        Send("Registered: ID->%d\n", AVAIL_ID);
    }
}

void DeleteKey(char *cmd) {
    char *token;
    int ID;
    token = strtok(cmd, " ");
    ID = atoi(strtok(NULL, ""));


    if(IN_MEM_KEYS[ID] != 0){
        free(IN_MEM_KEYS[ID]);
        IN_MEM_KEYS[ID] = 0x0;
        Send("Key deleted successfuly. \n");
        Log("Key deleted -> %d", ID);
    } else {
        Send("No key to delete. \n"); 
    }

}

void DeleteKeyFromDB(char *cmd) {
    char *token;
    int ID;
    token = strtok(cmd, " ");
    token = strtok(NULL, "");

    if(token) {
        ID = atoi(token);
    } else {
        Send("Id must be between 1 - 10. \n");
        return;
    }


    if(IN_MEM_KEYS[ID] != 0){
        if(KEYS_BUF == 0) {
            LoadKeysDB();
        }

        token = strstr(KEYS_BUF, IN_MEM_KEYS[ID]);
        if(token) {

            size_t len = strlen(IN_MEM_KEYS[ID]) + 1;
            memmove(token, (token + len), strlen(token + len) + 1);

            int file, bytesWrit;
            file = open(KEYS_DB_FILE, O_WRONLY | O_TRUNC);
            
            if (file == -1) {
                Log("Failed to open the keys file");
                Send("Error.");
                return;
            }

            bytesWrit = write(file, KEYS_BUF, strlen(KEYS_BUF));
            write(file, "\n", 1);
            if (bytesWrit == -1) {
                Log("Error writing to the file");
                Send("Error.");
                close(file);
                return;
            }

            Send("Deleted. \n");
            Log("Key deleted from database-> %d", ID);
            close(file);
        } else {
            Send("Key doesn't exist in the database. \n");
        }

    } else {
        Send("No key to delete. \n"); 
    }

}

int CheckIfKeyExist(int id) {
    IN_MEM_KEYS[id][strlen(IN_MEM_KEYS[id])] = '\x00';

    if (KEYS_BUF == 0) {
        LoadKeysDB();
    }

    if(strstr(KEYS_BUF, IN_MEM_KEYS[id]) != NULL) {
        return 1;
    }
    return 0;
}

void SaveKey(char *cmd) {
    char *token;
    int ID, file, bytesWrit;
    token = strtok(cmd, " ");
    token = strtok(NULL, "");

    if(token) {
        ID = atoi(token);
    } else {
        Send("Id must be between 1 - 10. \n");
        return;
    }
    

    if(ID >= 0 && ID <= 10) {
        if(IN_MEM_KEYS[ID] == 0) {
            Send("There is no key with the id -> %d \n", ID);
        } else {
            if(strstr(IN_MEM_KEYS[ID], "admin")) {
                Send("You can't save keys with admin role from here.\n");
                return;
            }
            if(CheckIfKeyExist(ID)) {
                Send("Key already exist in the database. \n");
                return;
            }

            file = open(KEYS_DB_FILE, O_WRONLY | O_APPEND);
            
            if (file == -1) {
                Log("Failed to open the keys file");
                Send("Error.");
                return;
            }

            bytesWrit = write(file, IN_MEM_KEYS[ID], strlen(IN_MEM_KEYS[ID]));
            write(file, "\n", 1);
            if (bytesWrit == -1) {
                Log("Error writing to the file");
                Send("Error.");
                close(file);
                return;
            }

            Send("Saved \n");
            close(file);
        }
    }
}

void Auth(char *cmd) {
    char *token;
    int ID, file, bytesWrit;
    token = strtok(cmd, " ");
    ID = atoi(strtok(NULL, ""));

    if(ID >= 0 && ID <= 10) {
        if(IN_MEM_KEYS[ID] == 0) {
            Send("There is no key with the id -> %d \n", ID);
        } else if(CheckIfKeyExist(ID)) {

            char buffer[128];
            char *token;
            strncpy(buffer, IN_MEM_KEYS[ID], strlen(IN_MEM_KEYS[ID])); 

            token = strtok(buffer, ":");
            CURRENT_USER = token;

            token = strtok(NULL, " ");
            CURRENT_ROLE = token;

            
            CURRENT_PROFILE = &(*IN_MEM_KEYS[ID]);

            Send("Authenticated with id -> %d \nUser: %s:%s \n", ID, CURRENT_USER, CURRENT_ROLE);
        } else {
            Send("Key should be exist in the database to use it for authentication. \n");
        }
    } else {
        Send("Id should be between 1-10 only.\n");
    }
}

void ReLoadDB() {
    if(KEYS_BUF != 0) {
        free(KEYS_BUF);
    }
    if(LoadKeysDB()) {
        Send("DB reloaded successfuly.\n");
    } else {
        Send("DB can't be reloaded. \n");
    }
}


void GetCurrentProfile() {
    char buffer[128];
    char *token;
    if(CURRENT_PROFILE != 0) {
        strncpy(buffer, CURRENT_PROFILE, KEY_BUFFER); 
        Send("Profile: %s \n", buffer);
    } else {
        Send("You need to be authenticated. \n");
    }
}

void GetAShell() {
    if(CURRENT_PROFILE != 0) {
        if (strcmp(CURRENT_ROLE, "admin") == 0) {
            Send("You can run commands now.\n$ ");
            dup2(cli, STDIN_FILENO);
            dup2(cli, STDOUT_FILENO);
            dup2(cli, STDERR_FILENO);

            execlp("/bin/sh", "/bin/sh", NULL);
        } else {
            Send("You need to be authenticated as admin to run this command \n");
        }
    } else {
        Send("You need to be authenticated. \n");
    }
}


void SendHelp() {
    char *HelpList = 
    "\nHello in Keys Holding Protocol Server. \n"
    "Available Commands: \n"
    "\tREKE: Register new user:key. \n"
    "\tDEKE: Delete key, usage: DEKE ID (e.g DEKE 5) \n"
    "\tDDKE: Delete key from database, usage: DDKE ID (e.g DDKE 5) \n"
    "\tSAVE: To save a key, usage: SAVE ID (e.g SAVE 5) \n"
    "\tAUTH: Authenticate with rgistered user:key, usage: AUTH ID (e.g AUTH 5) \n"
    "\tGTPR: Get current profile. \n"
    "\tRLDB: Reload the database. \n"
    "\tEXEC: Open a shell. (Only for Admins). \n"
    "\tHELP: To show this message. \n"
    "\tEXIT: To exit. \n";

    Send(HelpList);
}

void handle_signal(int signal) {
    close(cli);
    pthread_exit(NULL);
}

void *StartService(void *arg) {
    signal(SIGPIPE, handle_signal);

    cli = *(int *)arg;

    char CMD[CMD_BUFFER];

    while(1) {
        memset(CMD, 0, CMD_BUFFER);
        Read(CMD);
        if(strncmp(CMD, "HELP", 4) == 0){
            SendHelp();
        } else if(strncmp(CMD, "REKE", 4) == 0){
            RegisterNewKey(CMD);
        } else if(strncmp(CMD, "DEKE", 4) == 0){
            DeleteKey(CMD);
        } else if(strncmp(CMD, "DDKE", 4) == 0){
            DeleteKeyFromDB(CMD);
        } else if(strncmp(CMD, "SAVE", 4) == 0){
            SaveKey(CMD);
        } else if(strncmp(CMD, "AUTH", 4) == 0){
            Auth(CMD);
        } else if(strncmp(CMD, "GTPR", 4) == 0){
            GetCurrentProfile();
        } else if(strncmp(CMD, "RLDB", 4) == 0){
            ReLoadDB();
        } else if(strncmp(CMD, "EXEC", 4) == 0){
            GetAShell();
        } else if(strncmp(CMD, "EXIT", 4) == 0) {
            break;
        } else {
            Send("Unknown command.\n");
        }
    }
    
    close(cli);
    pthread_exit(NULL);
}