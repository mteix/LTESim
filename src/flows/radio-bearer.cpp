/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010,2011,2012,2013 TELEMATICS LAB, Politecnico di Bari
 *
 * This file is part of LTE-Sim
 *
 * LTE-Sim is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation;
 *
 * LTE-Sim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with LTE-Sim; if not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Giuseppe Piro <g.piro@poliba.it>
 */


#include "radio-bearer.h"
#include "../device/NetworkNode.h"
#include "../device/UserEquipment.h"
#include "../device/IPClassifier/ClassifierParameters.h"
#include "application/Application.h"
#include "../protocolStack/packet/Packet.h"
#include "../flows/MacQueue.h"
#include "../flows/QoS/QoSParameters.h"
#include "../flows/QoS/QoSForEXP.h"
#include "../flows/QoS/QoSForFLS.h"
#include "../flows/QoS/QoSForM_LWDF.h"
#include "../protocolStack/rlc/rlc-entity.h"
#include "../protocolStack/rlc/tm-rlc-entity.h"
#include "../protocolStack/rlc/um-rlc-entity.h"
#include "../protocolStack/rlc/am-rlc-entity.h"
#include "../protocolStack/rlc/amd-record.h"
#include "../load-parameters.h"
#include <cmath>


//##########################################################################################
//Function: initialize
//obs.: Initialize filter parameters
//
//
//##########################################################################################


RadioBearer::RadioBearer()
{
  m_macQueue = new MacQueue ();
  m_application = NULL;

  //RlcEntity *rlc = new TmRlcEntity ();
  RlcEntity *rlc = new AmRlcEntity ();
  //RlcEntity *rlc = new UmRlcEntity ();

  rlc->SetRadioBearer (this);
  SetRlcEntity(rlc);

  // Kalman Filter initial pararameters
  m_averageTransmissionRate = 1000000; //start value = 1kbps
  
 /* P0 = 1.0;
  Q = pow(10,-6);
  R = pow(.1,2);


  // Sub-Filter definitions

  sigmaQ2 = pow(10,-32);
  P1Q = P0;  // changed on 23-Jan-2019
  q1 = 1.0; // changed on 21-Jan-2019 





  P0 =pow(10,6);
  //P0 = 1;
  Q = pow(10,6);
  R = pow(.1,2);
  q1 = 1.0;
  P1Q = pow(10,-6);
  sigmaQ2 = pow(10,-32);
*/
  ResetTransmittedBytes ();
}

RadioBearer::~RadioBearer()
{
  Destory ();
  delete m_macQueue;
  m_application = NULL;
}

MacQueue*
RadioBearer::GetMacQueue (void)
{
  return m_macQueue;
}

void
RadioBearer::SetApplication (Application* a)
{
  m_application = a;
}

Application*
RadioBearer::GetApplication (void)
{
  return m_application;
}

void
RadioBearer::UpdateTransmittedBytes (int bytes)
{
  m_transmittedBytes += bytes;
}

int
RadioBearer::GetTransmittedBytes (void) const
{
  return m_transmittedBytes;
}

void
RadioBearer::ResetTransmittedBytes (void)
{
  m_transmittedBytes = 0;
  SetLastUpdate ();
}

void
RadioBearer::UpdateAverageTransmissionRate ()
{
  /*
   * Update Transmission Data Rate with
   * a Moving Average
   * R'(t+1) = (0.8 * R'(t)) + (0.2 * r(t))
   */

  double rate = (GetTransmittedBytes () * 8)/(Simulator::Init()->Now() - GetLastUpdate());

/*Kalman filter and subfilter implementation using matrices*/

RadioBearer::initialize();
m_averageTransmissionRate = RadioBearer::filter(rate);


/*End filter and subfilter*/

#ifdef SCHEDULER_DEBUG
  std::cout << "******* SCH_DEB UPDATE AVG RATE, bearer  " << GetApplication ()->GetApplicationID () <<
  "\n\t tx byte " << GetTransmittedBytes () <<
  "\n\t interval " << Simulator::Init()->Now() - GetLastUpdate() <<
  "\n\t old rate " << m_averageTransmissionRate <<
  "\n\t new rate " << rate <<
  "\n\t new estimated rate ******** SCH_DEB" << m_averageTransmissionRate << std::endl;
#endif


  ResetTransmittedBytes();
}

double
RadioBearer::GetAverageTransmissionRate (void) const
{
  return m_averageTransmissionRate;
}

void
RadioBearer::SetLastUpdate (void)
{
  m_lastUpdate = Simulator::Init()->Now();
}

double
RadioBearer::GetLastUpdate (void) const
{
  return m_lastUpdate;
}

Packet*
RadioBearer::CreatePacket (int bytes)
{
  Packet *p = new Packet ();

  p->SetID(Simulator::Init()->GetUID ());
  p->SetTimeStamp(Simulator::Init()->Now ());

  UDPHeader *udp = new UDPHeader (GetClassifierParameters ()->GetSourcePort(),
    GetClassifierParameters ()->GetDestinationPort ());
  p->AddUDPHeader(udp);

  IPHeader *ip = new IPHeader (GetClassifierParameters ()->GetSourceID (),
   GetClassifierParameters ()->GetDestinationID());
  p->AddIPHeader(ip);

  PDCPHeader *pdcp = new PDCPHeader ();
  p->AddPDCPHeader (pdcp);

  RLCHeader *rlc = new RLCHeader ();
  p->AddRLCHeader(rlc);

  PacketTAGs *tags = new PacketTAGs ();
  tags->SetApplicationType(PacketTAGs::APPLICATION_TYPE_INFINITE_BUFFER);
  p->SetPacketTags(tags);

  if (_APP_TRACING_)
  {
	  /*
	   * Trace format:
	   *
	   * TX   APPLICATION_TYPE   BEARER_ID  SIZE   SRC_ID   DST_ID   TIME
	   */
   UserEquipment* ue = (UserEquipment*) GetApplication ()->GetDestination ();
   std::cout << "TX";
   switch (p->GetPacketTags ()->GetApplicationType ())
   {
    case Application::APPLICATION_TYPE_VOIP:
    {
     std::cout << " VOIP";
     break;
   }
   case Application::APPLICATION_TYPE_TRACE_BASED:
   {
    std::cout << " VIDEO";
    break;
  }
  case Application::APPLICATION_TYPE_CBR:
  {
   std::cout << " CBR";
   break;
 }
 case Application::APPLICATION_TYPE_INFINITE_BUFFER:
 {
   std::cout << " INF_BUF";
   break;
 }
 default:
 {
   std::cout << " UNDEFINED";
   break;
 }
}

if (bytes > 1490) bytes = 1490;
else bytes = bytes - 13;

std::cout << " ID " << p->GetID ()
<< " B " << GetRlcEntity ()->GetRlcEntityIndex ()
<< " SIZE " << bytes
<< " SRC " << GetSource ()->GetIDNetworkNode ()
<< " DST " << GetDestination ()->GetIDNetworkNode ()
<< " T " << Simulator::Init()->Now()
<< " " << ue->IsIndoor () << std::endl;
}

return p;
}

int
RadioBearer::GetQueueSize (void)
{
  int size = 0;
  size += GetMacQueue ()->GetQueueSizeWithMACHoverhead ();

  if (GetRlcEntity ()->GetRlcModel () == RlcEntity::AM_RLC_MODE)
  {
    AmRlcEntity* amRlc = (AmRlcEntity*) GetRlcEntity ();
    size += amRlc->GetSizeOfUnaknowledgedAmd ();
  }

  return size;
}

double
RadioBearer::GetHeadOfLinePacketDelay (void)
{
  double HOL = 0.;
  double now = Simulator::Init()->Now();
  if (GetRlcEntity ()->GetRlcModel () == RlcEntity::AM_RLC_MODE)
  {
   AmRlcEntity* amRlc = (AmRlcEntity*) GetRlcEntity ();
   if (amRlc->GetSentAMDs()->size() > 0)
   {
    HOL = now - amRlc->GetSentAMDs()->at (0)->m_packet->GetTimeStamp ();
  }
  else
  {
    HOL = now - GetMacQueue ()->Peek ().GetTimeStamp ();
  }
}
else
{
 HOL = now - GetMacQueue ()->Peek().GetTimeStamp ();
}

if (HOL < 0.00001) HOL = 0.00001;

return HOL;
}

void
RadioBearer::CheckForDropPackets ()
{
  if (m_application->GetApplicationType ()  != Application::APPLICATION_TYPE_TRACE_BASED
    &&
    m_application->GetApplicationType ()  != Application::APPLICATION_TYPE_VOIP)
  {
   return;
 }

 GetMacQueue()->CheckForDropPackets(
  GetQoSParameters()->GetMaxDelay(), m_application->GetApplicationID ());
}

void
RadioBearer::Enqueue (Packet *packet)
{
#ifdef TEST_ENQUEUE_PACKETS
  std::cout << "Enqueue packet on " << GetSource ()->GetIDNetworkNode () << std::endl;
#endif

  GetMacQueue ()->Enqueue(packet);
}


bool
RadioBearer::HasPackets (void)
{
  if (m_application->GetApplicationType ()  == Application::APPLICATION_TYPE_INFINITE_BUFFER)
  {
   return true;
 }

 if (GetRlcEntity ()->GetRlcModel () == RlcEntity::AM_RLC_MODE)
 {
   AmRlcEntity* amRlc = (AmRlcEntity*) GetRlcEntity ();
   if (amRlc->GetSentAMDs()->size() > 0)
   {
    return true;
  }
  else
  {
    return !GetMacQueue ()->IsEmpty();
  }
}
else
{
 return !GetMacQueue ()->IsEmpty();
}
}

int
RadioBearer::GetHeadOfLinePacketSize (void)
{
  int size = 0;
  if (GetRlcEntity ()->GetRlcModel () == RlcEntity::AM_RLC_MODE)
  {
    AmRlcEntity* amRlc = (AmRlcEntity*) GetRlcEntity ();
    if (amRlc->GetSentAMDs()->size() > 0)
    {
      size = amRlc->GetSizeOfUnaknowledgedAmd ();
    }
    else
    {
      size = GetMacQueue ()->Peek ().GetSize () - GetMacQueue ()->Peek ().GetFragmentOffset();
    }
  }
  else
  {
    size = GetMacQueue ()->Peek ().GetSize () - GetMacQueue ()->Peek ().GetFragmentOffset();
  }

  return size;
}

int
RadioBearer::GetByte (int byte)
{
	int maxData= 0;
	if (GetRlcEntity ()->GetRlcModel () == RlcEntity::AM_RLC_MODE)
 {
  AmRlcEntity* amRlc = (AmRlcEntity*) GetRlcEntity ();
  std::vector<AmdRecord*>* am_segments = amRlc->GetSentAMDs ();
  if (am_segments->size() > 0)
  {
   for (int i=0; i < am_segments->size(); i++)
   {
    maxData =+ am_segments->at (i)->m_packet->GetSize () + 6;
    if (maxData >= byte) continue;
  }
}
}

if (maxData < byte)
{
  maxData += GetMacQueue ()->GetByte (byte - maxData);
}

return maxData;

}


// The Kalman filter and subfilter functions
void 
RadioBearer::initialize(){

  #ifdef DEBUG
  printf("initialize()\n");
  #endif

  n = 1 ; 

  Q[0][0]=1.0;
  Q[0][1]=0.0;
  Q[1][0]=0.0;
  Q[1][1]=1.0;

  R = 1.0; 
  H[0] = 1.0;
  H[1] = 0.0;

  P0[0][0] = 1.0E+6; 
  P0[0][1] = 0.0;
  P0[1][0] = 0.0;
  P0[1][1] = 1.0E+6;

  X = 0.0;
  Z = X; 
  W = X;

  Phi[0][0] = 1.0; 
  Phi[0][1] = 1.0;
  Phi[1][0] = 0.0;
  Phi[1][1] = 1.0;

    P1Q[0][0] = 1.0E+6; //10^6*1
    P1Q[0][1] = 0.0;
    P1Q[1][0] = 0.0;
    P1Q[1][1] = 1.0E+6; //10^6*1
    
    q1[0] = 1.0;
    q1[1] = 1.0; 
    
    I[0][0]=1.0;
    I[0][1]=0.0;
    I[1][0]=0.0;
    I[1][1]=1.0;
    
    X0[0]= 0.0;
    X0[1]= 0.0;

}

double 
RadioBearer::filter(double rate){

    #ifdef DEBUG
  printf("Filter() \n");
    #endif

// Define the value of X based on input rate
  Y = rate;

//P1 = Phi*P0*Phi' + Q; //vetor
  for(i=0;i<2;i++){
    for(j=0;j<2;j++){
      Mat1[i][j]=0.0;
      for(k=0;k<2;k++)
        Mat1[i][j] += Phi[i][k] * P0[k][j] ;
    }
  }

  /*#ifdef DEBUG
    printf("\nMat1:\n [%f %f] \n [%f %f] \n\n",Mat1[0][0],Mat1[0][1],Mat1[1][0],Mat1[1][1]);
    #endif
  */
  for(i=0;i<2;i++){
    for(j=0;j<2;j++){
      Mat2[i][j]=0.0;
      for(k=0;k<2;k++)
        Mat2[i][j] += Mat1[i][k] * Phi[j][k] ;
    }
  }

  /*#ifdef DEBUG
    printf("\nMat2:\n [%f %f] \n [%f %f] \n\n",Mat2[0][0],Mat2[0][1],Mat2[1][0],Mat2[1][1]);
    #endif
  */
  for(i=0;i<2;i++){
    for(j=0;j<2;j++){
      P1[i][j] = Mat2[i][j] + Q[i][j] ;
    }
  }

  #ifdef DEBUG
  printf("\nP1:\n [%f %f] \n [%f %f] \n\n",P1[0][0],P1[0][1],P1[1][0],P1[1][1]);
    #endif

//K = P1*H'/(H*P1*H'+R); //vetor
  for(i=0;i<2;i++){
    Vec1[i]=0.0;
    for(k=0;k<2;k++)
      Vec1[i] += P1[i][k] * H[k] ;
  }

  for(i=0;i<2;i++){
    Vec2[i]=0.0;
    for(k=0;k<2;k++)
      Vec2[i] += P1[i][k] * H[k] ;
  }

  aux = 0.0 ;
  for(k=0;k<2;k++)
    aux  +=  H[k] * Vec2[k]  ;

  for(i=0;i<2;i++)
    K[i] = Vec1[i] / (aux+R) ; 

  #ifdef DEBUG
  printf("\nK:\n [%f] \n [%f] \n\n",K[0],K[1]);
  #endif

//X1 = Phi*X0;  //vetor
  for(i=0;i<2;i++){
    X1[i]=0;
    for(k=0;k<2;k++)
      X1[i] += Phi[i][k] * X0[k] ;
  }

   #ifdef DEBUG
  printf("\nX1:\n [%f] \n [%f] \n\n",X1[0],X1[1]);
   #endif

//IT = Y - H*X1; //double
  aux = 0.0 ;
  for(k=0;k<2;k++)
    aux  +=  H[k] * X1[k]  ;

  IT = Y - aux ;

  #ifdef DEBUG
  printf("\nIT: %f \n",IT);
  #endif

// if IT/(H*P1*H'+R) >= 3, 
//     IT = 3*(H*P1*H'+R); 
// end

// if IT/(H*P1*H'+R) <= -3, 
//     IT = -3*(H*P1*H'+R); 
// end

  for(i=0;i<2;i++){
    Vec2[i]=0.0;
    for(k=0;k<2;k++)
      Vec2[i] += P1[i][k] * H[k] ;
  }

  aux = 0.0 ;
  for(k=0;k<2;k++)
    aux  +=  H[k] * Vec2[k]  ;

  double HP1HR = aux + R ;

  if ((IT/HP1HR) >= 3) {
    IT = 3*HP1HR; 
  }

  if ((IT/HP1HR) <= -3) {
    IT = -3*HP1HR; 
  }

// X0 = X1 + K*IT;
  for(i=0;i<2;i++)
    X0[i] = X1[i] + K[i]*IT ;

     #ifdef DEBUG
  printf("\nX0:\n [%f] \n [%f] \n\n",X0[0],X0[1]);
     #endif

//P0 = (I - K*H)*P1;    Calculation of pseudo-observation vector
  for(i=0;i<2;i++)
    for(j=0;j<2;j++)
      Mat1[i][j] = K[i] * H[j] ;

    for(i=0;i<2;i++)
      for(j=0;j<2;j++)
        Mat2[i][j] = I[i][j] - Mat1[i][j] ;

      for(i=0;i<2;i++)
        for(j=0;j<2;j++){
          P0[i][j] = 0.0;
          for(k=0;k<2;k++)
            P0[i][j] += (Mat2[i][k] * P1[k][j]) ;
        } 

      #ifdef DEBUG
      printf("\nP0:\n [%f %f] \n [%f %f] \n\n",P0[0][0],P0[0][1],P0[1][0],P0[1][1]);
      #endif

 //[q1,P1Q] = Covmod_q(IT,R,Phi,H,P0,Q,q1,P1Q);
 subFilter();

 //for j = 1:2, 
 //    if q1(j,1) >= 0,  
 //        Q(j,j) = q1(j,1);
 //    else 
 //        Q(j,j) = 0; 
 //    end, 
 //end
      int j;
      for (j=0;j<2;j++){
        if (q1[j] >= 0){
          Q[j][j] = q1[j];
        }else{
          Q[j][j] = 0;
        }
      }

    #ifdef DEBUG
      printf("\nQ:\n [%f %f] \n [%f %f] \n\n",Q[0][0],Q[0][1],Q[1][0],Q[1][1]);
    #endif

//X(1) = H*X1;         One-step ahead prediction
      aux = 0.0 ;
      for(k=0;k<2;k++)
        aux  +=  H[k] * X1[k]  ;

      X = aux ;

//Z(1) = H*X0;         Filtered value
      aux = 0.0 ;
      for(k=0;k<2;k++)
        aux  +=  H[k] * X0[k]  ;

      Z = aux ;

//W(1) = H*(Phi^n)*X0; 
      for(i=0;i<2;i++){
        Vec1[i]=0.0;
        for(k=0;k<2;k++)
          Vec1[i] += pow(Phi[i][k],n) * X0[k] ;
      }

      aux = 0.0 ;
      for(k=0;k<2;k++)
        aux  +=  H[k] * Vec1[k]  ;

      W = aux ;
      return Z;

    }

void
RadioBearer::subFilter(){

#ifdef DEBUG
      printf("subFilter() \n");
#endif

//eta2 = 4*IT^2*R + 2*R^2;   
      eta2=4*IT*IT*R+2*R*R;

    #ifdef DEBUG
      printf("\neta2: %f \n",eta2);
    #endif   

//sigmaQ2 = 10^(-2*16)*[1 0; 0 1]; 
sigmaQ2[0][0] = 10.0E-32;//10^(-2*16)*1;
sigmaQ2[0][1] = 0;
sigmaQ2[1][0] = 0;
sigmaQ2[1][1] = 10.0E-32;

//zk = IT^2 + R - H*Phi*P0*Phi'*H'; //double

for(i=0;i<2;i++){
  for(j=0;j<2;j++){
    Mat1[i][j]=0.0;
    for(k=0;k<2;k++)
      Mat1[i][j] += Phi[i][k] * P0[k][j] ;
  }
}


for(i=0;i<2;i++){
  for(j=0;j<2;j++){
    Mat2[i][j]=0.0;
    for(k=0;k<2;k++)
      Mat2[i][j] += Mat1[i][k] * Phi[j][k] ;
  }
}


for(i=0;i<2;i++){
  Vec1[i]=0.0;
  for(k=0;k<2;k++)
    Vec1[i] += Mat2[i][k] * H[k] ;
}

aux = 0.0 ;
for(k=0;k<2;k++)
  aux  +=  H[k] * Vec1[k]  ;


zk = IT*IT + R - aux ;

    #ifdef DEBUG
printf("\nzk: %f \n",zk);
    #endif

//M = [1 0];
M[0] = 1;
M[1] = 0;

//PhiQ = [1 0; 0 1];
PhiQ[0][0] = 1; 
PhiQ[1][0] = 0;
PhiQ[0][1] = 0;
PhiQ[1][1] = 1;

//calculo da pseudo-observaçao
//KQ = P1Q*M'/(M*P1Q*M'+eta2); //vetor 2 linha 1 coluna
for(i=0;i<2;i++){
  Vec1[i]=0.0;
  for(k=0;k<2;k++)
    Vec1[i] += P1Q[i][k] * M[k] ;
}

for(i=0;i<2;i++){
  Vec2[i]=0.0;
  for(k=0;k<2;k++)
    Vec2[i] += P1Q[i][k] * M[k] ;
}

aux = 0.0 ;
for(k=0;k<2;k++)
  aux  +=  M[k] * Vec2[k]  ;

for(i=0;i<2;i++)
  KQ[i] = Vec1[i] / (aux+eta2) ; 

    #ifdef DEBUG
printf("\nKQ: [%f]\n [%f] \n",KQ[0],KQ[1]);
    #endif
//q0 = q1+KQ*(zk-M*q1); //vetor
aux = 0.0 ;
for(k=0;k<2;k++)
  aux  +=  M[k] * q1[k]  ;

for(i=0;i<2;i++)
  q0[i] = q1[i] + KQ[i] * (zk - aux) ; 

    #ifdef DEBUG
printf("\nq0: [%f]\n [%f] \n",q0[0],q0[1]);
    #endif

//P0Q = P1Q - KQ*M*P1Q;  //vetor
for(i=0;i<2;i++){
  for(j=0;j<2;j++)
    Mat1[i][j] = KQ[i] * M[j] ;
}

for(i=0;i<2;i++){
  for(j=0;j<2;j++){
    Mat2[i][j]=0.0;
    for(k=0;k<2;k++)
      Mat2[i][j] += Mat1[i][k] * P1Q[k][j] ;
  }
}

for(i=0;i<2;i++)
  for(j=0;j<2;j++)
    P0Q[i][j] = P1Q[i][j] - Mat2[i][j] ;

    #ifdef DEBUG
  printf("\nP0Q:\n [%f %f] \n [%f %f] \n\n",P0Q[0][0],P0Q[0][1],P0Q[1][0],P0Q[1][1]);
    #endif

//q1 = PhiQ*q0; //vetor
  for(i=0;i<2;i++){
    q1[i]=0;
    for(k=0;k<2;k++)
      q1[i] += PhiQ[i][k] * q0[k] ;
  }

    #ifdef DEBUG
  printf("\nq1: [%f]\n [%f] \n",q1[0],q1[1]);
    #endif

//P1Q = PhiQ*P0Q*PhiQ' + sigmaQ2; //vetor
  for(i=0;i<2;i++){
    for(j=0;j<2;j++){
      Mat1[i][j]=0.0;
      for(k=0;k<2;k++)
        Mat1[i][j] += PhiQ[i][k] * P0Q[k][j] ;
    }
  }
  
  for(i=0;i<2;i++){
    for(j=0;j<2;j++){
      Mat2[i][j]=0.0;
      for(k=0;k<2;k++)
        Mat2[i][j] += Mat1[i][k] * PhiQ[j][k] ;
    }
  }
  
  for(i=0;i<2;i++)
    for(j=0;j<2;j++)
      P1Q[i][j] = Mat2[i][j] + sigmaQ2[i][j] ;

   #ifdef DEBUG
    printf("\nP1Q:\n [%f %f] \n [%f %f] \n\n",P1Q[0][0],P1Q[0][1],P1Q[1][0],P1Q[1][1]);
    printf("\nsigmaQ2: %lf \n\n",sigmaQ2[1][1]);
   #endif

  }    
