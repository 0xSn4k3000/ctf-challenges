#ifndef LOADED
#define LOADED

    #include "headers.h"

#endif

#define MAX_LINE_LEN 200
#define MAX_SECTION_LEN 100


bool Config() {
    FILE *file = fopen("config.cfg", "r");

    if(file == NULL) {
        Log("ERROR", "unable to open config file");
        return false;
    }
    
    Log("INFO", "reading config file");
    
    char line[MAX_LINE_LEN];
    char section[MAX_SECTION_LEN];
    char key[MAX_LINE_LEN];
    char value[MAX_LINE_LEN];

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = '\0';

        if(line[0] == '[' && line[strlen(line) - 1] == ']') {
            sscanf(line, "[%[^]]", section);
        } else if (strlen(line) == 0 || line[0] == '#') {
            continue; 
        } else {
            sscanf(line, "%[^=] = %[^\n]", key, value);
            sscanf(key, "%[^ \t]", key);

            if(strcmp(section, "METHODS") == 0) {
                for(int i = 0 ; i < MethodsCount ; i++) {
                    if((strcmp(Methods[i].MethodName, key) == 0) && (strcmp(value, "enabled") == 0)) {
                        Methods[i].Enabled = true;
                    }
                }
            } else if (strcmp(section, "NETWORK") == 0) {
                if(strcmp(key, "HOST") == 0) {
                    strncpy(HOST , value, 100);
                } else if(strcmp(key, "PORT") == 0) {
                    PORT = atoi(value);
                }
            } else if (strcmp(section, "DEVICE") == 0) {
                if(strcmp(key, "SSID") == 0) {
                    strncpy(DevInfo.param.SSID , value, 256);
                } else if(strcmp(key, "Encryption") == 0) {
                    sscanf(value, "%[^ \t]", value);
                    if(strcmp(value, "WEP") == 0) {
                        DevInfo.param.Encryption = WEP;
                    } else if (strcmp(value, "WPA") == 0){
                        DevInfo.param.Encryption = WPA;
                    } else if (strcmp(value, "WPA1") == 0){
                        DevInfo.param.Encryption = WPA1;
                    } else if (strcmp(value, "WPA2") == 0){
                        DevInfo.param.Encryption = WPA2;
                    }
                } else if(strcmp(key, "Password") == 0) {
                    char redacted[256];
                    for(int i = 0 ; i < strlen(value) ; i++) {
                        redacted[i] = '*';
                    }
                    strncpy(DevInfo.param.Password , redacted, strlen(redacted));
                    memset(value, 0, sizeof(value));
                }
            }

        }

    }

    return true;    
}