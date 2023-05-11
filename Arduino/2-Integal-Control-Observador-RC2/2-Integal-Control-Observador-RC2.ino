// Integral-Control-Observador-RC-RC

// ******************************************************** //
//                                                          //
// Control integral por asignacion de polos                 //
//                                                          //
// Octubre 28 del 2022, MAPG, EAG                           //
//                                                          //
// Instrucciones                                            //
//                                                          //
// Modificar solamente                                      //
//    Polos de control                                      //
//    Polos de observador                                   //
//    Valores de R1, C1, R2, C2 en planta                   // 
//                                                          //
// Servoproblema mediante control integral                  //
// ******************************************************** //


// ******************************************************** //
//---------- Polos cont --------//                          //
// ******************************************************** //
  #define a1r -1.25                                         //
  #define a1i -1.7320508                                   //
  #define a2r -1.25                                         //
  #define a2i 1.7320508                                    //
  #define a3r -1                                            //
  #define a3i 0                                            //
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
//----------  Muestreo  --------//                          //
// ******************************************************** //
  unsigned long TS = 50;      // Muestreo TS miliseg        //
  float Tseg = 0.05;          // Muestreo en Tseg segundos  //
// ******************************************************** //

                             // Fin de parámetros modificables



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

//---------- Definicion Pines analogicos --------//
  #define pR 0                  
  #define pX1 1 
  #define pX2 2                                    
  #define pU 10   

//---------- Definición Pines IO discretos--------//                  
  #define pLED8 8             // LED 8 en tarjeta
  #define pLED9 9             // LED 9 en tarjeta   
  #define pSW2 2              // SW 2 en tarjeta                                    
  #define pSW3 3              // SW 3 en tarjeta  


// ******************************************************** //
//----------  Variables globales  --------//                //
// ******************************************************** //


//---------- Ganancias de Controlador --------//
  float K0 = 0;
  float K1 = 0;
  float K2 = 0;

//---------- Ganancias de Observador --------//
  float H1 = 0;
  float H2 = 0;

//---------- Tiempo --------//
  unsigned long TIC = 0;
  unsigned long TS_code = 0; 
  unsigned long TC = 0;
  
   
//----------  Señales --------//
  float R = 0;
  float Y = 0;
  float X0 = 0;
  float X1 = 0;
  float X2 = 0;
  float U = 0;
  int Ui = 0;

//----------  Observador --------//

//-- Estados obs --//
  float XeR1 = 0; // Estado estimado 1 en k, Xe1[k]
  float XeR2 = 0; // Estado estimado 2 en k, Xe2[k]
  float XeN1 = 0; // Estado estimado 1 en k+1, Xe1[k+1]
  float XeN2 = 0; // Estado estimado 2 en k+1, Xe2[k+1]
  float X0N = 0; // Estado integrador en k+1, X0[k+1]
  float f1 = 0; // Dinamica de observador
  float f2 = 0; // Dinamica de observador
  float f3 = 0; // Dinamica del integrador

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
  float sum_a = -(a1r  + a2r + a3r);
  float prod_a = -( a1r*a2r - a1i*a2i )*a3r;
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
  // u = K0*(X0) - K1*(Vcap1) - K2*(Vcap2)

  K0 = p11*p22*prod_a;
  K1 = p11*(sum_a - 1/(p11) - 1/(p21) - 1/(p22) );
  K2 = p11*(p22*(a1r*a3r + a2r*a3r + a1r*a2r)+1/(p21));
  
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
  //coms_python(&R,&Y,&U);                          // Comunicaciones
  espera();
}


// ******************************************************** //
//---------- Rutinias --------//                            //
// ******************************************************** //


// ******************************************************** //
//---------- Rutinias de control y observador--------       //                          
// ******************************************************** //


//-- Observador para estimar X1 --//
void observador(){
  f1 = Am11*XeR1 + Am12*XeR2 + Bm1*U + H1*(Y-XeR2);     // Dinamica estado 1 observador
  f2 = Am21*XeR1 + Am22*XeR2 + Bm2*U + H2*(Y-XeR2);     // Dinamica estado 2 observador
  f3 = R - Y;
  XeN1 = XeR1 + Tseg*f1;              // Integrador estado 1 mediante Euler
  XeN2 = XeR2 + Tseg*f2;              // Integrador estado 2 mediante Euler
  X0N  = X0 + Tseg*f3;
  XeR1 = XeN1;
  XeR2 = XeN2;
  X0  = Habilitado*X0N;
}

//-- Control --//
void control(){
  // Ley de control
   U = K0*X0 - K1*XeR1 - K2*XeR2;       // Retroalimentación de estado mas control integral con estados de observador 

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
  Serial.print("y(t):");            // Referencia
  Serial.print(Y);                    // Referencia
  Serial.print(",");                  // Separador  
  Serial.print("u(t):");              // Salida
  Serial.println(U);                  // Salida (terminar con "serial.println")
}

//-- Comunicación con script python 2 monitor.py --//
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
