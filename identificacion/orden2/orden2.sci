// Identificacion de sistema de 2do 
clc
clear
// ---------------------------------------------
// Es necesario recolectar datos experimentales de respuesta escalón,
// o usar el archivo proporcionado
// ---------------------------------------------

tol = 0.000000000000001; // El programa termina cuando la mejora en el MSE sea menor a tol
itmax = 20;    // Máximas iteraciones para buscar

// Directorio donde se encuentran los datos, cambiar con tu directorio 
path = '/home/miguel/Dropbox/Trabajo/2022/RC-Arduino/Identificacion/';
Datos = evstr(read_csv(path + 'datos-2022-11-15T20:47:22.004556.csv'));

u=Datos(235:1500,4).';
y=Datos(235:1500,3).';

N=length(y);
Ts=0.05;
t0=0;
t=t0:Ts:(N-1)*Ts;
x0=[0;0];
u_amp=mean(u);

Datos= [u; y]'  // <<---- Par entrada/salida

it=1;
J=0;
lambda0= 100;   
lambda1=20;

while(1)
    // Sistema para filtrar datos de entrada y salida

    pol=[1 lambda1 lambda0];
    F=[0 -lambda0; 1 -lambda1];
    Bn=[0; 1];


    // Se obtiene sistema discreto
    Filtro=syslin('c',F,Bn,[1 0; 0 1])   
    Filtro_d=dscr(Filtro,Ts)
    Fd=Filtro_d(2);
    Bd=Filtro_d(3);

    // Filtrado discreto de entrada
    E=zeros(2,N);
    for k = 1:(N-1)
        E(:,k+1)=Fd*E(:,k)+Bd*u(k);
    end
    E2 = csim(u,t,Filtro);
    // Filtrado discreto de salida
    Z=zeros(2,N);
    for k = 1:(N-1)
        Z(:,k+1)=Fd*Z(:,k)+Bd*y(k)
    end
    Z2 = csim(u,t,Filtro);

    // Solución de inv(sI-F)*[1; 0] = T*inv(sI-F)*[0; 1] 

    T=[-lambda1/lambda0 1;-1/lambda0 0];
    // Se forma regresor 
    for k = 1:N
        phi(k,:)=[0 1]*[T*E(:,k) E(:,k) T*Z(:,k) Z(:,k)];
    end

    // Solución de mínimos cuadrados
    Theta=inv(phi'*phi)*phi'*y';

    // Los parámetros del sistema identificado son:
    b0=Theta(1,1);
    b1=Theta(2,1);
    a0=lambda0-Theta(3,1);
    a1=lambda1-Theta(4,1);

    // Sistema identificado
    Ai=[0 -a0;1 -a1];
    Bi=[b0;b1];
    Ci=[0 1];
    sl=syslin('c',Ai,Bi,Ci)
    Hi=ss2tf(sl);
    
    lambda1 = a1;
    lambda0 = a0;

    // Simulación numérica de sistema identificado
    yi = csim(u,t,sl);

    // Función de costo
    JR = J;
    J=((y-yi)*(y-yi)')/N;
    
    // Salida del loop.
    if it > itmax
        disp("El programa termina porque se ha alcanzado número máximo de iteraciones")
        break
    end

    if (abs(JR-J) < tol)
        disp("El programa termina porque |J(" + string(it-1) + ")-J(" + string(it-2) + ")| < " + string(tol))
        break;
        
    end

    // Resultados.
    printf("*************************************************************************** \n")
    printf("En la iteración " + string(it) + ", el MSE es de " + string(J) + ". \n\n")
    printf("La función de transferencia propuesta para el sistema en la iteración " +    string(it) + "es: \n\n")
    disp(Hi)
    printf("\n\n")
    printf("El MSE entre la salida identificada y la salida medida es de " + string(J) + ".\n")
    printf("*************************************************************************** \n")
    
    // Muestra resultados 
    scf(1), clf; // Fig 1
    plot(t',y',t',yi','Linewidth',2)
    xgrid
    title('Salidas','fontsize',3)
    legend('$\Large{y(t)}$','$\Large{\hat{y}(t)}$',4)
    xlabel('tiempo','fontsize',3)

    // Iteraciones.
    it = it + 1;

end

