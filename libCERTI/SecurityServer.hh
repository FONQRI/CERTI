// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*- 
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
// $Id: SecurityServer.hh,v 3.1 2002/12/11 00:47:33 breholee Exp $
// ---------------------------------------------------------------------------

// $Id: SecurityServer.hh,v 3.1 2002/12/11 00:47:33 breholee Exp $
// ---------------------------------------------------------------------------
// Liste dynamique mettant gerant la securite des donnees, en
// affectant des numeros aux differents niveaux de securite, des
// niveaux a chacun des federes, et en reglementant les acces aux
// donnees par les federes.

#ifndef _CERTI_SECURITY_SERVER_HH
#define _CERTI_SECURITY_SERVER_HH

#include "List.hh"
#include "SocketServer.hh"
#include "AuditFile.hh"
#include "FederateLevelList.hh"
#include "SecureTCPSocket.hh"

namespace certi {

// ---------------------------
// -- SecurityServer Class --
// ---------------------------

// This class is an interface at the Federation Level for the previous
// class, where the Federation Handle is constant for all calls.
// It also adds security features, like a mapping between Security Levels
// Names and Level IDs, and a method to check if a Federate has a valid
// clearance to access Data at a certain level.

class SecurityServer : private List <SecurityLevel *>
{
public:

  // ------------------------------
  // -- Constructor & Destructor --
  // ------------------------------

  // Initialize the Federation's SocketServer.
  SecurityServer(SocketServer *theRTIGServer,
		 AuditFile *theAuditFile,
		 FederationHandle theFederation);

  // Free the List.
  ~SecurityServer();

  // -----------------------
  // -- Public Components --
  // -----------------------

  // This part of the security server is linked to the RTIG Audit Server.
  AuditFile *Audit;

  // ----------------------------
  // -- Socket Related Methods --
  // ----------------------------

  FederationHandle federation() const { return MyFederation; };

  // Each call to this method is passed to the RTIG's SocketServer,
  // by including our Federation Handle.
  Socket *getSocketLink(FederateHandle theFederate,
			TransportType theType = RELIABLE) const
  { return RTIG_SocketServer->getSocketLink(MyFederation, 
					    theFederate,
					    theType); };
 
  // ------------------------------
  // -- Security Related Methods --
  // ------------------------------

  Boolean dominates(SecurityLevelID A,
		    SecurityLevelID B); 

  Boolean canFederateAccessData(FederateHandle theFederate,
				SecurityLevelID theDataLevelID);

  SecurityLevelID getLevelIDWithName(SecurityLevelName theName);

  void registerFederate(FederateName theFederateName,
			SecurityLevelID theLevelID);

private:

  // ------------------------
  // -- Private Attributes --
  // ------------------------

  SocketServer *RTIG_SocketServer; // Never free this object.
  FederationHandle MyFederation;

  SecurityLevelID LastLevelID; // Last Level ID attributed

  FederateLevelList FedLevelList;

  // ---------------------
  // -- Private Methods --
  // ---------------------

  SecurityLevelID getLevel(const char *theFederate);

  void insertPublicLevel(void);
};

}

#endif // _CERTI_SECURITY_SERVER_HH

// $Id: SecurityServer.hh,v 3.1 2002/12/11 00:47:33 breholee Exp $

