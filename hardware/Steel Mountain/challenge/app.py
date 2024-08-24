#!/usr/bin/python3

from time import sleep, perf_counter
from random import uniform, randint
from BAC0.core.devices.local.object import ObjectFactory
from bacpypes.object import ReadableProperty
from bacpypes.primitivedata import CharacterString
from BAC0 import connect, device
from BAC0.core.devices.local.models import (
    analog_input,
    analog_output,
    binary_output,
    binary_input, 
    multistate_output,
    multistate_input, 
)

from os import _exit

TSR_DOOR_LOCKED = False
ELEVATORS_COND = True
TAPES_DAMAGED = False

FLAG = "HTB{b4cn3t_!5_Fun_4nd_D4nger0u5}"

CLIENTS = []

import threading
from flask import Flask, render_template, jsonify

from socket import *

def def_objects(device):
    ObjectFactory.clear_objects()

    # LEVEL 1
    _new_objects = analog_input(
        instance=10,
        name="Temp-L1-10",
        properties={"units": "degreesCelsius"},
        description="FL-1-10",
        presentValue=21,
        
    )
    analog_output(
        instance=11,
        name="Therm-L1-11",
        properties={"units": "degreesCelsius"},
        description="FL-1-11",
        presentValue=22,
    )
    binary_output(
        instance=12,
        name="ACS-L1-12",
        description="Air Conditioning Stat-L1-13 : [0: OFF , 1: ON]",
        presentValue=1,
    )
    analog_output(
        instance=13,
        name="OHAP-L1-13",
        description="Over Heat Alarm Point, FL-1, 30°C",
        presentValue=30,
    )
    binary_input(
        instance=14,
        name="OHA-L1-14",
        description="[0: ALARM OFF , 1: ALARM ON]",
        presentValue=0,
    )
   
    # LEVEL 2
    analog_input(
        instance=20,
        name="Temp-L2-20",
        properties={"units": "degreesCelsius"},
        description="FL-2-20",
        presentValue=21,
        
    )
    analog_output(
        instance=21,
        name="Therm-L2-21",
        properties={"units": "degreesCelsius"},
        description="FL-2-21",
        presentValue=22,
    )
    binary_output(
        instance=22,
        name="ACS-L2-22",
        description="Air Conditioning Stat-L2-23 : [0: OFF , 1: ON]",
        presentValue=1,
    )
    analog_output(
        instance=23,
        name="OHAP-L2-23",
        description="Over Heat Alarm Point , FL-2 , 25°C",
        presentValue=30,
    )
    binary_input(
        instance=24,
        name="OHA-L2-24",
        description="[0: ALARM OFF , 1: ALARM ON]",
        presentValue=0,
    )
    # LEVEL 3

    analog_input(
        instance=30,
        name="Temp-L3-30",
        properties={"units": "degreesCelsius"},
        description="FL-3-30",
        presentValue=21,
        
    )
    analog_output(
        instance=31,
        name="Therm-L3-31",
        properties={"units": "degreesCelsius"},
        description="FL-3-31",
        presentValue=22,
    )
    binary_output(
        instance=32,
        name="ACS-L3-32",
        description="Air Conditioning Stat-L3-33 : [0: OFF , 1: ON]",
        presentValue=1,
    )
    analog_output(
        instance=33,
        name="OHAP-L3-33",
        description="Over Heat Alarm Point , FL-3 , 21°C",
        presentValue=21,
    )
    binary_input(
        instance=34,
        name="OHA-L3-34",
        description="[0: ALARM OFF , 1: ALARM ON]",
        presentValue=0,
    )



    # Air Handling Units (AHU)
    # AHU 1

    multistate_input(
        instance=201,
        name="AHU1",
        description="Roof Air Handling Unit 1, [0: OFF , 1: ON-COOLING, 2: ON-WARMING]",
        properties={
            "presentValue": 1,
            "stateText": ["OFF", "ON-COOLING", "ON-WARMING"],
        }
    )
    # AHU 2

    multistate_input(
        instance=202,
        name="AHU2",
        description="Roof Air Handling Unit 2, [0: OFF , 1: ON-COOLING, 2: ON-WARMING]",
        properties={
            "presentValue": 1,
            "stateText": ["OFF", "ON-COOLING", "ON-WARMING"],
        }
    )


    # Elevators

    # E1
    analog_output(
        instance=80,
        name="ELE-1-CF",
        description="",
        presentValue=1,
    )

    multistate_input(
        instance=81,
        name="ELE-1-STAT",
        description="[current_floor(ELE-1-CF), status[0: stop, 1: moving_up, 2:moving_down] , target_floor(ELE-1-TF)]",
        properties={
            "presentValue": 0,
            "stateText": ["STOP", "MOVING-UP", "MOVING-DOWN"],
        }
    )

    analog_output(
        instance=82,
        name="ELE-1-TF",
        description="",
        presentValue=1,
    )

    # E2
    analog_output(
        instance=83,
        name="ELE-2-CF",
        description="",
        presentValue=1,
    )

    multistate_input(
        instance=84,
        name="ELE-2-STAT",
        description="[current_floor(ELE-2-CF), status[0: stop, 1: moving_up, 2:moving_down] , target_floor(ELE-2-TF)]",
        properties={
            "presentValue": 0,
            "stateText": ["STOP", "MOVING-UP", "MOVING-DOWN"],
        }
    )

    analog_output(
        instance=85,
        name="ELE-2-TF",
        description="",
        presentValue=1,
    )


    # Doors
    multistate_output(
        instance=101,
        name="L1-LOBBY-DR",
        description="[0: OPEN , 1: CLOSED-UNLOCKED, 2: CLOSED-LOCKED]",
        properties={
            "presentValue": 1,
            "stateText": ["OPEN", "CLOSED-UNLOCKED", "CLOSED-LOCKED"],
        }
    )

    multistate_output(
        instance=102,
        name="L2-TSR-DR",
        description="[0: OPEN , 1: CLOSED-UNLOCKED, 2: CLOSED-LOCKED]",
        properties={
            "presentValue": 1,
            "stateText": ["OPEN", "CLOSED-UNLOCKED", "CLOSED-LOCKED"],
        }
    )

    multistate_output(
        instance=103,
        name="L3-SR-DR",
        description="[0: OPEN , 1: CLOSED-UNLOCKED, 2: CLOSED-LOCKED]",
        properties={
            "presentValue": 2,
            "stateText": ["OPEN", "CLOSED-UNLOCKED", "CLOSED-LOCKED"],
        }
    )

    binary_output(
        instance=500,
        name="Message",
        description="",
        presentValue=0
    )

    return _new_objects.add_objects_to_application(device)


def FloorOne():
    global device1

    while True:

        current_temp = device1["Temp-L1-10"].presentValue
        setpoint = device1["Therm-L1-11"].presentValue
        
        LobbyDoor = device1["L1-LOBBY-DR"]
        r = randint(0, 10)
        
        if r == 1:
            LobbyDoor.presentValue = 0
        elif r == 8:
            LobbyDoor.presentValue = 0

    
        if current_temp < setpoint:
            inc = round(uniform(0, 0.5), 1)

            if (current_temp + (inc * 2)) < setpoint:
                device1["Temp-L1-10"].presentValue += round((inc + 1), 1)

            device1["Temp-L1-10"].presentValue += inc
        else:

            # raise alarm if temp > 30.
            if current_temp > device1["OHAP-L1-13"].presentValue:
                device1["OHA-L1-14"].presentValue = 1
                device1["Message"].description = CharacterString("You have raised an alarm, mission failed, try again in 10 seconds")
                sleep(3)
                device1["Therm-L1-11"].presentValue = 19.0
                sleep(10)
                Exit()


            if device1["ACS-L1-12"].presentValue == 0:
                device1["ACS-L1-12"].presentValue = 1;
    
            if device1["ACS-L1-12"].presentValue == 1:
                dec = round(uniform(0, 1), 1)

                if (current_temp - (dec + 1)) > setpoint:
                    device1["Temp-L1-10"].presentValue -= round((dec + 1), 1)

                device1["Temp-L1-10"].presentValue -= dec
        
        device1["Temp-L1-10"].presentValue = float("{:.1f}".format(device1["Temp-L1-10"].presentValue))

        sleep(3)
        LobbyDoor.presentValue = 1


def FloorTwo():
    global device1, TAPES_DAMAGED, TSR_DOOR_LOCKED, FLAG

    Total = 0
    start_time = 0

    while True:

        current_temp = device1["Temp-L2-20"].presentValue
        setpoint = device1["Therm-L2-21"].presentValue
        
        if current_temp < setpoint:
            inc = round(uniform(0, 0.5), 1)

            if (current_temp + (inc * 2)) < setpoint:
                device1["Temp-L2-20"].presentValue += round((inc + 1), 1)

            device1["Temp-L2-20"].presentValue += inc
        else:

            # raise alarm if temp > 30.
            if current_temp > device1["OHAP-L2-23"].presentValue:
                device1["OHA-L2-24"].presentValue = 1
                device1["Message"].description = CharacterString("You have raised an alarm, mission failed, try again in 10 seconds")
                sleep(3)
                device1["Therm-L2-21"].presentValue = 19.0
                sleep(10)
                Exit()
                


            if device1["ACS-L2-22"].presentValue == 0:
                device1["ACS-L2-22"].presentValue = 1;
    
            if device1["ACS-L2-22"].presentValue == 1:
                dec = round(uniform(0, 1), 1)

                if (current_temp - (dec + 1)) > setpoint:
                    device1["Temp-L2-20"].presentValue -= round((dec + 1), 1)

                device1["Temp-L2-20"].presentValue -= dec
        
        device1["Temp-L2-20"].presentValue = float("{:.1f}".format(device1["Temp-L2-20"].presentValue))

        if(device1["L2-TSR-DR"].presentValue == 2):
            TSR_DOOR_LOCKED = True


        if(current_temp > 32):
            if(TSR_DOOR_LOCKED):
                if(start_time == 0):
                    start_time = perf_counter()
                else:
                    Total += (perf_counter() - start_time)
            else:
                device1["Therm-L2-21"].presentValue = 19.0

        else:
            start_time = 0
        

        if(Total > 120):
            TAPES_DAMAGED = True          

            if(TAPES_DAMAGED and TSR_DOOR_LOCKED and ELEVATORS_COND):
                device1["Message"].description = CharacterString(FLAG)

        sleep(3)


def FloorThree():
    global device1

    while True:

        current_temp = device1["Temp-L3-30"].presentValue
        setpoint = device1["Therm-L3-31"].presentValue
        
        if current_temp < setpoint:
            inc = round(uniform(0, 0.5), 1)

            if (current_temp + (inc * 2)) < setpoint:
                device1["Temp-L3-30"].presentValue += round((inc + 1), 1)

            device1["Temp-L3-30"].presentValue += inc
        else:

            # raise alarm if temp > 30.
            if current_temp > device1["OHAP-L3-33"].presentValue:
                device1["OHA-L3-34"].presentValue = 1
                device1["Message"].description = CharacterString("You have raised an alarm, mission failed, try again in 10 seconds")
                sleep(3)
                device1["Therm-L3-31"].presentValue = 19.0
                sleep(10)
                Exit()


            if device1["ACS-L3-32"].presentValue == 0:
                device1["ACS-L3-32"].presentValue = 1;
    
            if device1["ACS-L3-32"].presentValue == 1:
                dec = round(uniform(0, 1), 1)

                if (current_temp - (dec + 1)) > setpoint:
                    device1["Temp-L3-30"].presentValue -= round((dec + 1), 1)

                device1["Temp-L3-30"].presentValue -= dec
        
        device1["Temp-L3-30"].presentValue = float("{:.1f}".format(device1["Temp-L3-30"].presentValue))

        sleep(3)



def Elevators():
    global device1, TSR_DOOR_LOCKED, ELEVATORS_COND
    

    e1st = device1["ELE-1-STAT"]
    e1cf = device1["ELE-1-CF"]
    e1tf = device1["ELE-1-TF"]
    
    e2st = device1["ELE-2-STAT"]
    e2cf = device1["ELE-2-CF"]
    e2tf = device1["ELE-2-TF"]
    
    e1st.presentValue = 0
    e1cf.presentValue = 1
    e1tf.presentValue = 1

    e2st.presentValue = 0
    e2cf.presentValue = 1
    e2tf.presentValue = 1

    while True:
        e1_n = randint(1, 3)
        e2_n = randint(1, 3)

        e1tf.presentValue = e1_n
        e1st.presentValue = 0

        e2tf.presentValue = e2_n
        e2st.presentValue = 0

        sleep(5)

        # status[0: stop, 1: moving_up, 2:moving_down]
        # E1
        if e1tf.presentValue > e1cf.presentValue:
            e1st.presentValue = 1
        elif e1tf.presentValue < e1cf.presentValue:
            e1st.presentValue = 2
        else:
            e1st.presentValue = 0

        # E2
        if e2tf.presentValue > e2cf.presentValue:
            e2st.presentValue = 1
        elif e2tf.presentValue < e2cf.presentValue:
            e2st.presentValue = 2
        else:
            e2st.presentValue = 0

        sleep(10)

        e1st.presentValue = 0
        e1cf.presentValue = e1tf.presentValue

        e2st.presentValue = 0
        e2cf.presentValue = e2tf.presentValue

        if(TSR_DOOR_LOCKED):
            if(e1cf.presentValue == 2 and e1st.presentValue == 0 and e1tf.presentValue == 2):
                ELEVATORS_COND = False
                
            if(e2cf.presentValue == 2 and e2st.presentValue == 0 and e2tf.presentValue == 2):
                ELEVATORS_COND = False

        sleep(5)


def initObjects(device1):    
    device1["Message"].presentValue = 0 
    # LEVEL 1
    device1["Temp-L1-10"].presentValue = 21.0
    device1["Therm-L1-11"].presentValue = 22.0
    device1["ACS-L1-12"].presentValue = 1
    device1["OHAP-L1-13"].presentValue = 30.0
    device1["OHA-L1-14"].presentValue = 0

    # LEVEL 2
    device1["Temp-L2-20"].presentValue = 21.0
    device1["Therm-L2-21"].presentValue = 19.0
    device1["ACS-L2-22"].presentValue = 1
    device1["OHAP-L2-23"].presentValue = 25
    device1["OHA-L2-24"].presentValue = 0

    # LEVEL 3
    device1["Temp-L3-30"].presentValue = 21.0
    device1["Therm-L3-31"].presentValue = 19.0
    device1["ACS-L3-32"].presentValue = 1
    device1["OHAP-L3-33"].presentValue = 21.0
    device1["OHA-L3-34"].presentValue = 0

    # Air Handling Units (AHU)
    device1["AHU1"].presentValue = 1
    device1["AHU2"].presentValue = 1

    # Elevators
    device1["ELE-1-CF"].presentValue = 1
    device1["ELE-1-STAT"].presentValue = 0
    device1["ELE-1-TF"].presentValue = 1
    device1["ELE-2-CF"].presentValue = 1
    device1["ELE-2-STAT"].presentValue = 0
    device1["ELE-2-TF"].presentValue = 1

    # Doors
    device1["L1-LOBBY-DR"].presentValue = 1
    device1["L2-TSR-DR"].presentValue = 1
    device1["L3-SR-DR"].presentValue = 2


def getObjects():
    global device1

    objs = ObjectFactory.objects

    final = {}
    for objname in objs:
        obj = objs[objname]

        final[objname] = { 
            "ObjectName"  : objname,
            "ObjectID"    : obj.objectIdentifier[1],
            "ObjectType"  : obj.objectType,
            "Description"  : obj.description.value,
            "presentValue" : obj.presentValue
        }

    return final


def Read(cmd):
    global device1

    ObjType  = cmd[0]
    InstId   = cmd[1]
    PropName = cmd[2]

    objs = ObjectFactory.objects

    for objname in objs:
        obj = objs[objname]

        if (str(obj.objectIdentifier[1]) == InstId) and (str(obj.objectType) == ObjType):
            if hasattr(obj, PropName):
                return str(getattr(obj, PropName))
            else:
                return "Unknown property: " + PropName

    return "There is no object with this objecType/objectId"


def Write(cmd):
    global device1
    writeable_objects = ["analogOutput", "analogValue", "binaryOutput", "binaryValue", "multiStateOutput", "multiStateValue"]

    ObjType  = cmd[0]
    InstId   = cmd[1]
    PropName = cmd[2]
    Value    = cmd[3]

    objs = ObjectFactory.objects

    for objname in objs:
        obj = objs[objname]

        if (str(obj.objectIdentifier[1]) == InstId) and (str(obj.objectType) == ObjType):
            if(str(obj.objectType) in writeable_objects):
                if hasattr(obj, PropName):
                    prop_type = type(getattr(obj, PropName))

                    if(setattr(obj, PropName, prop_type(Value)) == None):
                        return "True"
                    else:
                        return "False"
                else:
                    return "Unknown property: " + PropName
            
            else:
                return "This object is not writeable"

    return "There is no object with this objectType/objectId"


sock = None

def ListenToClients():
    global sock
    HOST = "0.0.0.0"
    PORT = 5001

    sock = socket(AF_INET, SOCK_STREAM)
    sock.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)
    sock.bind((HOST, PORT))
    sock.listen()
    print(f"Server is listening on {HOST}:{PORT}")

    try:
        while True:
            cli, client_address = sock.accept()
            CLIENTS.append(cli)
            hc = threading.Thread(target=HandleClient, args=(cli,))
            hc.start()
            
    except:
        sock.close()


def HandleClient(cli):
    prompt = f"""

1. objects
2. bacnet.read
3. bacnet.write

>> """

    read_prompt = b"""
usage: 
>>> object_type object_id property_name

>>> """

    write_prompt = b"""
usage: 
>>> object_type object_id property_name value

>>> """
    while True:
        try:
            cli.send(prompt.encode())
            msg = cli.recv(4)
            opt = int(msg.decode().rstrip("\n"))
            
            if opt == 1:
                cli.send(str(getObjects()).encode())
            
            elif opt == 2:
                cli.send(read_prompt)
                msg = cli.recv(512)
                cmd = msg.decode().rstrip("\n").split(" ")
        
                try:
                    result = Read(cmd)            
                    cli.send(result.encode())
                except Exception as e:
                    print(str(e))
            
            elif opt == 3:
                cli.send(write_prompt)
                msg = cli.recv(512)
                cmd = msg.decode().rstrip("\n").split(" ")
        
                try:
                    result = Write(cmd)            
                    cli.send(result.encode())
                except Exception as e:
                    print(str(e))


        except Exception as e:
            print(str(e))
            break
    
    cli.close()


def Exit():
    global sock, CLIENTS
    
    sock.close()

    for cli in CLIENTS:
        try:
            cli.close()
        except Exception as e:
            print(str(e))

    _exit(0)
    


def WebApp():
    app = Flask(__name__)

    @app.route("/")
    def index():
        return render_template("index.html")

    @app.route("/data")
    def data():
        return jsonify(getObjects())


    app.run(host="0.0.0.0", debug=False)
  

device1 = None

def start():
    global device1
    device1 = connect("127.0.0.1/16")
    def_objects(device1)
    initObjects(device1)

    F1 = threading.Thread(target=FloorOne)
    F1.start()

    F2 = threading.Thread(target=FloorTwo)
    F2.start()

    F3 = threading.Thread(target=FloorThree)
    F3.start()

    ELE = threading.Thread(target=Elevators)
    ELE.start()

    ltc = threading.Thread(target=ListenToClients)
    ltc.start()

    # The web dashboard
    webapp = threading.Thread(target=WebApp)
    webapp.start()

    # ...
    

start()