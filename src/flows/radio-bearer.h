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


#ifndef RADIOBEARER_H_
#define RADIOBEARER_H_

#include "radio-bearer-instance.h"

class NetworkNode;
class ClassifierParameters;
class Application;
class MacQueue;
class QoSParameters;
class Packet;
class RlcEntity;

class RadioBearer : public RadioBearerInstance {
public:
	RadioBearer();
	virtual ~RadioBearer();

	MacQueue* GetMacQueue (void);

	void SetApplication (Application* a);
	Application* GetApplication (void);

	void UpdateTransmittedBytes (int bytes);
	int GetTransmittedBytes (void) const;
	void ResetTransmittedBytes (void);
	void UpdateAverageTransmissionRate ();
	double GetAverageTransmissionRate (void) const;
	void SetLastUpdate (void);
	double GetLastUpdate (void) const;


	void Enqueue (Packet *packet);
	bool HasPackets (void);

	Packet* CreatePacket (int bytes);

	void CheckForDropPackets ();

	int GetQueueSize (void);
	double GetHeadOfLinePacketDelay (void);
	int GetHeadOfLinePacketSize (void);
	int GetByte (int byte); //for FLS scheduler

//functions (from matrix Kalman implementation)
	void initialize();
	void subFilter();
	double filter(double);

private:
	Application* m_application;

	MacQueue *m_macQueue;

	// Scheduler Information
	double m_averageTransmissionRate;
	int m_transmittedBytes;
	double m_lastUpdate;

	// Kalman filter variables

	



//auxiliary
	int n,i,j,k;
	double Mat1[2][2],Mat2[2][2],Vec1[2],Vec2[2],aux;  

//global variables
	double Q[2][2];
	double R;
	double H[2];
	double P0[2][2];
	double X;
	double Y;
	double Z;
	double W;
	double Phi[2][2];
	double P1Q[2][2];
	double q1[2];
	double I[2][2];
	double P1[2][2];
	double X0[2];

//filter variables
	double X1[2];
	double IT;
	double K[2];

//Subfilter
	double eta2;
	double sigmaQ2[2][2];
	double zk;
	double M[2];
	double q0[2];
	double KQ[2];
	double PhiQ[2][2];
	double P0Q[2][2];

/*//void printMat(float **, int , int);
	double randn (double , double );
*/
};

#endif /* RADIOBEARER_H_ */
