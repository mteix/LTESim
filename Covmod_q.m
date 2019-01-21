 function [q1,P1Q] = Covmod_q(IT,R,Phi,H,P0,Q,q1,P1Q)
% file name: Covmod_q.m

eta2 = 4*IT^2*R + 2*R^2; 
sigmaQ2 = 10^(-2*16)*[1 0; 0 1]; 
zk = IT^2 + R - H*Phi*P0*Phi'*H';
M = [1 0]; 
PhiQ = [1 0; 0 1]; 

%calculo da pseudo-observaçao
KQ = P1Q*M'/(M*P1Q*M'+eta2);
q0 = q1+KQ*(zk-M*q1);
P0Q = P1Q - KQ*M*P1Q;
q1 = PhiQ*q0;
P1Q = PhiQ*P0Q*PhiQ' + sigmaQ2;






   




