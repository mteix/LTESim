%X é o vetor das previsões a um passo, 
%Z o vetor da filtragem e 
%W é o vetor a mais de um passo (a n passos). 
%Y é o vetor das observações (entrada)

%function [X,Z,W] = Kalman(Y,n)

%Q = [1 0;0 1]; 
%R = 1; 
%H = [1 0]; 
%P0 = 10^6*[1 0; 0 1]; 
%X = 0;
%Z = X; 
%W = X;
%Phi = [1 1; 0 1]; 
%P1Q = P0; 
%q1 = [1; 1];
%I=Q;
%X0(1,1) = Y(1);
%X0(2,1) = 0.00; 

 P1 = Phi*P0*Phi' + Q;
 K = P1*H'/(H*P1*H'+R);
 X1 = Phi*X0;
 IT = Y - H*X1;
 
 if IT/(H*P1*H'+R) >= 3, 
     IT = 3*(H*P1*H'+R); 
 end
 
 if IT/(H*P1*H'+R) <= -3, 
     IT = -3*(H*P1*H'+R); 
 end
 X0 = X1 + K*IT;
 
 P0 = (I - K*H)*P1; %calculo da pseudo-observaçao
 [q1,P1Q] = Covmod_q(IT,R,Phi,H,P0,Q,q1,P1Q);
 for j = 1:2, 
     if q1(j,1) >= 0,  
         Q(j,j) = q1(j,1);
     else 
         Q(j,j) = 0; 
     end, 
 end
 X(1) = H*X1;         %-> previsao um passo a frente
 Z(1) = H*X0;         %o Valor filtrado
 W(1) = H*(Phi^n)*X0; %
