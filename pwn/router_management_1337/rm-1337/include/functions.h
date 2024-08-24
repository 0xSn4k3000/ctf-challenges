#ifndef LOADED
#define LOADED

    #include "headers.h"

#endif

int Read(int cli, char *buffer);
void Send(int cli, const char *format, ...);

#define BUFFER 512

bool Config();
bool SaveConfig();

// Methods
struct Method {
    char *MethodName;
    char *MethodDescription;
    bool Enabled;
};
struct Method Methods[5];

#define MethodsCount 5

// Device information
#define WEP 1
#define WPA 2
#define WPA1 3
#define WPA2 4

struct Parameters {
    char SSID[256];
    int Encryption;
    char Password[256];
};

struct DeviceInfo {
    char Manufacturer[124]; 
    char Model[124];

    struct Parameters param;

    char Uptime[64];
    double TotalRam;
    double FreeRam;
};
struct DeviceInfo DevInfo;

#define ListAllowedMethodsIndex 0
#define ListAllMethodsIndex 1
#define SetParameterValueIndex 2
#define GetParameterValueIndex 3
#define GetInformationIndex 4

bool InitMethods() {
    Methods[0].MethodName = "ListAllowedMethods";
    Methods[0].MethodDescription = "List allowed to use methods.";
    Methods[0].Enabled = false;

    Methods[1].MethodName = "ListAllMethods";
    Methods[1].MethodDescription = "List all methods even disabled.";
    Methods[1].Enabled = false;

    Methods[2].MethodName = "SetParameterValue";
    Methods[2].MethodDescription = "Set parameter value.";
    Methods[2].Enabled = false;

    Methods[3].MethodName = "GetParameterValue";
    Methods[3].MethodDescription = "Get parameter value.";
    Methods[3].Enabled = false;

    Methods[4].MethodName = "GetInformation";
    Methods[4].MethodDescription = "Get device's information.";
    Methods[4].Enabled = false;
}

void AddSeccompRules() {
    Log("INFO", "Sandboxing the process...");
    
    scmp_filter_ctx ctx;
    ctx = seccomp_init(SCMP_ACT_ALLOW);

    if (seccomp_rule_add(ctx, SCMP_ACT_ERRNO(EPERM), SCMP_SYS(execve), 0) < 0) {
        perror("seccomp_rule_add");
        seccomp_release(ctx);
    }

    if (seccomp_rule_add(ctx, SCMP_ACT_ERRNO(EPERM), SCMP_SYS(execveat), 0) < 0) {
        perror("seccomp_rule_add");
        seccomp_release(ctx);
    }

    if (seccomp_rule_add(ctx, SCMP_ACT_ERRNO(EPERM), SCMP_SYS(open), 0) < 0) {
        perror("seccomp_rule_add");
        seccomp_release(ctx);
    }

    if (seccomp_rule_add(ctx, SCMP_ACT_ERRNO(EPERM), SCMP_SYS(openat), 0) < 0) {
        perror("seccomp_rule_add");
        seccomp_release(ctx);
    }

    if (seccomp_load(ctx) < 0) {
        perror("seccomp_load");
        seccomp_release(ctx);
    }

    seccomp_release(ctx);
    
    Log("INFO", "Filtering syscalls complete");
}

// Reading device information
int read_value_from_file(const char *filename, char *value) {
        FILE *fp;
        char buf[256];

        fp = fopen(filename, "r");
        if (fp == NULL) {
            Log("Error", "Can't opening file to read device information.");
            return -1;
        }

        if (fgets(buf, sizeof(buf), fp) == NULL) {
            Log("Error", "Can't read device information from file.");
            fclose(fp);
            return -1;
        }

        buf[strcspn(buf, "\n")] = '\0';
        strcpy(value, buf);
        fclose(fp);
        return 0;
};

void GettingDeviceInfo() {
    char manufacturer[124], model[124];
    struct sysinfo si;

    if (read_value_from_file("/sys/class/dmi/id/sys_vendor", manufacturer) != 0) {
        Log("ERROR", "Failed to read manufacturer information.");
    } else {
        strcpy(DevInfo.Manufacturer, manufacturer);
    }

    if (read_value_from_file("/sys/class/dmi/id/product_name", model) != 0) {
        Log("ERROR", "Failed to read model information.");
    } else {
        strcpy(DevInfo.Model, model);
    }

    if(sysinfo(&si) != 0){
        Log("ERROR", "Failed to read system information.");
    } else {
        char Uptime[100];
        snprintf(Uptime, sizeof(Uptime), "%ld:%ld:%ld", si.uptime/3600, si.uptime%3600/60, si.uptime%60);
        strcpy(DevInfo.Uptime, Uptime);

        unsigned long long total_ram_bytes = (unsigned long long)si.totalram * si.mem_unit;
        DevInfo.TotalRam = (double)total_ram_bytes / (1024 * 1024 * 1024);

        unsigned long long free_ram_bytes = (unsigned long long)si.freeram * si.mem_unit;
        DevInfo.FreeRam = (double)free_ram_bytes / (1024 * 1024 * 1024);
    }
}


// Commands parsing
void parseCommand(char *command, char *ParsedCommand[]) {
    const char *delimiter = ".";
    char *token;
    int i = 0;

    token = strtok(command, delimiter);
   
    while (token != NULL)
    {
        if(token[strlen(token) - 1] == '\n') {
            token[strlen(token) - 1] = '\0';
        }

        if (i < 3) {
            ParsedCommand[i] = token;
        }
        token = strtok(NULL, delimiter);
        i++;
    }
}


char *trim(char *str) {
    char* end;
    int st_ws, ed_ws;

    st_ws = 0;
    ed_ws = 0;

    for(int i = 0; i < strlen(str); i++) {
        if(str[i] == 0x20) {
            st_ws++;
        } else {
            break;
        }
    }

    for(int i = (strlen(str) - 1); i >= 0 ; i--) {
        if(str[i] == 0x20) {
            ed_ws++;
        } else {
            break;
        }
    }

    memmove(str, str+st_ws, strlen(str) - ed_ws);

    str[strlen(str) - (ed_ws + st_ws)] = 0x00;    

    return str;
}


// -------------------

void ListAllowedMethods(char **AllowedMethods) {
    char buffer[512]; 

    *AllowedMethods = (char *)malloc(512 * sizeof(char));
    if (*AllowedMethods == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return;
    }
    
    strcpy(*AllowedMethods, "[AllowedMethods]\n");

    for (int i = 0; i < MethodsCount; i++) {
        if (Methods[i].Enabled) {
            snprintf(buffer, sizeof(buffer), "%s : %s \n", Methods[i].MethodName, Methods[i].MethodDescription);
            strcat(*AllowedMethods, buffer);
        }
    }
}

void ListAllMethods(char **AllMethods) {
    char buffer[512]; 
    char *methodStat;

    *AllMethods = (char *)malloc(512 * sizeof(char));
    if (*AllMethods == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return;
    }
    
    strcpy(*AllMethods, "[Methods]\n");

    for (int i = 0; i < MethodsCount; i++) {
        if(Methods[i].Enabled) {
            methodStat = "Enabled";
        } else {
            methodStat = "Disabled";
        }

        snprintf(buffer, sizeof(buffer), "%s : %s : %s\n", Methods[i].MethodName, Methods[i].MethodDescription, methodStat);
        strcat(*AllMethods, buffer);
    }
}

void GetInformation(char **Information) {
    size_t size = 512 * sizeof(char);
    *Information = (char *)malloc(size);

    char *Format = 
    "[DeviceInformation]\n"
    "Manufacturer: %s \n"
    "Model: %s \n"
    "SSID: %s \n"
    "Encryption: %s \n"
    "Password: %s \n"
    "Uptime: %s \n"
    "Ram: %.2f GB \n"
    "Free Ram: %.2f GB \n";

    char *EncType;
    const char newline[2] = "\n";
    if(DevInfo.param.Encryption == WEP) {
        EncType = "WEP";
    } else if(DevInfo.param.Encryption == WPA) {
        EncType = "WPA";
    } else if(DevInfo.param.Encryption == WPA1) {
        EncType = "WPA1";
    } else if(DevInfo.param.Encryption == WPA2) {
        EncType = "WPA2";
    } else {
        EncType = "Unknown";
    }

    snprintf(*Information, size, Format, DevInfo.Manufacturer, DevInfo.Model, strtok(DevInfo.param.SSID, newline), EncType, strtok(DevInfo.param.Password, newline), DevInfo.Uptime, DevInfo.TotalRam, DevInfo.FreeRam);
}

void GetParameterValue(char **Value, char *param) {
    size_t size = 128 * sizeof(char);
    *Value = (char *)malloc(size);

    if(strcmp(param, "SSID") == 0) {
        snprintf(*Value, size, DevInfo.param.SSID); // Format String Vulnerability Here
    } else if(strcmp(param, "Encryption") == 0) {
        if(DevInfo.param.Encryption == WEP) {
            sprintf(*Value, "WEP");
        } else if(DevInfo.param.Encryption == WPA) {
            sprintf(*Value, "WPA");
        } else if(DevInfo.param.Encryption == WPA1) {
            sprintf(*Value, "WPA1");
        } else if(DevInfo.param.Encryption == WPA2) {
            sprintf(*Value, "WPA2");
        } else {
            sprintf(*Value, "Unknown");
        }
    } else if(strcmp(param, "Password") == 0) {
        snprintf(*Value, size, DevInfo.param.Password); // Format String Vulnerability Here
    }
}

bool SetParameterValue(char *param, int sock) {
    char buffer[465];
    char *value;

    Send(sock, "[WAITING] \n\r");

    Read(sock , buffer);
    value = trim(buffer);

    if(strcmp(param, "SSID") == 0) {
        strncpy(DevInfo.param.SSID, value, sizeof(DevInfo.param.SSID));
        return true;
    } else if(strcmp(param, "Encryption") == 0) {
        if(strcmp(value, "WEP") == 0) {
            DevInfo.param.Encryption = WEP;
        } else if(strcmp(value, "WPA") == 0) {
            DevInfo.param.Encryption = WPA;
        } else if(strcmp(value, "WPA1") == 0) {
            DevInfo.param.Encryption = WPA1;
        } else if(strcmp(value, "WPA2") == 0) {
            DevInfo.param.Encryption = WPA2;
        } else {
            DevInfo.param.Encryption = 0;
        }
        return true;
    } else if(strcmp(param, "Password") == 0) {
        strncpy(DevInfo.param.Password, value, sizeof(DevInfo.param.SSID));
        return true;
    } else {
        return false;
    }
    return true;
}