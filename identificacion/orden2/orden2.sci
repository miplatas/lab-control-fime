// Identificacion de sistema de 2do 
clc
clear
// ---------------------------------------------
// Sistema a identificar (sobre-amortiguado)
// ---------------------------------------------

Tol = 0.0002;  // Tolerancia de error medio cuadratico
itmax = 20;    // Máximas iteraciones para buscar

// Directorio donde se encuentran los datos, cambiar con tu directorio 
path = '/home/miguel/Dropbox/Trabajo/2022/RC-Arduino/Identificacion/';
Datos = evstr(read_csv(path + 'datos-2022-11-15T11:44:22.247327.csv'));

u=Datos(281:1200,4).';
y=Datos(281:1200,3).';

N=length(y);
Ts=0.05;
t0=0;
t=t0:Ts:(N-1)*Ts;
x0=[0;0];
u_amp=mean(u);

Datos= [u; y]'  // <<---- Practica modificar aqui para leer los datos del archivo xls con las plantas. 

it=1;
J=Tol*2;
lambda0= 1;
lambda1=2;

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

    // Filtrado discreto de salida
    Z=zeros(2,N);
    for k = 1:(N-1)
        Z(:,k+1)=Fd*Z(:,k)+Bd*y(k)
    end

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
    
    eigAi = spec(Ai)
    lambda1 = abs(eigAi(1) + eigAi(2));
    lambda0 = abs(eigAi(1)*eigAi(2));

    // Simulación numérica de sistema identificado
    function xdot=linearident(t, x)
        xdot=Ai*x + Bi*u_amp
    endfunction
    // Rutina de integración
    yi=[0 1]*ode(x0,t0,t,linearident);

    // Función de costo
    J=((y-yi)*(y-yi)')/N;
    lambda0= 0.153687;
    lambda1=1.3709633;
    // Iteraciones.
    it = it + 1;
    
    if it > itmax
        error("Error: se ha alcanzado número máximo de iteraciones")
    end

    if J < Tol
        printf("La función de transferencia propuesta para el sistema es")
        disp(Hi)
            // Muestra resultados 
            scf(1); // Fig 1
            plot(t',y',t',yi','Linewidth',2)
            xgrid
            title('Salidas','fontsize',3)
            legend('$\Large{y(t)}$','$\Large{\hat{y}(t)}$',4)
            xlabel('tiempo','fontsize',3)
        break;
    end

end
