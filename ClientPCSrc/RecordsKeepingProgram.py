#!/usr/bin/env python
import serial
import re
import string
import tkinter
from tkinter import *
import threading
import time

ser = None
rows = []
cols = []


def headers(c, message):    
    Label(master, text=message).grid(column=c, row=1)

master = Tk()    
entryGrid = ""
numberToPop = []

#data ="1,45,33\n2,67,54\n3,0,0\n4,99,90\n5,0,0\n6,20,15\n7,53,33\n"
#data = ""
#data = data.splitlines()[:]
#data.pop(2)
#print(len(data))
#print()


'''
for x in range(len(data)) :
    temp = data[x].split('_')[:]
    print(temp[1] + " " + temp[2])
    if temp[1] == '0' and temp[2] == '0' :
        #print("got here")
        numberToPop.append(x)
'''
#print()
#for i in range(len(numberToPop)) :        
#    print(numberToPop[i])

for i in range(len(numberToPop)) :    
    data.pop(numberToPop[i] - i) 
    
#for i in range(len(data)) :
#    print(data[i].strip())

print("print_CSV function")

headers(0, 'Measurement Number')
headers(1, 'Active Angle Mesaurement(Degrees)')
headers(2, 'Passive Angle Measurement (Degrees)')

'''
rows = []
for i in range(len(data)): #Rows
    cols = []
    for j in range(3): #Columns
        entryGrid = Entry(master, relief=RIDGE)
        entryGrid.grid(row=i+2, column=j, sticky=NSEW)                        
        #e.insert(END, '%d.%d' % (i+1, j+1))
        temp = data[i].split(',')[:]
        entryGrid.insert(END, temp[j])
        entryGrid.config(background="white")
        #entryGrid.config(state=DISABLED)
        cols.append(entryGrid)
    rows.append(cols)
'''

def onPress():
    data = []
    
    fileName = en.get()
    f = open(fileName + ".csv", "w")        

    i = 0
    j = 0
    while i < len(rows) :
        while j < len(cols) :              
            data.append(cols[j].get())
            j = j + 1
        i = i + 1 
    
    x = 0
    y = 3
    z = 0
    length = len(data)/3
    print(length)
    while z < (len(data)/3):
        print(','.join(map(str, data[x:y])))
        f.write(','.join(map(str, data[x:y]))+ "\n")
        x = x + 3
        y = y + 3
        z = z + 1
        
    f.close()
    print(fileName)

        
def save() :   
    fileName = en.get()
    f = open(fileName + ".csv", "w")
    
    for i in range(len(data)) :
        f.write(data[i] + "\n")
    f.close()
    print(fileName)
    
def connect():
    global ser
    COMPort = en2.get()
    print("COMPORT={}".format(COMPort))
    ser = serial.Serial(port=COMPort, baudrate="9600", timeout=1)  # connect to serial COM port
    
    #number = re.sub("\w", "", str(port.readline())) # Read in the number from serial from the sonar device
        #if number and number.isdigit() : #Data check
        

def check_serial():
    global rows 
    global cols
    global printData
    can_start = False
    count = 0
    measureCount = 0
    while True:
        if ser:
            raw_str = str(ser.readline()).strip()
            filtered = ''.join(char_ for char_ in raw_str if (char_ in string.digits or char_ == '_' or char_ == 'S' or char_ == 'E'))
            printData = filtered
            print("the value of measureCount is : " + str(measureCount))
            #if "EEE" in filtered:
            if measureCount == 33 :
                print("data process has ended!")
                can_start = False
            if can_start:
                print("filt={}".format(filtered)) #KEVIN: add record adding functionality here
                #filtered should be something like "1 _280_149"
                temp = filtered.split('_')[:]
                #print(temp[1] + " " + temp[2])
                if not(temp[1] == '0' and temp[2] == '0') :
                    print("got here")
                    
                    #rows = []                    
                    #for i in range(len(data)): #Rows
                    
                    #cols = []
                    for j in range(3): #Columns
                        entryGrid = Entry(master, relief=RIDGE)
                        entryGrid.grid(row=count+2, column=j, sticky=NSEW)                        
                        #e.insert(END, '%d.%d' % (i+1, j+1))
                        #temp = data[count].split(',')[:]
                        entryGrid.insert(END, temp[j])
                        entryGrid.config(background="white")
                        #entryGrid.config(state=DISABLED)
                        cols.append(entryGrid)
                    rows.append(cols)
                    count = count + 1
            if "SSS" in filtered:
                print("data process has started!")
                can_start = True
            measureCount = measureCount + 1
        #print("hello world!")
        
#COM port connection
Label(master, text="COM Port:").grid(column=0, row=32)    
Button(master, text='Connect', command=connect).grid(column=2, row=32)
en2=Entry(master, width = 10)
en2.grid(column=1, row=32)

#Title
Label(master, text='Autogoni').grid(column=1, row= 0) 

#File management       
Button(master, text='Save', command=onPress).grid(column=2 ,row=34)
Label(master, text='Enter File Name:').grid(column=0, row=34)
en=Entry(master, width=60)
en.grid(row=34, column=1, sticky=W)

master.geometry("700x200")

t = threading.Thread(target=check_serial)
t.setDaemon(True) # It is a daemon
t.start()       



mainloop()
