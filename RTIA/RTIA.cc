// ---------------------------------------------------------------------------
// CERTI - HLA RunTime Infrastructure
// Copyright (C) 2002  ONERA
//
// This file is part of CERTI
//
// CERTI is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// CERTI is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// $Id: RTIA.cc,v 3.0.2.1 2002/11/22 00:52:56 breholee Exp $
// ---------------------------------------------------------------------------

#include "RTIA.hh"

namespace certi {
namespace rtia {

static pdCDebug D("RTIA", "(RTIA ) - ");

// Tableau des messages pouvant etre recus du RTIG

#define MSG_RTIG_MAX 18

char *Messages_RTIG [MSG_RTIG_MAX] = {
  "MESSAGE NULL",
  "SET_TIME_REGULATING",
  "REQUEST_PAUSE",
  "REQUEST_RESUME",
  "DISCOVER_OBJECT",
  "START_REGISTRATION_FOR_OBJECT_CLASS",
  "STOP_REGISTRATION_FOR_OBJECT_CLASS",
  "REFLECT_ATTRIBUTE_VALUES",
  "SEND_INTERACTION",
  "REMOVE_OBJECT",
  "INFORM_ATTRIBUTE_OWNERSHIP",
  "ATTRIBUTE_IS_NOT_OWNED",
  "REQUEST_ATTRIBUTE_OWNERSHIP_ASSUMPTION",
  "ATTRIBUTE_OWNERSHIP_UNAVAILABLE",
  "ATTRIBUTE_OWNERSHIP_ACQUISITION_NOTIFICATION",
  "ATTRIBUTE_OWNERSHIP_DIVESTITURE_NOTIFICATION",
  "REQUEST_ATTRIBUTE_OWNERSHIP_RELEASE",
  "CONFIRM_ATTRIBUTE_OWNERSHIP_ACQUISITION_CANCELLATION"
};

// Tableau des messages pouvant etre recus du Federe.

#define MSG_FED_MAX 52

char *Messages_Fed [MSG_FED_MAX] = {
  "CREATE_FEDERATION_EXECUTION",
  "DESTROY_FEDERATION_EXECUTION",
  "JOIN_FEDERATION_EXECUTION",
  "RESIGN_FEDERATION_EXECUTION",
  "REQUEST_PAUSE",
  "PAUSE_ACHIEVED",
  "REQUEST_RESUME",
  "RESUME_ACHIEVED",
  "PUBLISH_OBJECT_CLASS",
  "UNPUBLISH_OBJECT_CLASS",
  "PUBLISH_INTERACTION_CLASS",
  "UNPUBLISH_INTERACTION_CLASS",
  "SUBSCRIBE_OBJECT_CLASS_ATTRIBUTE",
  "UNSUBSCRIBE_OBJECT_CLASS_ATTRIBUTE",
  "SUBSCRIBE_INTERACTION_CLASS",
  "UNSUBSCRIBE_INTERACTION_CLASS",
  "REQUEST_ID",
  "REGISTER_OBJECT",
  "UPDATE_ATTRIBUTE_VALUES",
  "SEND_INTERACTION",
  "DELETE_OBJECT",
  "CHANGE_ATTRIBUTE_TRANSPORT_TYPE",
  "CHANGE_ATTRIBUTE_ORDER_TYPE",
  "CHANGE_INTERACTION_TRANSPORT_TYPE",
  "CHANGE_INTERACTION_ORDER_TYPE",
  "REQUEST_FEDERATION_TIME",
  "REQUEST_LBTS",
  "REQUEST_FEDERATE_TIME",
  "SET_LOOKAHEAD",
  "REQUEST_LOOKAHEAD",
  "TIME_ADVANCED_REQUEST",
  "NEXT_EVENT_REQUEST",
  "GET_OBJECT_CLASS_HANDLE",
  "GET_OBJECT_CLASS_NAME",
  "GET_ATTRIBUTE_HANDLE",
  "GET_ATTRIBUTE_NAME",
  "GET_INTERACTION_CLASS_HANDLE",
  "GET_INTERACTION_CLASS_NAME",
  "GET_PARAMETER_HANDLE",
  "GET_PARAMETER_NAME",
  "SET_TIME_REGULATING",
  "SET_TIME_CONSTRAINED",
  "TICK_REQUEST",
  "IS_ATTRIBUTE_OWNED_BY_FEDERATE",
  "QUERY_ATTRIBUTE_OWNERSHIP",
  "NEGOTIATED_ATTRIBUTE_OWNERSHIP_DIVESTITURE",
  "ATTRIBUTE_OWNERSHIP_ACQUISITION_IF_AVAILABLE",
  "UNCONDITIONAL_ATTRIBUTE_OWNERSHIP_DIVESTITURE",
  "ATTRIBUTE_OWNERSHIP_ACQUISITION",
  "CANCEL_NEGOTIATED_ATTRIBUTE_OWNERSHIP_DIVESTITURE",
  "ATTRIBUTE_OWNERSHIP_RELEASE_RESPONSE",
  "CANCEL_ATTRIBUTE_OWNERSHIP_ACQUISITION"
};

// ------------
// -- Compte --
// ------------

void RTIA::count()
{
#ifdef RTI_PRINTS_STATISTICS

  char* s = getenv("CERTI_NO_STATISTICS");
  if(s) return ;

  int j;
  printf("\nRTIA: Statistiques(messages traites)\n");
 
  for(j = 0; j < MSG_FED_MAX ;j++)
    printf(" Requetes Federe du type %s: %d\n", 
	   Messages_Fed [j],nb_requetes[j]);
 
  printf("\n");
 
  for(j = 0; j < MSG_RTIG_MAX ;j++)
    printf(" Messages RTIG du type %s: %d\n", 
	   Messages_RTIG [j],nb_messages[j]);
 
  printf("\n Nombre d'interactions : %d\n",nb_evenements);
  printf(" Nombre de messages RTIG : %d\n",TOTAL);
 
#endif
}


// -----------
// -- RTIA --
// -----------

RTIA::RTIA()
{
  int j;

  // No SocketServer is passed to the RootObject.
  _theRootObj = new RootObject(NULL);

  _GC = new Communications();
  _GQueues = new Queues;
  _GF = new FederationManagement(_GC);
  _GO = new ObjectManagement(_GC, _GQueues, _GF, _theRootObj);
  _GP = new OwnershipManagement(_GC, _GQueues, _GF, _theRootObj); 
  _GD = new DeclarationManagement(_GC, _GQueues, _GF, _theRootObj);
  _GT = new TimeManagement(_GC, _GQueues, _GF, _GO, _GP);

  _GF->_GT = _GT;
  _GQueues->_GF = _GF;

  TOTAL = 0;
  nb_evenements = 0;

  for(j = 0; j < MSG_FED_MAX; j++)
    nb_requetes[j]=0;
 
  for(j = 0; j < MSG_RTIG_MAX; j++)
    nb_messages[j]=0;
}


// ------------
// -- ~RTIA --
// ------------

RTIA::~RTIA()
{
  // BUG: destruction de la liaison TCP ?

  delete _GT;
  delete _GD;
  delete _GO;
  delete _theRootObj;
  delete _GF;
  delete _GQueues;
  delete _GC;

  // Affiche les statistiques 
  count();

  printf("RTIA: Fin d'execution\n");
}



// ---------------
// -- Execution --
// ---------------

// Boucle principale du RTIA

void RTIA::execute()
{
  Message *msg_un;
  NetworkMessage *msg_tcp_udp;
  int n;
 
 
  while(!_GF->_fin_execution) {
 
    msg_tcp_udp = new NetworkMessage;
    msg_un = new Message;
 
    try {
      _GC->readMessage(n, msg_tcp_udp, msg_un);
    }
    catch(NetworkSignal) {
      _GF->_fin_execution = RTI_TRUE; 
      n = 0;
      delete msg_un;
      delete msg_tcp_udp;
    }

    if(n == 1)
      {
	processNetworkMessage(msg_tcp_udp);
	delete msg_un;
      }

    if(n == 2)
      {
	processFederateRequest(msg_un);
	delete msg_tcp_udp;
      }
  }
}
}
}

// $Id: RTIA.cc,v 3.0.2.1 2002/11/22 00:52:56 breholee Exp $
