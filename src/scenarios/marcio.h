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

#include "../channel/LteChannel.h"
#include "../core/spectrum/bandwidth-manager.h"
#include "../networkTopology/Cell.h"
#include "../core/eventScheduler/simulator.h"
#include "../flows/application/InfiniteBuffer.h"
#include "../flows/QoS/QoSParameters.h"
#include "../componentManagers/FrameManager.h"
#include "../componentManagers/FlowsManager.h"
#include "../device/ENodeB.h"




static void Marcio ()
{

  // CREATE COMPONENT MANAGERS
  Simulator *simulator = Simulator::Init();
  FrameManager *frameManager = FrameManager::Init();
  NetworkManager* networkManager = NetworkManager::Init();  //é chamada pela classe FrameManager
  FlowsManager* flowsManager = FlowsManager::Init ();


  //Create CHANNELS
  LteChannel *dlCh = new LteChannel ();
  LteChannel *ulCh = new LteChannel ();


  // CREATE SPECTRUM
  BandwidthManager* spectrum = new BandwidthManager (5, 5, 0, 0);


  // CREATE CELL 

  //SOME CHANGES HERE TO 
  int idCell = 0;
  int radius = 2; //km
  int minDistance = 0.0035; //km
  int posX = 10;
  int posY = 10;
  Cell* cell = networkManager->CreateCell (idCell, radius, minDistance, posX, posY);


  int idEnb = 1;
  ENodeB* enb = networkManager->CreateEnodeb (idEnb, cell, posX, posY, dlCh, ulCh, spectrum);

  enb->SetDLScheduler (ENodeB::DLScheduler_TYPE_PROPORTIONAL_FAIR);
  enb->SetULScheduler(ENodeB::ULScheduler_TYPE_ROUNDROBIN);
 

/*//CELL # 2
  idCell= 1;
  radius = .5;
  minDistance = 0.0035;
  posX = 100;
  posY = 100;
  Cell* cell2 = networkManager->CreateCell (idCell, radius, minDistance, posX, posY);
*/


/*  //Create ENodeB
  
  idEnb = 2;posX=100;
  ENodeB* enb2 = networkManager->CreateEnodeb (idEnb, cell, posX, posY, dlCh, ulCh, spectrum);

  enb2->SetDLScheduler (ENodeB::DLScheduler_TYPE_PROPORTIONAL_FAIR);
  enb2->SetULScheduler(ENodeB::ULScheduler_TYPE_MAXIMUM_THROUGHPUT);
 
*/  //Create GW
  Gateway *gw = networkManager->CreateGateway ();



/*Creating more UES
*/  
//Create UE
  int idUe = 2;
  int posX_ue = 400; //m
  int posY_ue = 0;  //m
  int speed = 3;    //km/h
  double speeDirection = 0;
  UserEquipment* ue2 = networkManager->CreateUserEquipment (idUe, posX_ue, posY_ue, speed, speeDirection, cell, enb);
 
//Create UE
  idUe = 1;
  posX_ue = 300; //m
  posY_ue = 0;  //m
  speed = 0;    //km/h
  speeDirection = 0;
  UserEquipment* ue1 = networkManager->CreateUserEquipment (idUe, posX_ue, posY_ue, speed, speeDirection, cell, enb);

 

  //Create an Application
  QoSParameters *qos = new QoSParameters ();
  int applicationID = 0;
  int srcPort = 0;
  int dstPort = 100;
  double startTime = 0.1; //s
  double stopTime = 0.12;  //s
  Application* be1 = flowsManager->CreateApplication (applicationID,
    gw, ue1,
    srcPort, dstPort, TransportProtocol::TRANSPORT_PROTOCOL_TYPE_UDP ,
    Application::APPLICATION_TYPE_INFINITE_BUFFER,
    qos,
    startTime, stopTime);

    Application* be2 = flowsManager->CreateApplication (applicationID,
    gw, ue2,
    srcPort, dstPort, TransportProtocol::TRANSPORT_PROTOCOL_TYPE_UDP ,
    Application::APPLICATION_TYPE_INFINITE_BUFFER,
    qos,
    startTime, stopTime);
 
  simulator->SetStop(0.13);
  simulator->Run ();


}
