# Laboratorio de control moderno

## Tabla de contenidos
* [Información general](#Información)
* [Descripción del Hardware usado](#Electrónica)
* [Descripción de los controladores usados](#Control)

## Información
Este proyecto muestra como implementar controladores y observadores lineales mediante la plataforma Arduino.

## Descripción del Hardware usado
El algoritmo de control/observación se realiza en la plataforma Arduino UNO R3, una descripción del hardware usado es mostrada en la Figrua. Las entradas y salidas del sistema son las siguientes 

![Algorithm schema](./imagenes/circuito.png)

Todos los controladores presentados se aplican en un circuito RC-RC, con R1 = R2 = 1 ohm, y C1 = C2 = 1 uf.

* Pin A0 Referencia a seguir
* Pin A1 Voltaje en el capacitor 1
* Pin A2 Voltaje en el capacitor 2
* Pin 10 Señal de control
* Pin 2 Entrada de arranque
* Pin 3 Entrada de paro
* Pin 8 Indicador LED sistema activado
* Pin 9 Indicador LED sistema desactivado
	
## Descripción de los controladores usados
To run this project, install it locally using npm:
