// ---------------------------------------------------------------------------
// CERTI - HLA RunTime Infrastructure
// Copyright (C) 2002  ONERA
//
// This file is part of CERTI-libcerti
//
// CERTI-libcerti is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// CERTI-libcerti is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
//
// $Id: SocketUN.cc,v 3.0.2.1 2002/11/22 00:52:57 breholee Exp $
// ---------------------------------------------------------------------------

#include <config.h>
 
#include "SocketUN.hh"

namespace certi {

#define MAX_ATTEMPTS 3

// ----------------
// -- AccepterUN --
// ----------------

void SocketUN::acceptUN()
{
  struct sockaddr_un nom_client, nom_serveur;
  socklen_t lg_nom;
  
  pD->Out(pdInit, "Opening Server UNIX Socket.");

  // Socket
  if((sock_connect = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    error("socket");
  
  pD->Out(pdInit, "Server has got UNIX Socket FpD->");

  // Set Server address
  memset(&nom_serveur, 0, sizeof(nom_serveur));

  nom_serveur.sun_family = AF_UNIX;

  if(strlen(NOM_FICHIER_SOCKET) > 90)
    throw RTIinternalError("NOM_FICHIER_SOCKET too long.");
  sprintf(SocketName, "%s.%d",  NOM_FICHIER_SOCKET, getpid());
  strcpy(nom_serveur.sun_path, SocketName);

  // Bind
  if(bind(sock_connect,(struct sockaddr*)&nom_serveur,
	  sizeof(struct sockaddr_un)) < 0)
    error("bind");

  pD->Out(pdInit, "Server: Bind succeeded, now listening.");

  // Listen
  if(listen(sock_connect, 10) == -1)
    error("listen");

  pD->Out(pdInit, "Server: Listen returned, now accepting.");

  // Accept
  lg_nom = sizeof(struct sockaddr_un);
  if((_socket_un = accept(sock_connect,
			(struct sockaddr*)&nom_client,
			  &lg_nom) ) < 0)
    // HPUX:(int*) &lg_nom) ) < 0)
    error("accept");

  pD->Out(pdInit, "Server: Accept OK, server running.");

  _est_init_un = RTI_TRUE;
  _est_serveur = RTI_TRUE;
}


// -----------------
// -- ConnecterUN --
// -----------------

void SocketUN::connectUN(pid_t Server_pid)
{
  int                Attempt     = 0;
  int                Result;
  struct sockaddr_un nom_serveur;

  while(Attempt < MAX_ATTEMPTS) { 

    pD->Out(pdInit, "Opening Client UNIX Socket.");

    // Socket
    if((_socket_un = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
      error("socket");
  
    pD->Out(pdInit, "Client has got UNIX Socket FpD->");

    // Clear and set Server adress
    memset(&nom_serveur, 0, sizeof(nom_serveur));

    nom_serveur.sun_family = AF_UNIX;

    if(strlen(NOM_FICHIER_SOCKET) > 90)
      throw RTIinternalError("NOM_FICHIER_SOCKET too long.");

    sprintf(SocketName, "%s.%d",  NOM_FICHIER_SOCKET, Server_pid);
    strcpy(nom_serveur.sun_path, SocketName);

    // Connect
    Result = connect(_socket_un,
		(struct sockaddr*) &nom_serveur,
		     sizeof(struct sockaddr_un));
    
    pD->Out(pdInit, "Client: Connect returned %d.", Result);

    // Success ? Yes->break
    if(Result == 0)
      break;

    // Failure
    printf("SocketUN: Connect, attempt #%d out of %d failed\n",
	    Attempt + 1, MAX_ATTEMPTS);
    sleep(1);
    Attempt ++;
  }

  pD->Out(pdInit, "Client: Done.");

  if(Result != -1)
    _est_init_un = RTI_TRUE;
  else
    error("Connect");
}


// ---------------
// -- SocketUN --
// ---------------

SocketUN::SocketUN(SignalHandlerType theType)
{
  _est_serveur   = RTI_FALSE;
  _est_init_un   = RTI_FALSE;

  HandlerType    = theType;
  SocketName [0] = '\0';

  SentBytesCount = 0;
  RcvdBytesCount = 0;

#ifdef SOCKUN_BUFFER_LENGTH
  RBLength = 0;
#endif

  pD = new pdCDebug("SOCKUN", "(SocketUN  ) - ");

  pD->Out(pdInit, "UNIX Socket created.");

}


// ----------------
// -- ~SocketUN --
// ----------------

SocketUN::~SocketUN()
{
  if(_est_init_un)
  {
    close(_socket_un);
    if(_est_serveur == RTI_TRUE)
      close(sock_connect);
    unlink(SocketName);
    if(_est_serveur == RTI_TRUE)
      pD->Out(pdTerm, "Server: Closed all sockets.");
    else
      pD->Out(pdTerm, "Client: Closed all sockets.");
  }

  delete pD;

#ifdef RTI_PRINTS_STATISTICS
  printf("\n");
  printf("UNIX Socket(%d): Total Sent Bytes      : %lld.\n",
	  _socket_un, SentBytesCount);
  printf("UNIX Socket(%d): Total Received Bytes  : %lld.\n",
	  _socket_un, RcvdBytesCount);
  printf("\n");
#endif
}

// ---------------------------------------------------------------------------
// send

void 
SocketUN::send(void *buffer, unsigned long size)
  throw(NetworkError, NetworkSignal)
{
  long sent = 0;
  unsigned long total_sent = 0;
 
  assert(_est_init_un);

  pD->Out(pdTrace, "Beginning to send UN message...");

  while(total_sent < size) {
    sent = write(_socket_un, (char *) buffer + total_sent, size - total_sent);

    if(sent > 0) {
      total_sent += sent;
      pD->Out(pdTrace, "Sent %ld bytes out of %ld.", total_sent, size);
    }
    else {     
      if(sent < 0) {
	pD->Out(pdExcept, "Error while sending on UN socket.");
	
	// Incoming Signal
	if(errno == EINTR) {
	  if(HandlerType == stSignalInterrupt) throw NetworkSignal();
	  else pD->Out(pdExcept, "EmettreUN ignoring signal interruption.");
	}
	// Other errors
	else {
	  perror("UN Socket(EmettreUN) : ");
	  throw NetworkError("Error while sending UN message.");
	}
      }
      
      if(sent == 0) {
	pD->Out(pdExcept, "No data could be sent, connection closed?.");
	throw NetworkError("Could not send any data on UN socket.");
      }
    }
  }
  SentBytesCount += total_sent;
}


// ------------
// -- Erreur --
// ------------

void SocketUN::error(const char *msg)
{
  char m[100];

  m[0] = 0;
  strcat(m, "SocketUN: ");
  strcat(m, msg);
  perror(m);
  exit(-1);
}


// -----------------
// -- IsDataReady --
// -----------------

Boolean SocketUN::isDataReady()
{
#ifdef SOCKUN_BUFFER_LENGTH
  if(RBLength > 0)
    return RTI_TRUE;
  else
    return RTI_FALSE;
#else
  return RTI_FALSE;
#endif
}


// ----------------
// -- RecevoirUN --
// ----------------

void SocketUN::receive(void *buffer, unsigned long Size)
  throw(NetworkError,
	 NetworkSignal)
{
  assert(_est_init_un);

  long nReceived  = 0;
#ifndef SOCKUN_BUFFER_LENGTH
  long RBLength   = 0;
#endif

  pD->Out(pdTrace, "Beginning to receive UN message...");

  while(RBLength < Size)
    {

#ifdef SOCKUN_BUFFER_LENGTH
      nReceived = read(_socket_un,
			ReadBuffer + RBLength,
			SOCKUN_BUFFER_LENGTH - RBLength);
#else
      nReceived = read(_socket_un,
			(char *) buffer + RBLength,
			Size - RBLength);
#endif

      if(nReceived < 0) {
	pD->Out(pdExcept, "Error while receiving on UN socket.");

	// Incoming Signal
	if(errno == EINTR) {
	  if(HandlerType == stSignalInterrupt)
	    throw NetworkSignal();
	  else
	    pD->Out(pdExcept, "RecevoirUN ignoring signal interruption.");
	}
	// Other errors
	else {
	  perror("UN Socket(RecevoirUN) : ");
	  throw NetworkError("Error while receiving UN message.");
	}
      }

      if(nReceived == 0) {
	pD->Out(pdExcept, "UN connection has been closed by peer.");
	throw NetworkError("Connection closed by client.");
      }

      if(nReceived > 0) {
	RBLength       += nReceived;
	RcvdBytesCount += nReceived;
	pD->Out(pdTrace, "Received %ld bytes out of %ld.", RBLength, Size);
      }
    }

#ifdef SOCKUN_BUFFER_LENGTH
  memcpy(buffer,(void *) ReadBuffer, Size);
  memmove((void *) ReadBuffer, 
	(void *)(ReadBuffer + Size), 
	   RBLength - Size);
  RBLength -= Size;
#endif
}

}

// $Id: SocketUN.cc,v 3.0.2.1 2002/11/22 00:52:57 breholee Exp $
