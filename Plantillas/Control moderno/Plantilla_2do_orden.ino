// Control por retro de estado con arduino.

// ******************************************************** //
//                                                          //
// Control por asignación de polos para circuito RC-RC      //
//                                                          //
// Noviembre 13 del 2022, MAPG, EAG                         //
//                                                          //
// Instrucciones                                            //
//                                                          //
// Modificar solamente el renglón 136                       //
//                                                          //
//      U = (4)*R - (-1)*X1 - (4)*X2;                       //
//                                                          //
// ******************************************************** //



// ******************************************************** //
//----------  Muestreo  --------//                          //
// ******************************************************** //
  unsigned long TS = 50;      // Muestreo TS miliseg        //
  float Tseg = 0.05;          // Muestreo en Tseg segundos  //
// ******************************************************** //


// ******************************************************** //
//----------  Definiciones  --------//                      //
// ******************************************************** //


// ******************************************************** //
//----------  Constantes  --------//                        //
// ******************************************************** //


//---------- Definición Pines IO analogicos--------//                  
  #define pR 0                // pin de referencia             
  #define pX1 1               // pin de estado 1   
  #define pX2 2               // pin de estado 2                     
  #define pU 10               // pin de salida de control (entrada a planta)      

//---------- Definición Pines IO discretos--------//
  #define pLED7 7             // LED 9 en tarjeta                     
  #define pLED8 8             // LED 8 en tarjeta
  #define pLED9 9             // LED 9 en tarjeta   
  #define pSW2 2              // SW 2 en tarjeta                                    
  #define pSW3 3              // SW 3 en tarjeta      
  #define pSW4 4              // SW 2 en tarjeta                                    
  #define pSW5 5              // SW 3 en tarjeta
  #define pSW6 6              // SW 2 en tarjeta                                    
 

//---------- Escalamientos para analogicas de 0 a 5 V --------//
  #define mX 0.004882813      // Pendiente 0-1023 -> 0 - 5
  #define bX 0                // Ajuste cero para 0-1023 -> 0 - 5
  #define mU 51               // Pendiente 0 - 5 -> 0 - 1023
  #define bU 0                // Ajuste cero 0 - 5 -> 0 - 1023



// ******************************************************** //
//----------  Variables globales  --------//                //
// ******************************************************** //

//---------- Tiempo --------//
  unsigned long TS_code = 0;  // Tiempo que tarda programa
  unsigned long TIC = 0;      // Estampa de tiempo inicio ciclos
  unsigned long TC = 0;       // Faltante para TS
   
//----------  Señales --------//
  float R = 0;                // Referencia
  float Y = 0;                // Salida
  float X1 = 0;               // Estado 1
  float X2 = 0;               // Estado 2
  float U = 0;                // Salida control
  int Ui = 0;                 // Salida control tarjeta 


//---------- Otros --------//
  bool Habilitado = 0;        // Señal {0,1} para entradas escalón


// ******************************************************** //
//----------  Librerias  --------//                      //
// ******************************************************** //

  
  #include <SoftwareSerial.h>



// ******************************************************** //
//---------- Rutinia de inicio --------//                   //
// ******************************************************** //


void setup() {
  //--Inicia serial--//
  Serial.begin(9600);

  //--Configura pines digitales--//  
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(4, INPUT);
  pinMode(5, INPUT);
  pinMode(6, INPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
}


// ******************************************************** //
//---------- Rutinia principal  --------//                  //
// ******************************************************** //


void loop() {                     
  proc_entradas();                    // Procesamiento de Entradas
  control();                          // Control
  proc_salidas();                     // Procesado de Salidas
  coms_arduino_ide();               // Comunicaciones
  espera();
}


// ******************************************************** //
//---------- Rutinias de control             --------       //                          
// ******************************************************** //


//-- Control --//
void control(){
  // Ley de control
    U = (4)*R - (-1)*X1 - (4)*X2;       // Ley de control retro estado estimado
   
  // Saturacion
  if(U >= 5.0) U = 5.0;               // Saturacion de control en rango 0 a 5V                      
  else if(U < 0) U = 0;
}


// ******************************************************** //
//---------- Rutinias de IO y control de tiempo     --------//                          
// ******************************************************** //


//-- Procesado de entradas --//
void proc_entradas(){
  X1 = analogRead(pX1)*mX+bX;            // Lectura de salida de planta en pin pX3
  X2 = analogRead(pX2)*mX+bX;               // Lectura de salida de planta en pin pX2
  R = Habilitado*(analogRead(pR)*mX+bX);    // Lectura de referencia en pin pR, Habilitado = {0,1} es para escalones
  Y = X2;
}


//-- Procesado de salidas --//
void proc_salidas(){
  Ui = int(U*mU+bU);                    // Escalamiento
  analogWrite(pU, Ui);                  // Salida PWM en pin pU
  botonesyleds();                       // Manejo de IO discretas
}


//-- Memoria {0,1} para entrada escalón --//
void botonesyleds(){

  if(digitalRead(pSW2) == 1) Habilitado = 1;      // Memoria on/off en Habilitado
  else if(digitalRead(pSW3) == 1) Habilitado = 0; // Set con SW2. Reset con SW3

  if(Habilitado == 0) digitalWrite(pLED8,HIGH);            // LED9 = 1
  else digitalWrite(pLED8, LOW);                           // Cuando Habilitado = 0
 
}

//-- Para muestreo uniforme --//
void espera(){   
  TS_code = millis()- TIC;                 // Tiempo de ciclo
  TC = TS - TS_code;                       // Calcula altante para TS
  if (TS_code < TS) delay(TC);             // Espera para completar ciclo de TS   
  TIC = millis();
}


//-- Comunicación con monitor --//
void coms_arduino_ide(){  
  Serial.print("y_d(t):");            // Referencia
  Serial.print(R);                    // Referencia
  Serial.print(",");                  // Separador     
  Serial.print("y(t):");              // Salida
  Serial.println(Y);                  // Salida (terminar con "serial.println")
}
