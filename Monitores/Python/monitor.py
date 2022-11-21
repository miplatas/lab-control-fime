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

# Para limpiar pantalla
def clear(): 
    if os.name == "posix":
        os.system ("clear")
    elif os.name == ("ce", "nt", "dos"):
        os.system ("cls")

# Para guardar los datos de configuracion de puerto serial en config.ini
def loadconfig():
    folder = os.getcwd()
    ruta_config = folder + "/config/config.ini"
    isFile = os.path.isfile(ruta_config)
    if isFile == True:
        print('Leyendo la configuracion del puerto serial del archivo:' + ruta_config)
        df = pd.read_csv(ruta_config, sep=':', header =None)
        data = (df.get(1))
        Port = (data.get(0)).strip()
        Baudrate = int(data.get(1))
        Samples = int(data.get(2))
        Sampling = int(data.get(3))
        Min_amp = int(data.get(4))
        Max_amp = int(data.get(5))
    else: 
        print('La configuracion del puerto no existe.')
        print('Introducir informacion de conexion de manera manual.')
        if sys.platform == "linux" or sys.platform == "linux2":
            # linux
            Port = input("Nombre del puerto a monitorear, incluir ' ', por ejemplo '/dev/ttyACM0' -> ")
        elif sys.platform == "darwin":
            # OS X
            Port = input("Nombre del puerto a monitorear, incluir ' ', por ejemplo '/dev/tty.usb' -> ")
        elif sys.platform == "win32":
            # Windows...
            Port = input("Nombre del puerto a monitorear, por ejemplo COM5 -> ")
        else:
            raise Exception("El sistema operativo no se ha podido determinar.")
        Baudrate = int(input("Baudrate de puerto? -> "))
        Samples = int(input("Muestras en el plot? -> "))     # number of points in x-axis of real time plot
        Sampling = int(input("Tiempo de actualizacion de plot en mseg -> "))    # Period at which the plot animation updates [ms]
        Min_amp = int(input("Minimo valor de plot en magnitud -> "))
        Max_amp = int(input("Maximo valor de plot en magnitud -> "))
        i = 0
        while i < 3:
            RecData = int(input("Desea guardar la configuracion para siguientes conexiones (1 - Si / 0 - No) -> "))
            if RecData == 1:
                print("El archivo config.ini contiene los datos de conexion, y se almacena en el folder /config/")
                time.sleep(3)
                try:
                    os.stat(os.getcwd() + '/config')
                except:
                    os.mkdir(os.getcwd() + '/config')	
                    print('Creando directorio /config...')
                    time.sleep(3)
                data_new = {'0': ['Port', 'Baudrate', 'Samples', 'Sampling_ms', 'Min_Value_Plot', 'Max_Value_Plot'], '1': [Port, Baudrate, Samples, Sampling, Min_amp, Max_amp]}
                df = pd.DataFrame(data=data_new)
                print('Guardando datos de conexion en config.ini...')
                df.to_csv(ruta_config, sep = ':', index = False, header = False)
                time.sleep(3)    
                print('Los datos de configuracion han sido almacenados en config/config.ini para posteriores sesiones.')
                time.sleep(3)  
                break
            elif RecData == 0:   
                print("Los datos de conexion no se almacenaran en archivo config.ini.")
                time.sleep(3)    
                break
            else:
                i += 1
                if i < 3:
                    print('Introduzca 1 / 0 solamente, va de nuevo.')
                else:    	
                    sys.exit("Error, introduzca 1/0 solamente.")
    time.sleep(3)
    return Port, Baudrate, Samples, Sampling, Min_amp, Max_amp

# Comunicacion por puerto serial
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

    def close(self,RecData):
        self.isRun = False
        self.thread.join()
        self.serialConnection.close()
        print('Desconectando...')
        time.sleep(3)
        if RecData == 1:
                print('Almacenando datos de monitoreo en archivo csv...')
                time.sleep(3)
                folder = os.getcwd()
                folder_datos = folder + "/datos"
                try:
                        os.stat(folder_datos)
                except:
                        os.mkdir(folder_datos)	
                        print('Creando directorio /datos...:' + folder_datos)
                        time.sleep(3)
                x = datetime.datetime.now()
                fecha_hora =  x.isoformat()
                fecha_hora = fecha_hora.replace(':', '', 2)
                fecha_hora = fecha_hora.replace('.', '', 2)
                fecha_hora = fecha_hora.replace('-', '', 2)
                fecha_hora = fecha_hora[2:15]
                df = pd.DataFrame(self.csvData, columns=['Tiempo en seg', 'Referencia', 'Salida', 'Control'])
                df.to_csv(folder_datos + '/datos-' + fecha_hora + ".csv")
                print('Los datos de monitoreo han sido almacenados en el archivo' + folder_datos + '/datos-' + fecha_hora + ".csv")
                time.sleep(3)                   
        print('Cerrando sesion...')
        time.sleep(3)   
                
    def backgroundThread(self):    # retrieve data
        time.sleep(1.0)  # give some buffer time for retrieving data
        self.serialConnection.reset_input_buffer()
        while (self.isRun):
            self.serialConnection.readinto(self.rawData)
            self.isReceiving = True
            #print(self.rawData)

    def getSerialData(self, frame, lines, lineValueText, lineLabel, timeText,RecData):
        if sys.version[0] == '2':
            currentTimer =  time.clock()
        else:
            currentTimer = time.perf_counter()
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
            self.csvData.append([currentTimer, self.data[0][-1], self.data[1][-1], self.data[2][-1]])



# Programa principal
def main():
    clear()
    print("--------------------------------------------------------------")
    print("|                                                            |")
    print("| Monitor de puerto serial.                                  |")
    print("| v1.01, 13 de noviembre del 2022.                           |")
    print("| Departamento de Electronica y Automatizacion, FIME-UANL.   |")
    print("|                                                            |")
    print("--------------------------------------------------------------")
    print("Introduzca los siguientes parametros:" )
    i = 0
    while i < 3:
        RecData = int(input("Desea grabar los datos (1 - Si / 0 - No) -> "))
        if RecData == 1:
            print("El monitoreo de datos sera grabado.")
            break
        elif RecData == 0:   
            print("El monitoreo de datos no sera grabado.")    
            break
        else:
            i += 1
            if i < 3:
                print('Introduzca 1 / 0 solamente, va de nuevo.')
            else:    	
                sys.exit("Error, introduzca 1/0 solamente.")

    # Configuracion de puerto
    [portName, baudRate, maxPlotLength, pltInterval, ymin, ymax] = loadconfig()

    # Monitoreo
    dataNumBytes = 4        # number of bytes of 1 data point
    numPlots = 3            # number of plots in 1 graph
    xmin = 0
    xmax = maxPlotLength

    s = serialPlot(portName, baudRate, maxPlotLength, dataNumBytes, numPlots)   # initializes all required variables
    s.readSerialStart()                                               # starts background thread
    
    print("Conectado, para terminar cierre la figura...")

    fig = plt.figure(figsize=(10, 10))
    ax = plt.axes(xlim=(xmin, xmax), ylim=(float(ymin - (ymax - ymin) / 10), float(ymax + (ymax - ymin) / 10)))
    ax.set_title('Monitor de puerto serie')
    ax.set_xlabel("Tiempo en muestras a " + str(pltInterval) + " mseg.")
    ax.set_ylabel("Magnitud")
    lineLabel = ['Referencia', 'Salida', 'Control']
    style = ['r', 'k-', 'b:']  # linestyles for the different plots
    timeText = ax.text(0.63, 0.95, '', transform=ax.transAxes)
    lines = []
    lineValueText = []
    for i in range(numPlots):
        lines.append(ax.plot([], [], style[i], label=lineLabel[i])[0])
        lineValueText.append(ax.text(0.63, 0.90-i*0.05, '', transform=ax.transAxes))
    anim = animation.FuncAnimation(fig, s.getSerialData, fargs=(lines, lineValueText, lineLabel, timeText,RecData), interval=pltInterval)    # fargs has to be a tuple

    plt.legend(loc="upper left")
    plt.grid()
    plt.show()
    
    s.close(RecData)


if __name__ == '__main__':
    main()
