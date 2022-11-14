#!/usr/bin/env python

import datetime
import os
import sys
from threading import Thread
import serial
import time
import collections
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import struct
import copy
import pandas as pd

def clear(): 
    if os.name == "posix":
        os.system ("clear")
    elif os.name == ("ce", "nt", "dos"):
        os.system ("cls")

class serialPlot:
    def __init__(self, serialPort='/dev/ttyACM0', serialBaud=9600, plotLength=100, dataNumBytes=2, numPlots=1):
        self.port = serialPort
        self.baud = serialBaud
        self.plotMaxLength = plotLength
        self.dataNumBytes = dataNumBytes
        self.numPlots = numPlots
        self.rawData = bytearray(numPlots * dataNumBytes)
        self.dataType = None
        if dataNumBytes == 2:
            self.dataType = 'h'     # 2 byte integer
        elif dataNumBytes == 4:
            self.dataType = 'f'     # 4 byte float
        self.data = []
        for i in range(numPlots):   # give an array for each type of data and store them in a list
            self.data.append(collections.deque([0] * plotLength, maxlen=plotLength))
        self.isRun = True
        self.isReceiving = False
        self.thread = None
        self.plotTimer = 0
        self.previousTimer = 0
        self.csvData = []

        print('Tratando de conectar con ' + str(serialPort) + ' a ' + str(serialBaud) + ' BAUD.')
        try:
            self.serialConnection = serial.Serial(serialPort, serialBaud, timeout=4)
            print('Conectado con ' + str(serialPort) + ' a ' + str(serialBaud) + ' BAUD.')
        except:
            print("Fallo la conexion con " + str(serialPort) + ' a ' + str(serialBaud) + ' BAUD.')

    def readSerialStart(self):
        if self.thread == None:
            self.thread = Thread(target=self.backgroundThread)
            self.thread.start()
            # Block till we start receiving values
            while self.isReceiving != True:
                time.sleep(0.1)

    def getSerialData(self, frame, lines, lineValueText, lineLabel, timeText,RecData):
        currentTimer = time.clock() #time.perf_counter()
        self.plotTimer = int((currentTimer - self.previousTimer) * 1000)     # the first reading will be erroneous
        self.previousTimer = currentTimer
        timeText.set_text('Intervalo de tiempo en = ' + str(self.plotTimer) + 'ms')
        privateData = copy.deepcopy(self.rawData[:])    # so that the 3 values in our plots will be synchronized to the same sample time
        for i in range(self.numPlots):
            data = privateData[(i*self.dataNumBytes):(self.dataNumBytes + i*self.dataNumBytes)]
            value,  = struct.unpack(self.dataType, data)
            self.data[i].append(value)    # we get the latest data point and append it to our array
            lines[i].set_data(range(self.plotMaxLength), self.data[i])
            lineValueText[i].set_text('[' + lineLabel[i] + '] = ' + str(value))
            if RecData == 1:            
            	self.csvData.append([self.data[0][-1], self.data[1][-1], self.data[2][-1]])

    def backgroundThread(self):    # retrieve data
        time.sleep(1.0)  # give some buffer time for retrieving data
        self.serialConnection.reset_input_buffer()
        while (self.isRun):
            self.serialConnection.readinto(self.rawData)
            self.isReceiving = True
            #print(self.rawData)

    def close(self,RecData):
        self.isRun = False
        self.thread.join()
        self.serialConnection.close()
        print('Desconectando...')
        if RecData == 1:
                folder = os.getcwd()
                folder_datos = folder + "/datos"
                try:
                        os.stat(folder_datos)
                except:
                        os.mkdir(folder_datos)	
                x = datetime.datetime.now()
                fecha_hora =  x.isoformat()
                df = pd.DataFrame(self.csvData)
                df.to_csv(folder_datos + '/datos-' + fecha_hora + ".csv")

def main():
    clear()
    print("Monitor de puerto serial")
    print("v1.00, 13 de noviembre del 2022")
    print("Introduzca los siguientes parametros" )
    i = 0
    while i < 3:
    	RecData = int(input("Desea grabar los datos (1 - Si / 0 - No) -> "))
    	print(RecData)
    	if RecData == 1:
    		print("El archivo CSV se almacena en el folder actual con el nombre datos-fecha-hora.csv")
    		break;
    	elif RecData == 0:   
    		print("Los datos no se almacenaran")    
    		break;
    	else:
        	i += 1
        	if i < 3:
        		print('Introduzca 1 / 0 solamente, va de nuevo')
        	else:    	
        		sys.exit("Error, introduzca Y/N solamente")

    portName = input("Nombre del puerto a monitorear, eg 'COM5' o '/dev/ttyACM0' -> ")
    baudRate = int(input("Baudrate de puerto? -> "))
    maxPlotLength = int(input("Muestras en el plot? -> "))     # number of points in x-axis of real time plot
    dataNumBytes = 4        # number of bytes of 1 data point
    numPlots = 3            # number of plots in 1 graph
    
    # plotting starts below
    pltInterval = int(input("Tiempo de actualizacion de plot en mseg -> "))    # Period at which the plot animation updates [ms]
    xmin = 0
    xmax = maxPlotLength
    ymin = float(input("Minimo valor de plot en magnitud -> "))
    ymax = float(input("Maximo valor de plot en magnitud -> "))

    s = serialPlot(portName, baudRate, maxPlotLength, dataNumBytes, numPlots)   # initializes all required variables
    s.readSerialStart()                                               # starts background thread
    
    print("Conectado, para terminar cierre la figura...")

    fig = plt.figure(figsize=(10, 10))
    ax = plt.axes(xlim=(xmin, xmax), ylim=(float(ymin - (ymax - ymin) / 10), float(ymax + (ymax - ymin) / 10)))
    ax.set_title('Monitor')
    ax.set_xlabel("Tiempo")
    ax.set_ylabel("Magnitud")
    lineLabel = ['Referencia', 'Salida', 'Control']
    style = ['r-', 'c-', 'b-']  # linestyles for the different plots
    timeText = ax.text(0.70, 0.95, '', transform=ax.transAxes)
    lines = []
    lineValueText = []
    for i in range(numPlots):
        lines.append(ax.plot([], [], style[i], label=lineLabel[i])[0])
        lineValueText.append(ax.text(0.70, 0.90-i*0.05, '', transform=ax.transAxes))
    anim = animation.FuncAnimation(fig, s.getSerialData, fargs=(lines, lineValueText, lineLabel, timeText,RecData), interval=pltInterval)    # fargs has to be a tuple

    plt.legend(loc="upper left")
    plt.show()
    
    s.close(RecData)


if __name__ == '__main__':
    main()
