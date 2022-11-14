# Laboratorio de control moderno

## Tabla de contenidos
* [Información general](#Información)
* [Descripción del Hardware usado](#Hardware)
* [Descripción de los controladores usados](#Software)

## Información
Este proyecto muestra como implementar controladores y observadores lineales mediante la plataforma Arduino.

## Hardware
El algoritmo de control/observación se realiza en la plataforma Arduino UNO R3 y una planta RC-RC, con R1 = R2 = 1 ohm, y C1 = C2 = 1 uf. Una descripción del hardware usado es mostrada en la Figura. 

![Circuito eléctrico](./imagenes/circuito.png)

### La lista de partes es la siguiente:

* Una placa de desarrollo Arduino UNO R3
* Opcional (Un shield LCFIME-PCB 1.0)
* 2 resistencias de 1 Mohm. 
* 2 capacitores de 1 uf.
* 1 Potenciometro 100 K
* 2 Resistencias 330 ohms
* 2 Resistencias 4.7 kohms
* 2 Leds
* 2 Switches

Todos los componentes pueden ser montados en un breadboard de acuerdo a la figura presnetada, o en el shield.

### Las entradas y salidas del sistema son las siguientes  

* Pin A0 Referencia a seguir
* Pin A1 Voltaje en el capacitor 1
* Pin A2 Voltaje en el capacitor 2
* Pin 10 Señal de control
* Pin 2 Entrada de arranque
* Pin 3 Entrada de paro
* Pin 8 Indicador LED sistema activado
* Pin 9 Indicador LED sistema desactivado
	
## Modelo

El modelo dinámico del circuito RC-RC es un sistema de segundo orden que puede ser representado en espacio de estados de la manera siguiente.

Primeramente, se usa la ley de mallas de Kirchhoff:
```math
\begin{eqnarray*}
  u_i(t) & = & i_1(t)R_1+V_{C1}(t),  \\
  V_{C1}(t) & = & i_2(t)R_2 + V_0(t), 
\end{eqnarray*}
```
junto con las relaciones a ser cumplidas por los capacitores:
```math
\begin{eqnarray*}
  i_1(t)-i_2(t) & = & C_1 \frac{dV_{C1}(t)}{dt}, \\
  i_2(t) & = & C_2\frac{dV_{C2}(t)}{dt},
\end{eqnarray*}
```
por lo tanto $i_1(t)=C_2\frac{dV_0(t)}{dt}+C_1\frac{dV_{C1}(t)}{dt}$

Re escribiendo las ecuaciones de malla en funcion de los voltajes en los capacitores y sus derivadas:
```math
\begin{eqnarray*}
u_i(t) & = & R_1C_2 \frac{dV_0(t)}{dt} +  R_1C_1 \frac{dV_{C1}(t)}{dt} + V_{C1}, \\
V_{C1}(t) & = & R_2C_2 \frac{dV_0(t)}{dt} + V_0(t),
\end{eqnarray*}
```
y acomodando en forma matricial, se tiene
```math
\begin{eqnarray*}
\left[\begin{array}{cc}R_1C_1 & R_1C_2 \\ 0 & R_2C_2\end{array}\right]\left[\begin{array}{l}\dot{V}_{C_1} \\ \dot{V}_o \end{array}\right] & = & \left[\begin{array}{rr}-1 & 0 \\ 1 & -1  \end{array}\right]\left[\begin{array}{l}V_{C_1}\\V_o\end{array}\right]+\left[\begin{array}{c}1 \\ 0 \end{array}\right]V_i.
\end{eqnarray*}
```

Considerando la inversa:
```math
\begin{eqnarray*}
\left[\begin{array}{cc}R_1C_1 & R_1C_2 \\ 0 & R_2C_2\end{array}\right]^{-1} & = & \frac{1}{R_1C_1R_2C_2}\left[\begin{array}{cr}R_2C_2 & -R_1C_2 \\ 0 & R_1C_1\end{array}\right]\\ & = & \left[\begin{array}{cr}\frac{1}{R_1C_1} & \frac{-1}{R_2C_1} \\ 0 & \frac{1}{R_2C_2}\end{array}\right],
\end{eqnarray*}
```
el sistema queda:
```math
\begin{eqnarray*}
  \left[\!\! \begin{array}{l}\dot{V}_{C_1} \\ \dot{V}_o \end{array}\!\! \right]\!\! & \!\! =\!\!  &\!\!  \left[\!\! \begin{array}{cr}\frac{1}{R_1C_1} & \frac{-1}{R_2C_1} \\ 0 & \frac{1}{R_2C_2}\end{array}\right]\!\! \left[\!\! \begin{array}{rr}-1 & 0 \\ 1 & -1  \end{array}\right]\!\! \left[\!\! \begin{array}{l}V_{C_1}\\V_o\end{array}\right]\!\! +\!\! \left[\!\! \begin{array}{cr}\frac{1}{R_1C_1} & \frac{-1}{R_2C_1} \\ 0 & \frac{1}{R_2C_2}\end{array}\right]\!\! \left[\!\! \begin{array}{c}1 \\ 0 \end{array}\!\! \right]u_i(t).
\end{eqnarray*}
```

Definiendo las variables de estado como sigue;
```math
\begin{eqnarray*}
  x_1(t) & \stackrel{\triangle}{=} & V_{C1}(t), \\
  x_2(t) & \stackrel{\triangle}{=} & V_{C2}(t)=V_0(t),
\end{eqnarray*}
```
el modelo en espacio de estados es: 
```math
\begin{eqnarray*}
  \left[\!\!\begin{array}{l}\dot{x}_{1}(t) \\ \dot{x}_2 (t) \end{array}\!\!\right] & = &  \left[\!\!\begin{array}{cr}-\frac{1}{R_1C_1}-\frac{1}{R_2C_1} & \frac{1}{R_2C_1} \\ \frac{1}{R_2C_2} & -\frac{1}{R_2C_2}\end{array}\!\!\right]\!\!\left[\begin{array}{l}x_{1}(t) \\ x_2(t) \end{array}\!\!\right]\!\!+\!\!\left[\!\!\begin{array}{c}\frac{1}{R_1C_1} \\ 0 \end{array}\right]u_i(t) \\ 
  y(t) & = & \left[\begin{array}{cc}0 & 1\end{array}\right]\left[\begin{array}{c}x_{1}(t) \\ x_2(t) \end{array}\right],
\end{eqnarray*}
```
donde $x_{1}(t)$ es el voltaje en el primer capacitor (conectado al pin A1), $x_{2}(t)$ el voltaje en el segundo (conectado al pin A2), $u(t)$ es la entrada a la planta (conectado al pin 10), y $y(t)$ la salida de la planta (conectado al pin A2). 

## Diseño de controladores y observadores

### Controlador

Se usa un controlador por retroalimentación de estado con ley de control dada por
```math
u(t) = - K_1 x_1(t) - K_2 x_2(t)
```
### Observador

### Controlador integral (Servosistema)

### Seguimiento de trayectorias (Tracking)



## Software

### Software Arduino

Todos los algoritmos se implementan en el microcontrolador ATMEL de la plataforma arduino. Los códigos están realizados en el lenguaje Arduino.
La carpeta Arduino contiene varios ejemplos que se detallas a continuación.



### Software PC
