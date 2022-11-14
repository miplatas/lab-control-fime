// Control-Observador-RC-RC

// ******************************************************** //
//                                                          //
// Control por asignación de polos para circuito RC-RC      //
//                                                          //
// Noviembre 13 del 2022, MAPG, EAG                         //
//                                                          //
// Instrucciones                                            //
//                                                          //
// Modificar solamente:                                     //
//    Polos de control                                      //
//    Polos de observador                                   //
//    Modelo Valores de R1, C1, R2, C2 en planta            // 
//    Muestreo                                              //
//                                                          //
// ******************************************************** //


// ******************************************************** //
//---------- Polos de control --------//                    //
// ******************************************************** //
  #define a1r -1                         // Real polo 1     //
  #define a1i -1.7320508                 // Imag polo 1     //
  #define a2r -1                         // Real polo 2     //
  #define a2i  1.7320508                 // Imag polo 2     //
// ******************************************************** //

  
// ******************************************************** //
//---------- Polos de observador   --------//               //
// ******************************************************** //
  #define b1r -10                        // Real polo 1     //
  #define b1i   0                        // Imag polo 1     //
  #define b2r -10                        // Real polo 2     //
  #define b2i   0                        // Imag polo 2     //
// ******************************************************** //


// ******************************************************** //
//----------  Valores de R1, C1, R2, C2 en planta  -------- //
// ******************************************************** //
  #define R1 1000000                                        //
  #define R2 1000000                                        //
  #define C1 0.000001                                       //
  #define C2 0.000001                                       //
// ******************************************************** //


// ******************************************************** //
//----------  Muestreo  --------//                          //
// ******************************************************** //
  unsigned long TS = 20;      // Muestreo TS miliseg        //
  float Tseg = 0.02;          // Muestreo en Tseg segundos  //
// ******************************************************** //

                             // Fin de parámetros modificables



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
  #define pLED8 8             // LED 8 en tarjeta
  #define pLED9 9             // LED 9 en tarjeta   
  #define pSW2 2              // SW 2 en tarjeta                                    
  #define pSW3 3              // SW 3 en tarjeta      

//---------- Escalamientos para analogicas de 0 a 5 V --------//
  #define mX 0.004882813      // Pendiente 0-1023 -> 0 - 5
  #define bX 0                // Ajuste cero para 0-1023 -> 0 - 5
  #define mU 51               // Pendiente 0 - 5 -> 0 - 1023
  #define bU 0                // Ajuste cero 0 - 5 -> 0 - 1023



// ******************************************************** //
//----------  Variables globales  --------//                //
// ******************************************************** //

  
//---------- Ganancias de Controlador --------//
  float Kp = 0;
  float K1 = 0;
  float K2 = 0;

//---------- Ganancias de Observador --------//
  float H1 = 0;
  float H2 = 0;

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

//----------  Observador --------//

//-- Estados obs --//
  float XeR1 = 0;             // Estado estimado 1 en k, Xe1[k]
  float XeR2 = 0;             // Estado estimado 2 en k, Xe2[k]

//-- Modelo obs --//
  // Matriz A
  float Am11 = 0;
  float Am12 = 0;
  float Am21 = 0;
  float Am22 = 0;
  // Matriz B
  float Bm1 = 0;
  float Bm2 = 0;  

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
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);

  //-- Determina ganancias --// 
  // Factores comunes
  float p11 = R1*C1;
  float p12 = R1*C2;
  float p21 = R2*C1;
  float p22 = R2*C2;
  float sum_a = -(a1r  + a2r);
  float prod_a = a1r*a2r - a1i*a2i;
  float sum_b = -(b1r  + b2r);
  float prod_b = b1r*b2r - b1i*b2i;
  // Planta
  Am11 = -1/p11 - 1/p21;
  Am12 = 1/p21;
  Am21 = 1/p22;
  Am22 = -1/p22;
  Bm1 = 1/p11;
  Bm2 = 0;
  // Ganancias de Controlador
  K1 = p11*(sum_a - 1/p11 - 1/p21 - 1/p22);
  K2 = p12*p22*(prod_a - 1/(p11*p22) - 1/(p21*p21) - K1/(p21*p22) + 1/(p21*p22));
  Kp = K1 + K2 + 1;
  // Ganancias de Observador
  H2 = sum_b - 1/p11 - 1/p22 - 1/p22;
  H1 = p22*(prod_b - ((1/p11 + 1/p21)*(1/p22 + H2))) + 1/p21;  
}


// ******************************************************** //
//---------- Rutinia principal  --------//                  //
// ******************************************************** //


void loop() {                     
  proc_entradas();                    // Procesamiento de Entradas
  observador();                       // Observador
  control();                          // Control
  proc_salidas();                     // Procesado de Salidas
  coms_arduino_ide();                 // Comunicaciones Arduino IDE
  //coms_python(&R,&Y,&U);            // Comunicaciones con script monitor.py
  espera();
}


// ******************************************************** //
//---------- Rutinias de control y observador--------       //                          
// ******************************************************** //


//-- Control --//
void control(){
  // Ley de control
    U = Kp*R - K1*XeR1 - K2*XeR2;       // Ley de control retro estado estimado
    // U = Kp*R - K1*X1 - K2*X2;       // Ley de control retro estado

  // Saturacion
  if(U >= 5.0) U = 5.0;               // Saturacion de control en rango 0 a 5V                      
  else if(U < 0) U = 0;
}


//-- Observador para estimar X1 --//
void observador(){
  // Dinamica observador
  float f1 = Am11*XeR1 + Am12*XeR2 + Bm1*U + H1*(Y-XeR2);     // Dinamica estado 1 observador
  float f2 = Am21*XeR1 + Am22*XeR2 + Bm2*U + H2*(Y-XeR2);     // Dinamica estado 2 observador

  // Integracion Euler
  float XeN1 = XeR1 + Tseg*f1;              // Integrador estado 1 mediante Euler
  float XeN2 = XeR2 + Tseg*f2;              // Integrador estado 2 mediante Euler
  XeR1 = XeN1;
  XeR2 = XeN2;

  // Los estados del observador no ocupan saturación porque están en float
}


// ******************************************************** //
//---------- Rutinias de IO y control de tiempo     --------//                          
// ******************************************************** //


//-- Procesado de entradas --//
void proc_entradas(){
  // No se ocupa leer X1 (Vc1) porque se estima con observador
  // X1 = analogRead(pX1)*mX+bX;            // Lectura de salida de planta en pin pX3
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

  static int n = 0; 
  if(n >= 1000/TS) n=0;                                // Señal cuadrada para led blink
  else n = n+1;

  if(digitalRead(pSW2) == 1) Habilitado = 1;      // Memoria on/off en Habilitado
  else if(digitalRead(pSW3) == 1) Habilitado = 0; // Set con SW2. Reset con SW3

  if(n >= 500/TS && Habilitado == 1) digitalWrite(pLED8,HIGH); // Led blink en LED8
  else digitalWrite(pLED8, LOW);                           // Cuando Habilitado = 1

  if(Habilitado == 0) digitalWrite(pLED9,HIGH);            // LED9 = 1
  else digitalWrite(pLED9, LOW);                           // Cuando Habilitado = 0
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


void coms_python(float* Rp, float* Yp, float* Up)
{
  byte* byteData1 = (byte*)(Rp);
  byte* byteData2 = (byte*)(Yp);
  byte* byteData3 = (byte*)(Up);
  byte buf[12] = {byteData1[0], byteData1[1], byteData1[2], byteData1[3],
                 byteData2[0], byteData2[1], byteData2[2], byteData2[3],
                 byteData3[0], byteData3[1], byteData3[2], byteData3[3]};
  Serial.write(buf, 12);
}
