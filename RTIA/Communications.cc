// -*- mode:C++ ; tab-width:4 ; c-basic-offset:4 ; indent-tabs-mode:nil -*-
// ----------------------------------------------------------------------------
// CERTI - HLA RunTime Infrastructure
// Copyright (C) 2002, 2003  ONERA
//
// This file is part of CERTI
//
// CERTI is free software ; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation ; either version 2 of the License, or
// (at your option) any later version.
//
// CERTI is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY ; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program ; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
// $Id: Communications.cc,v 3.6 2003/02/17 09:17:03 breholee Exp $
// ----------------------------------------------------------------------------

#include "Communications.hh"

namespace certi {
namespace rtia {

static pdCDebug D("RTIA_COMM", "(RTIA Comm) ");

// ----------------------------------------------------------------------------
/*! Wait a message coming from RTIG. Parameters are :
  1- Returned message,
  2- Message type expected,
  3- Federate which sent the message, 0 if indifferent.
*/
void Communications::waitMessage(NetworkMessage *msg,
                                 TypeNetworkMessage type_msg,
                                 FederateHandle numeroFedere)
{
    NetworkMessage *tampon ;

    assert(type_msg > 0 && type_msg < 100);

    D.Out(pdProtocol, "Waiting for Message of Type %d.", type_msg);

    // Does a new message has arrived ?
    if (searchMessage(type_msg, numeroFedere, msg) == RTI_TRUE)
        return ;

    // Otherwise, wait for a message with same type than expected and with
    // same federate number.
    tampon = new NetworkMessage ;
    tampon->read((SecureTCPSocket *)this);

    D.Out(pdProtocol, "TCP Message of Type %d has arrived.", type_msg);

    while ((tampon->type != type_msg) ||
           ((numeroFedere != 0) &&(tampon->federate != numeroFedere))) {
        waitingList.push_back(tampon);
        tampon = new NetworkMessage ;
        tampon->read((SecureTCPSocket *) this);

        D.Out(pdProtocol, "Message of Type %d has arrived.", type_msg);
    }

    // BUG: Should use copy operator.
    memcpy((void *) msg, (void *) tampon, sizeof(NetworkMessage));
    delete tampon ;

    assert(msg != NULL);
    assert(msg->type == type_msg);
}

// ----------------------------------------------------------------------------
//! Communications.
Communications::Communications(void)
    : SocketUN(), SecureTCPSocket(), SocketUDP()
{
    char nom_serveur_RTIG[200] ;

    // Federate/RTIA link creation.
    acceptUN();

    // RTIG TCP link creation.
    char *certihost = getenv("CERTI_HOST");

    ifstream* file = NULL ;
    if (certihost==NULL) {
        file = new ifstream("RTIA.dat", ios::in);
        if (!file->is_open()) {
            cout << "RTIA ERROR: Unable to find RTIG host." << endl ;
            cout << "No RTIA.dat file found, no CERTI_HOST variable set" << endl ;
            exit(-1);
        }

        file->get(nom_serveur_RTIG, 200);
        file->close();
        delete file ;
        certihost = nom_serveur_RTIG ;
    }

    const char *tcp_port = getenv("CERTI_TCP_PORT");
    const char *udp_port = getenv("CERTI_UDP_PORT");
    if (tcp_port==NULL) tcp_port = PORT_TCP_RTIG ;
    if (udp_port==NULL) udp_port = PORT_UDP_RTIG ;

    createTCPClient(atoi(tcp_port), certihost);
    createUDPClient(atoi(udp_port), certihost);
}

// ----------------------------------------------------------------------------
//! ~Communications.
Communications::~Communications(void)
{
    // Advertise RTIG that TCP link is being closed.

    NetworkMessage msg ;
    msg.type = m_CLOSE_CONNEXION ;
    msg.write((SecureTCPSocket *) this);

    SecureTCPSocket::close();
}

// ----------------------------------------------------------------------------
//! Request a service to federate.
void
Communications::requestFederateService(Message *req, Message *rep)
{
    assert(req != NULL);
    D.Out(pdRequest, "Sending Request to Federate, Type %d.", req->type);
    sendUN(req);
    receiveUN(rep);
    D.Out(pdAnswer, "Received Answer from Federate.");
    assert(req->type == rep->type);
}

// ----------------------------------------------------------------------------
unsigned long
Communications::getAddress(void)
{
    return((SocketUDP *) this)->getAddr();
}

// ----------------------------------------------------------------------------
unsigned int
Communications::getPort(void)
{
    return((SocketUDP *) this)->getPort();
}

// ----------------------------------------------------------------------------
//! read message.
/*! Reads a message either from the network or from the federate
  Returns the actual source in the 1st parameter (RTIG=>1 federate=>2)
*/
void
Communications::readMessage(int &n, NetworkMessage *msg_reseau, Message *msg)
{
    // initialize fdset for use with select.
    fd_set fdset ;
    FD_ZERO(&fdset);
    FD_SET(_socket_un, &fdset);
    FD_SET(SecureTCPSocket::returnSocket(), &fdset);
    FD_SET(SocketUDP::returnSocket(), &fdset);

#ifdef FEDERATION_USES_MULTICAST
    // if multicast link is initialized (during join federation).
    if (_est_init_mc)
        FD_SET(_socket_mc, &fdset);
#endif

    if (!waitingList.empty()) {
        // One message is in waiting buffer.
        NetworkMessage *msg2 ;
        msg2 = waitingList.front();
        waitingList.pop_front();
        memcpy(msg_reseau, msg2, TAILLE_MSG_RESEAU);
        delete msg2 ;
        n = 1 ;
    }
    else if (SecureTCPSocket::isDataReady() == RTI_TRUE) {
        // Datas are in TCP waiting buffer.
        // Read a message from RTIG TCP link.
        msg_reseau->read((SecureTCPSocket *) this);
        n = 1 ;
    }
    else if (SocketUDP::isDataReady() == RTI_TRUE) {
        // Datas are in UDP waiting buffer.
        // Read a message from RTIG UDP link.
        msg_reseau->read((SocketUDP *) this);
        n = 1 ;
    }
    else if (SocketUN::isDataReady() == RTI_TRUE) {
        // Datas are in UNIX waiting buffer.
        // Read a message from federate UNIX link.
        msg->read((SocketUN *) this);
        n = 2 ;
    }
    else {
        // waitingList is empty and no data in TCP buffer.
        // Wait a message (coming from federate or network).
        if (select(ulimit(4, 0), &fdset, NULL, NULL, NULL) < 0) {
            if (errno == EINTR)
                throw NetworkSignal();
            else
                throw NetworkError();
        }

        // At least one message has been received, read this message.

#ifdef FEDERATION_USES_MULTICAST
        // Priorite aux messages venant du multicast(pour essayer d'eviter
        // un depassement de la file et donc la perte de messages)

        if (_est_init_mc && FD_ISSET(_socket_mc, &fdset)) {
            // Read a message coming from the multicast link.
            receiveMC(msg_reseau);
            n = 1 ;
        }
#endif

        if (FD_ISSET(SecureTCPSocket::returnSocket(), &fdset)) {
            // Read a message coming from the TCP link with RTIG.
            msg_reseau->read((SecureTCPSocket *) this);
            n = 1 ;
        }
        else if (FD_ISSET(SocketUDP::returnSocket(), &fdset)) {
            // Read a message coming from the UDP link with RTIG.
            msg_reseau->read((SocketUDP *) this);
            n = 1 ;
        }
        else {
            // Read a message coming from the federate.
            assert(FD_ISSET(_socket_un, &fdset));
            receiveUN(msg);
            n = 2 ;
        }
    }
}

// ----------------------------------------------------------------------------
/*! Returns RTI_TRUE if a 'type_msg' message coming from federate
  'numeroFedere' (or any other federate if numeroFedere == 0) was in
  the queue and was copied in 'msg'. If no such message is found,
  returns RTI_FALSE.
*/
Boolean
Communications::searchMessage(TypeNetworkMessage type_msg,
                              FederateHandle numeroFedere,
                              NetworkMessage *msg)
{
    list<NetworkMessage *>::iterator i ;
    for (i = waitingList.begin(); i != waitingList.end(); i++) {

        D.Out(pdProtocol, "Rechercher message de type %d .", type_msg);

        if ((*i)->type == type_msg) {
            // if numeroFedere != 0, verify that federateNumbers are similar
            if (((*i)->federate == numeroFedere) || (numeroFedere == 0)) {
                memcpy(msg, (*i), TAILLE_MSG_RESEAU);
                waitingList.erase(i);
                delete (*i);
                D.Out(pdProtocol,
                      "Message of Type %d was already here.",
                      type_msg);
                return RTI_TRUE ;
            }
        }
    }
    return RTI_FALSE ;
}

// ----------------------------------------------------------------------------
void
Communications::sendMessage(NetworkMessage *Msg)
{
    Msg->write((SecureTCPSocket *) this);
}

// ----------------------------------------------------------------------------
void
Communications::sendUN(Message *Msg)
{
    Msg->write((SocketUN *) this);
}

// ----------------------------------------------------------------------------
void
Communications::receiveUN(Message *Msg)
{
    Msg->read((SocketUN *) this);
}

}} // namespace certi/rtia

// $Id: Communications.cc,v 3.6 2003/02/17 09:17:03 breholee Exp $
