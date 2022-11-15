// ******************************************************** //
//                                                          //
// Control Tracking por asignacion de polos                 //
//                                                          //
// r(t) = sen(w t)+sen(3w t)/3                              //
//                                                          //
// Noviembre 8 del 2022, MAPG, EAG                          //
//                                                          //
// Instrucciones                                            //
//                                                          //
// Modificar solamente                                      //
//    Polos de control                                      //
//    Polos de observador                                   //
//    Valores de R1, C1, R2, C2 en planta                   // 
//    Valor de la frecuencia fundamental de la referencia   //
//                                                          //
// ******************************************************** //

// ********************************************************//
// ------- Frecuencia de la referenca------------
// *********************************************************//
  #define omega1 0.2
  #define omega2 0.3
// ******************************************************** //

// ******************************************************** //
//---------- Polos cont --------//                          //
// ******************************************************** //
  #define a1r -1                                            //
  #define a1i -1.7320508                                    //
  #define a2r -1                                          //
  #define a2i 1.7320508                                    //
// ******************************************************** //

// ******************************************************** //
//---------- Polos obs --------//                           //
// ******************************************************** //
  #define b1r -10                                           //
  #define b1i   0                                           //
  #define b2r -10                                           //
  #define b2i   0                                           //
// ******************************************************** //

// ******************************************************** //
//----------  Planta  --------//                            //
// ******************************************************** //
  #define R1 1000000                                        //
  #define R2 1000000                                        //
  #define C1 0.000001                                       //
  #define C2 0.000001                                       //
// ******************************************************** //



// ******************************************************** //
//----------  Librerias  --------//                      //
// ******************************************************** //

  #include <SoftwareSerial.h>



// ******************************************************** //
//----------  Definiciones  --------//                      //
// ******************************************************** //

// ******************************************************** //
//----------  Constantes  --------//                        //
// ******************************************************** //

//---------- Escalamientos --------//
  #define mX 0.004882813
  #define bX 0
  #define mU 51
  #define bU 0

//---------- Definición Pines IO discretos--------//                  
  #define pLED8 8             // LED 8 en tarjeta
  #define pLED9 9             // LED 9 en tarjeta   
  #define pSW2 2              // SW 2 en tarjeta                                    
  #define pSW3 3              // SW 3 en tarjeta      


//---------- Definicion Pines analogicos --------//
  #define pR 0                  
  #define pX1 1 
  #define pX2 2                                    
  #define pU 10                   


// ******************************************************** //
//----------  Variables globales  --------//                //
// ******************************************************** //


//---------- Ganancias de Controlador --------//
  float Kp = 0;
  float K1 = 0;
  float K2 = 0;
  float G1 = 0;
  float G2 = 0;
  float G3 = 0;
  float G4 = 0;

//---------- Ganancias de Observador --------//
  float H1 = 0;
  float H2 = 0;

//---------- Definiciones --------//
  unsigned long TIC = 0;
  unsigned long TS = 20;  // Muestreo 50 miliseg
  float Tseg = 0.02;      // Muestreo en segundos
  unsigned long TS_code = 0; 
  unsigned long TC = 0;
  
   
//----------  Señales --------//
  float R = 0;
  float Rw = 0;
  float Y = 0;
  float X1 = 0;
  float X2 = 0;
  float U = 0;
  int Ui = 0;

//----------  Observador --------//
//-- Estados obs --//
  float W1 = 0; // Estado exosistema 1 en k, W1[k]
  float W2 = 1/3; // Estado exosistema 2 en k, W2[k]
  float W3 = 0; // Estado exosistema 1 en k, W1[k]
  float W4 = -1; // Estado exosistema 2 en k, W2[k]
  float WN1 = 0; // Estado exosistema 1 en k+1, W1[k+1]
  float WN2 = 0; // Estado exosistema 2 en k+1, W2[k+1]
  float WN3 = 0; // Estado exosistema 3 en k+1, W3[k+1]
  float WN4 = 0; // Estado exosistema 4 en k+1, W4[k+1]
  float XeR1 = 0; // Estado estimado 1 en k, Xe1[k]
  float XeR2 = 0; // Estado estimado 2 en k, Xe2[k]
  float XeN1 = 0; // Estado estimado 1 en k+1, Xe1[k+1]
  float XeN2 = 0; // Estado estimado 2 en k+1, Xe2[k+1]
  float f1 = 0; // Dinamica de observador
  float f2 = 0; // Dinamica de observador

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
  // u = Kp*(Vref) - K1*(Vcap1) - K2*(Vcap2) + G1*W1 + G2*W2

  K1 = p11*(sum_a - 1/p11 - 1/p21 - 1/p22);
  K2 = p12*p22*(prod_a - 1/(p11*p22) - 1/(p21*p21) - K1/(p21*p22) + 1/(p21*p22));
  Kp = K1 + K2 + 1;
  G1 = 1 - omega1*omega1*p11*p22+K1 + K2;
  G2 = omega2*(p22 + p12) - K1*omega2*p22;
  G3 = p11*(1/p11 + 1/p21 - omega2*omega2*p22) + K1 + K2;
  G4 = omega1*(1 + p22/p11 + p22/p21) + omega1*K1*p21;
  
  // Ganancias de Observador
  // Xe[k+1] = A*Xe[k] + B*U[k] + H*(Y[k]-Xe2[k])

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
  coms_arduino_ide();               // Comunicaciones
  //coms_python(&Rw,&Y,&U);
  espera();
}




// ******************************************************** //
//---------- Rutinias --------//                            //
// ******************************************************** //

// ******************************************************** //
//---------- Rutinias de control y observador--------       //                          
// ******************************************************** //

//-- Observador e integradores --//
void observador(){
  f1 = Am11*XeR1 + Am12*XeR2 + Bm1*U + H1*(Y-XeR2);     // Dinamica estado 1 observador
  f2 = Am21*XeR1 + Am22*XeR2 + Bm2*U + H2*(Y-XeR2);     // Dinamica estado 2 observador

  XeN1 = XeR1 + Tseg*f1;              // Integrador estado 1 mediante Euler
  XeN2 = XeR2 + Tseg*f2;              // Integrador estado 2 mediante Euler
  WN1 = W1 + Tseg*omega1*W4;
  WN2 = W2 + Tseg*omega2*W3;
  WN3 = W3 - Tseg*omega2*W2;
  WN4 = W4 - Tseg*omega1*W1;
  XeR1 = XeN1;
  XeR2 = XeN2;
  W1 = WN1;
  W2 = WN2;
  W3 = WN3;
  W4 = WN4;
}

//-- Control --//
void control(){
  // Ley de control
  U = Habilitado*(Kp*R + G1*W1 + G2*W2 + G3*W3 + G4*W4 - K1*XeR1 - K2*XeR2);       // Ley de control retro estado estimado

  // Saturacion
  if(U >= 5.0) U = 5.0;               // Saturacion de control en rango 0 a 5V                      
  else if(U < 0) U = 0;
}


// ******************************************************** //
//---------- Rutinias de IO y control de tiempo     --------//                          
// ******************************************************** //


//-- Procesado de entradas --//
void proc_entradas(){
  // No se ocupa leer X1 (Vc1) porque se estima con observador
  X1=analogRead(pX1)*mX+bX;           // Lectura de salida de planta en pin pX3
  X2=analogRead(pX2)*mX+bX;           // Lectura de salida de planta en pin pX2
  R=analogRead(pR)*mX+bX;             // Lectura de referencia en pin pR
  Y = X2;
  Rw = Habilitado*(R + W1 + W3);
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
  Serial.print(Rw);                    // Referencia
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
