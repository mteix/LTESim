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
//#include <cmath>

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
  
  P0 = 1.0;
  Q = pow(10,-6);
  R = pow(.1,2);


  // Sub-Filter definitions

  sigmaQ2 = pow(10,-32);
  P1Q = pow(10,0); // changed to 1 on 14-May-2019


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
/*  double beta = 0.2;

  m_averageTransmissionRate =
      ((1 - beta) * m_averageTransmissionRate) + (beta * rate);

  if (m_averageTransmissionRate < 1)
    {
	  m_averageTransmissionRate = 1;
    }
*/



  // 28-Nov-2018 by MJT 
  //#####################################
  //########## KALMAN FILTER ############
  //#####################################


  // 1. rate is the observation (z)
  // 2. estimates for initial P0 and m_averageTransmissionRate (xhat) must 
  // be provided


  //Kalman estimates

  xhatminus = m_averageTransmissionRate;
  Pminus = P0 + Q;

  //Kalman updates
  K = Pminus/(Pminus + R);
  m_averageTransmissionRate = xhatminus + K*(rate - xhatminus);
  P0 = (1-K)*Pminus;

/*// 02-Dec-2018 by MJT
=======
/*
// 02-Dec-2018 by MJT
>>>>>>> a79c470f55087ff7451e20e1f3debb882628d817
//##########################
//  KALMAN SUB-FILTER
//##########################

q1 = m_averageTransmissionRate;
eta2 = 4*(rate - xhatminus)*(rate - xhatminus)*R + 2*R*R;
// declared in beginning of class sigmaQ2 = pow(10,-32); // 2x 16 digit precision
zk = (rate - xhatminus)*(rate - xhatminus)+R-P0;
kq = P1Q/(P1Q+eta2);
q0 = (q1+kq)*(zk-q1);
P0Q = P1Q*(1-kq);
q1 = q0;
P1Q = P0Q + sigmaQ2;

if(q1>0)
  Q = q1;
else 
  Q = 0;


#ifdef MY_DEBUG
  std::cerr << "\n ------> Sub-Filter parameters \n " <<

  " q1 " << q1 << " eta2 " << eta2 <<
  "   kq " << kq << "  q0 " << q0 <<
  "   P0Q " << P0Q << "  P1Q \n" << P1Q 

  << std::endl;
#endif


//##########################
// END KALMAN SUB-FILTER
//##########################

*/

#ifdef SCHEDULER_DEBUG
  std::cout << "******* SCH_DEB UPDATE AVG RATE, bearer  " << GetApplication ()->GetApplicationID () <<
		  "\n\t tx byte " << GetTransmittedBytes () <<
		  "\n\t interval " << Simulator::Init()->Now() - GetLastUpdate() <<
		  "\n\t Kalman output  " << m_averageTransmissionRate <<
          "\n\t  rate " << rate << std::endl; 
		  std::cout << "*********************************************" << std::endl;
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
