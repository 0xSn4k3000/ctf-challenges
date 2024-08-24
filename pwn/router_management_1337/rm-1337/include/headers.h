#ifndef LOADED
#define LOADED

    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <unistd.h>
    #include <stdint.h>
    #include <stdbool.h>
    #include <stdarg.h> 
    #include <stddef.h>
    #include <ctype.h>
    #include <malloc.h>
    #include <time.h>

    #include <fcntl.h>
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <syscall.h>

    #include <sys/sysinfo.h>
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <netinet/in.h>

    #include <signal.h>
    #include <pthread.h>
    
    #include <errno.h>

    #include <seccomp.h>

    #include "logging.h"
    #include "functions.h"
    #include "network.h"
    #include "config.h"

#endif