// ----------------------------------------------------------------------------
// CERTI - HLA RunTime Infrastructure
// Copyright (C) 2002-2005  ONERA
//
// This file is part of CERTI-libRTI
//
// CERTI-libRTI is free software ; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation ; either version 2 of
// the License, or (at your option) any later version.
//
// CERTI-libRTI is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY ; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program ; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
//
// $Id: RTIambassador.cc,v 3.37.2.1 2007/02/02 14:19:35 rousse Exp $
// ----------------------------------------------------------------------------

#include <config.h>
#include "certi.hh"

#include "RTIambPrivateRefs.hh"

#include "FedRegion.hh"
#include "Message.hh"
#include "PrettyDebug.hh"

#include <signal.h>
#include <iostream>
#include <unistd.h>
#include <cassert>

using std::cout ;
using std::cerr ;
using std::endl ;

namespace {

static pdCDebug D("LIBRTI", __FILE__);

using namespace certi ;

std::vector<RTI::Handle>
build_region_handles(RTI::Region **regions, int nb)
    throw (RTI::RegionNotKnown)
{
    std::vector<RTI::Handle> vect(nb);
    for (int i = 0 ; i < nb ; ++i) {
	RTI::Region *region = regions[i] ;
	try {
	    vect[i] = dynamic_cast<FedRegion *>(region)->getHandle();
	}
	catch (std::bad_cast) {
	    throw RTI::RegionNotKnown("");
	}
    }
    return vect ;
}

RTI::Handle
get_handle(const RTI::Region &region)
    throw (RegionNotKnown, RTIinternalError)
{
    try {
	return dynamic_cast<const FedRegion &>(region).getHandle();
    }
    catch (std::bad_cast) {
	throw RTI::RegionNotKnown("");
    }
    throw RTI::RTIinternalError("");
}

} // anonymous namesapce

// ----------------------------------------------------------------------------
//! Start RTIambassador processes for communication with RTIG.
/*! When a new RTIambassador is created in the application, a new process rtia
  is launched. This process is used for data exchange with rtig server.
  This process connects to rtia after one second delay (UNIX socket).
*/
RTI::RTIambassador::RTIambassador()
    throw (MemoryExhausted, RTIinternalError)
{
    PrettyDebug::setFederateName( "Federate-process" );

    privateRefs = new RTIambPrivateRefs();

    privateRefs->socketUn = new SocketUN(stIgnoreSignal);
	
    privateRefs->is_reentrant = false ;
    const char *rtiaexec = "rtia" ;
    const char *rtiaenv = getenv("CERTI_RTIA");
    const char *rtiacall ;
    if (rtiaenv) rtiacall = rtiaenv ;
    else rtiacall = rtiaexec ;

    // creating RTIA process.
    switch((privateRefs->pid_RTIA = fork())) {
      case -1: // fork failed.
        perror("fork");
        throw RTIinternalError("fork failed in RTIambassador constructor");
        break ;

      case 0: // child process (RTIA).
        execlp(rtiacall, NULL);
        perror("execlp");

        cerr << "Could not launch RTIA process." << endl
             << "Maybe RTIA is not in search PATH environment." << endl ;

        exit(-1);

      default: // father process (Federe).
        sleep(1);

        if( privateRefs->socketUn->connectUN(privateRefs->pid_RTIA) )
        {
           D.Out( pdError, "Cannot connect to RTIA. Abort." ) ;
           kill( privateRefs->pid_RTIA, SIGINT ) ;
           throw RTIinternalError( "Cannot connect to RTIA" ) ;
        };
        break ;
    }
}

// ----------------------------------------------------------------------------
//! Closes processes.
/*! When destructor is called, kill rtia process.
 */
RTI::RTIambassador::~RTIambassador()
    throw (RTIinternalError)
{
    kill(privateRefs->pid_RTIA, SIGINT);
    delete privateRefs ;
}

// ----------------------------------------------------------------------------
// Tick
RTI::Boolean
RTI::RTIambassador::tick()
    throw (SpecifiedSaveLabelDoesNotExist,
           ConcurrentAccessAttempted,
           RTIinternalError)
{
    Message vers_RTI, vers_Fed ;

    // Throw exception if reentrant call.
    if (privateRefs->is_reentrant)
        throw ConcurrentAccessAttempted("");

    privateRefs->is_reentrant = true ;

    // Prevenir le RTI
    vers_RTI.type = Message::TICK_REQUEST ;

    try {
        vers_RTI.write(privateRefs->socketUn);
    }
    catch (NetworkError) {
        cout << "tick 1." << endl ;
        cout << "LibRTI: Catched NetworkError, throw RTIinternalError."
             << endl ;
        throw RTIinternalError("");
    }

    for (;;) {

        // Lire la reponse du RTIA local
        try {
            vers_Fed.read(privateRefs->socketUn);
        }
        catch (NetworkError) {
            cout << "tick 2." << endl ;
            cout << "LibRTI: Catched NetworkError, throw RTIinternalError."
                 << endl ;
            throw RTIinternalError("");
        }

        // Si c'est de type TICK_REQUEST, il n'y a qu'a traiter l'exception.
        if (vers_Fed.type == Message::TICK_REQUEST) {
            privateRefs->is_reentrant = false ;
            privateRefs->processException(&vers_Fed);
            return RTI::Boolean(vers_Fed.getBoolean());
        }

        // Sinon, le RTI nous demande un service, donc on appele une methode
        // du FederateAmbassador.

        vers_RTI.setException(e_NO_EXCEPTION);
        try {
            switch (vers_Fed.type) {

              case Message::SYNCHRONIZATION_POINT_REGISTRATION_SUCCEEDED:
                privateRefs->fed_amb->synchronizationPointRegistrationSucceeded(
		    vers_Fed.getLabel());
                break ;

              case Message::ANNOUNCE_SYNCHRONIZATION_POINT:
                privateRefs->fed_amb->announceSynchronizationPoint(vers_Fed.getLabel(),
                                                      vers_Fed.getTag());
                break ;

              case Message::FEDERATION_SYNCHRONIZED:
                privateRefs->fed_amb->federationSynchronized(vers_Fed.getLabel());
                break ;

              case Message::INITIATE_FEDERATE_SAVE:
                privateRefs->fed_amb->initiateFederateSave(vers_Fed.getLabel());
                break ;

              case Message::FEDERATION_SAVED:
                privateRefs->fed_amb->federationSaved();
                break ;

              case Message::REQUEST_FEDERATION_RESTORE_SUCCEEDED:
                privateRefs->fed_amb->requestFederationRestoreSucceeded(
                    vers_Fed.getLabel());
                break ;

              case Message::REQUEST_FEDERATION_RESTORE_FAILED:
                privateRefs->fed_amb->requestFederationRestoreFailed(vers_Fed.getLabel(),
                                                        vers_Fed.getTag());
                break ;

              case Message::FEDERATION_RESTORE_BEGUN:
                privateRefs->fed_amb->federationRestoreBegun();
                break ;

              case Message::INITIATE_FEDERATE_RESTORE:
                privateRefs->fed_amb->initiateFederateRestore(vers_Fed.getLabel(),
                                                 vers_Fed.getFederate());
                break ;

              case Message::FEDERATION_RESTORED:
                privateRefs->fed_amb->federationRestored();
                break ;

              case Message::FEDERATION_NOT_RESTORED:
                privateRefs->fed_amb->federationNotRestored();
                break ;

              case Message::START_REGISTRATION_FOR_OBJECT_CLASS: {
                  privateRefs->fed_amb->startRegistrationForObjectClass(
		      vers_Fed.getObjectClass());
              } break ;

              case Message::STOP_REGISTRATION_FOR_OBJECT_CLASS: {
                  privateRefs->fed_amb->
                      stopRegistrationForObjectClass(vers_Fed.getObjectClass());
              } break ;

              case Message::TURN_INTERACTIONS_ON: {
                  privateRefs->fed_amb->turnInteractionsOn(vers_Fed.getInteractionClass());
              } break ;

              case Message::TURN_INTERACTIONS_OFF: {
                  privateRefs->fed_amb->turnInteractionsOff(vers_Fed.getInteractionClass());
              } break ;

              case Message::DISCOVER_OBJECT_INSTANCE: {
                  privateRefs->fed_amb->
                      discoverObjectInstance(vers_Fed.getObject(),
                                             vers_Fed.getObjectClass(),
                                             vers_Fed.getName());
              } break ;

              case Message::REFLECT_ATTRIBUTE_VALUES: {
                  AttributeHandleValuePairSet *attributes = vers_Fed.getAHVPS();
                  privateRefs->fed_amb->
                      reflectAttributeValues(vers_Fed.getObject(),
                                             *attributes,
                                             vers_Fed.getFedTime(),
                                             vers_Fed.getTag(),
                                             vers_Fed.getEventRetraction());

                  delete attributes ;
              } break ;

              case Message::RECEIVE_INTERACTION: {
                  ParameterHandleValuePairSet *parameters = vers_Fed.getPHVPS();

                  privateRefs->fed_amb->receiveInteraction(vers_Fed.getInteractionClass(),
                                              *parameters,
                                              vers_Fed.getFedTime(),
                                              vers_Fed.getTag(),
                                              vers_Fed.getEventRetraction());

                  delete parameters ;
              } break ;

              case Message::REMOVE_OBJECT_INSTANCE: {
                  privateRefs->fed_amb->removeObjectInstance(vers_Fed.getObject(),
                                                vers_Fed.getFedTime(),
                                                vers_Fed.getTag(),
                                                vers_Fed.getEventRetraction());
              } break ;

              case Message::PROVIDE_ATTRIBUTE_VALUE_UPDATE: {



                  // privateRefs->fed_amb->provideAttributeValueUpdate();
              } break ;

              case Message::REQUEST_RETRACTION: {

              } break ;

              case Message::REQUEST_ATTRIBUTE_OWNERSHIP_ASSUMPTION: {
                  AttributeHandleSet *attributeSet = vers_Fed.getAHS();

                  privateRefs->fed_amb->
                      requestAttributeOwnershipAssumption(vers_Fed.getObject(),
                                                          *attributeSet,
                                                          vers_Fed.getTag());
                  delete attributeSet ;
              } break ;

              case Message::REQUEST_ATTRIBUTE_OWNERSHIP_RELEASE: {
                  AttributeHandleSet *attributeSet = vers_Fed.getAHS();

                  privateRefs->fed_amb->requestAttributeOwnershipRelease(
		      vers_Fed.getObject(),
		      *attributeSet,
		      vers_Fed.getTag());

                  delete attributeSet ;
              } break ;

              case Message::ATTRIBUTE_OWNERSHIP_UNAVAILABLE: {
                  AttributeHandleSet *attributeSet = vers_Fed.getAHS();

                  privateRefs->fed_amb->attributeOwnershipUnavailable(vers_Fed.getObject(),
                                                         *attributeSet);

                  delete attributeSet ;
              } break ;

              case Message::ATTRIBUTE_OWNERSHIP_ACQUISITION_NOTIFICATION: {
                  AttributeHandleSet *attributeSet = vers_Fed.getAHS();

                  privateRefs->fed_amb->attributeOwnershipAcquisitionNotification(
		      vers_Fed.getObject(),
		      *attributeSet);

                  delete attributeSet ;
              } break ;

              case Message::ATTRIBUTE_OWNERSHIP_DIVESTITURE_NOTIFICATION: {
                  AttributeHandleSet *attributeSet = vers_Fed.getAHS();

                  privateRefs->fed_amb->attributeOwnershipDivestitureNotification(
		      vers_Fed.getObject(),
		      *attributeSet);

                  delete attributeSet ;
              } break ;

              case Message::CONFIRM_ATTRIBUTE_OWNERSHIP_ACQUISITION_CANCELLATION: {
                  AttributeHandleSet *attributeSet = vers_Fed.getAHS();

                  privateRefs->fed_amb->
                      confirmAttributeOwnershipAcquisitionCancellation
                      (vers_Fed.getObject(), *attributeSet);

                  delete attributeSet ;
              } break ;

              case Message::INFORM_ATTRIBUTE_OWNERSHIP: {
                  privateRefs->fed_amb->
                      informAttributeOwnership(vers_Fed.getObject(),
                                               vers_Fed.getAttribute(),
                                               vers_Fed.getFederate());
              } break ;

              case Message::ATTRIBUTE_IS_NOT_OWNED: {
                  privateRefs->fed_amb->attributeIsNotOwned(vers_Fed.getObject(),
                                               vers_Fed.getAttribute());
              } break ;

              case Message::TIME_ADVANCE_GRANT: {
                  privateRefs->fed_amb->timeAdvanceGrant(vers_Fed.getFedTime());
              } break ;

              default: {
                  privateRefs->leave("RTI service requested by RTI is unknown.");
              }
            }
        }
        catch (InvalidFederationTime &e) {
            vers_RTI.setException(e_InvalidFederationTime, e._reason);
            throw ;
        }
        catch (TimeAdvanceWasNotInProgress &e) {
            vers_RTI.setException(e_TimeAdvanceWasNotInProgress, e._reason);
            throw ;
        }
        catch (FederationTimeAlreadyPassed &e) {
            vers_RTI.setException(e_FederationTimeAlreadyPassed, e._reason);
        }
        catch (FederateInternalError &e) {
            vers_RTI.setException(e_FederateInternalError, e._reason);
            throw ;
        }
        catch (Exception &e) {
            vers_RTI.setException(e_RTIinternalError, e._reason);
            throw ;
        }

        // retourner au RTI la reponse du service demande
        vers_RTI.type = vers_Fed.type ;

        try {
            vers_RTI.write(privateRefs->socketUn);
        }
        catch (NetworkError) {
            cout << "tick 3." << endl ;
            cout << "LibRTI: Catched NetworkError, throw RTIinternalError."
                 << endl ;
            throw RTIinternalError("");
        }
    }
}

// ----------------------------------------------------------------------------
RTI::Boolean
RTI::RTIambassador::tick(TickTime, TickTime)
    throw (SpecifiedSaveLabelDoesNotExist, ConcurrentAccessAttempted,
           RTIinternalError)
{
    return tick();
}


// ----------------------------------------------------------------------------
//! Create Federation Execution.
/*! Send a CREATE_FEDERATION_EXECUTION request type to inform rtia process a
  new federation is being created.
*/
void
RTI::RTI::RTIambassador::createFederationExecution(const char *executionName,
                                         const char */* FED */)
    throw (RTI::RTIinternalError, RTI::ConcurrentAccessAttempted, 
	   RTI::ErrorReadingFED, RTI::CouldNotOpenFED, 
	   RTI::FederationExecutionAlreadyExists)
{
    Message req, rep ;

    req.type = Message::CREATE_FEDERATION_EXECUTION ;
    req.setFederationName(executionName);

    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
//! Destroy Federation Execution.
void
RTI::RTIambassador::destroyFederationExecution(const char *executionName)
    throw (RTI::RTIinternalError, RTI::ConcurrentAccessAttempted,
	   RTI::FederationExecutionDoesNotExist, RTI::FederatesCurrentlyJoined)
{
    Message req, rep ;

    req.type = Message::DESTROY_FEDERATION_EXECUTION ;
    req.setFederationName(executionName);

    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
//! Join Federation Execution.
FederateHandle
RTI::RTIambassador::joinFederationExecution(const char *yourName,
				       const char *executionName,
				       FederateAmbassadorPtr fedamb)
    throw (RTI::RTIinternalError, RTI::RestoreInProgress, RTI::SaveInProgress,
	   RTI::ConcurrentAccessAttempted, RTI::ErrorReadingFED, RTI::CouldNotOpenFED, 
	   RTI::FederationExecutionDoesNotExist, RTI::FederateAlreadyExecutionMember)
{
    Message req, rep ;

    privateRefs->fed_amb = (FederateAmbassador *) fedamb ;

    req.type = Message::JOIN_FEDERATION_EXECUTION ;
    req.setFederateName(yourName);
    req.setFederationName(executionName);

    privateRefs->executeService(&req, &rep);
    return rep.getFederate();
}

// ----------------------------------------------------------------------------
//! Resign Federation Execution.
void
RTI::RTIambassador::resignFederationExecution(ResignAction theAction)
    throw (FederateOwnsAttributes,
           FederateNotExecutionMember,
           InvalidResignAction,
           ConcurrentAccessAttempted,
           RTIinternalError)
{
    Message req, rep ;

    req.type = Message::RESIGN_FEDERATION_EXECUTION ;
    req.setResignAction(theAction);
    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
//! Register Federation Synchronization Point
void
RTI::RTIambassador::registerFederationSynchronizationPoint(const char *label,
                                                      const char *the_tag)
    throw (FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
{
    Message req, rep ;

    req.type = Message::REGISTER_FEDERATION_SYNCHRONIZATION_POINT ;
    req.setLabel(label);
    req.setTag(the_tag);

    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
void
RTI::RTIambassador::registerFederationSynchronizationPoint(const char *label,
						      const char *theTag,
						      const FederateHandleSet &)
    throw (RTI::RTIinternalError, RTI::RestoreInProgress, RTI::SaveInProgress,
	   RTI::ConcurrentAccessAttempted, RTI::FederateNotExecutionMember)
{
    throw UnimplementedService("");
    Message req, rep ;

    req.type = Message::REGISTER_FEDERATION_SYNCHRONIZATION_POINT ;
    req.setLabel(label);
    req.setTag(theTag);

    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
//! Synchronization Point Achieved
void
RTI::RTIambassador::synchronizationPointAchieved(const char *label)
    throw (SynchronizationPointLabelWasNotAnnounced,
           FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
{
    Message req, rep ;

    req.type = Message::SYNCHRONIZATION_POINT_ACHIEVED ;
    req.setLabel(label);

    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
//! Request Federation Save.
void
RTI::RTIambassador::requestFederationSave(const char *label,
                                     const FedTime& theTime)
    throw (FederationTimeAlreadyPassed,
           InvalidFederationTime,
           FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
{
    throw UnimplementedService("");
    Message req, rep ;

    req.type = Message::REQUEST_FEDERATION_SAVE ;
    req.setFedTime(theTime);
    req.setLabel(label);
    req.setBoolean(true);

    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
//! Request Federation Save.
void
RTI::RTIambassador::requestFederationSave(const char *label)
    throw (FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
{
    throw UnimplementedService("");
    Message req, rep ;

    req.type = Message::REQUEST_FEDERATION_SAVE ;
    req.setLabel(label);
    req.setBoolean(false);

    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
//! Federate Save Begun.
void
RTI::RTIambassador::federateSaveBegun()
    throw (SaveNotInitiated,
           FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           RestoreInProgress,
           RTIinternalError)
{
    throw UnimplementedService("");
    Message req, rep ;

    req.type = Message::FEDERATE_SAVE_BEGUN ;

    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
//! Federate Save Complete.
void
RTI::RTIambassador::federateSaveComplete()
    throw (SaveNotInitiated,
           FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           RestoreInProgress,
           RTIinternalError)
{
    throw UnimplementedService("");
    Message req, rep ;

    req.type = Message::FEDERATE_SAVE_COMPLETE ;

    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
// Federate Save Not Complete.
void
RTI::RTIambassador::federateSaveNotComplete()
    throw (SaveNotInitiated,
           FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           RestoreInProgress,
           RTIinternalError)
{
    throw UnimplementedService("");
    Message req, rep ;

    req.type = Message::FEDERATE_SAVE_NOT_COMPLETE ;

    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
//! Request Restore.
void
RTI::RTIambassador::requestFederationRestore(const char *label)
    throw (RTI::RTIinternalError, RTI::RestoreInProgress, RTI::SaveInProgress, 
	   RTI::ConcurrentAccessAttempted, RTI::FederateNotExecutionMember)
{
    throw UnimplementedService("");
    Message req, rep ;

    req.type = Message::REQUEST_FEDERATION_RESTORE ;
    req.setLabel(label);

    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
//! Restore Complete.
void
RTI::RTIambassador::federateRestoreComplete()
    throw (RTI::RTIinternalError, RTI::SaveInProgress,
	   RTI::ConcurrentAccessAttempted, RTI::FederateNotExecutionMember,
	   RTI::RestoreNotRequested)
{
    throw UnimplementedService("");
    Message req, rep ;

    req.type = Message::FEDERATE_RESTORE_COMPLETE ;

    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
//! Federate Restore Not Complete.
void
RTI::RTIambassador::federateRestoreNotComplete()
    throw (RTI::RTIinternalError, RTI::SaveInProgress,
	   RTI::ConcurrentAccessAttempted, RTI::FederateNotExecutionMember,
	   RTI::RestoreNotRequested)
{
    throw UnimplementedService("");
    Message req, rep ;

    req.type = Message::FEDERATE_RESTORE_NOT_COMPLETE ;

    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
// Publish Object Class
void
RTI::RTIambassador::publishObjectClass(ObjectClassHandle theClass,
                                  const AttributeHandleSet& attributeList)
    throw (RTI::RTIinternalError, 
	   RTI::RestoreInProgress, RTI::SaveInProgress, RTI::ConcurrentAccessAttempted, 
	   RTI::FederateNotExecutionMember, RTI::OwnershipAcquisitionPending, 
	   RTI::AttributeNotDefined, RTI::ObjectClassNotDefined)
{
    Message req, rep ;

    req.type = Message::PUBLISH_OBJECT_CLASS ;
    req.setObjectClass(theClass);
    req.setAHS(attributeList);

    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
// UnPublish Object Class
void
RTI::RTIambassador::unpublishObjectClass(ObjectClassHandle theClass)
    throw (RTI::RTIinternalError, RTI::RestoreInProgress, RTI::SaveInProgress, 
	   RTI::ConcurrentAccessAttempted, RTI::FederateNotExecutionMember, 
	   RTI::OwnershipAcquisitionPending, RTI::ObjectClassNotPublished, 
	   RTI::ObjectClassNotDefined)
{
    Message req, rep ;

    req.type = Message::UNPUBLISH_OBJECT_CLASS ;
    req.setObjectClass(theClass);
    privateRefs->executeService(&req, &rep);
}


// ----------------------------------------------------------------------------
// Publish Interaction Class
void
RTI::RTIambassador::publishInteractionClass(InteractionClassHandle theInteraction)
    throw (RTI::RTIinternalError, RTI::RestoreInProgress, RTI::SaveInProgress, 
	   RTI::ConcurrentAccessAttempted, RTI::FederateNotExecutionMember, 
	   RTI::InteractionClassNotDefined)
{
    Message req, rep ;

    req.type = Message::PUBLISH_INTERACTION_CLASS ;
    req.setInteractionClass(theInteraction);
    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
// Publish Interaction Class
void
RTI::RTIambassador::unpublishInteractionClass(InteractionClassHandle theInteraction)
    throw (RTI::RTIinternalError, RTI::RestoreInProgress, RTI::SaveInProgress, 
	   RTI::ConcurrentAccessAttempted, RTI::FederateNotExecutionMember, 
	   RTI::InteractionClassNotPublished, RTI::InteractionClassNotDefined)
{
    Message req, rep ;

    req.type = Message::UNPUBLISH_INTERACTION_CLASS ;
    req.setInteractionClass(theInteraction);
    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
// Subscribe Object Class Attributes
void
RTI::RTIambassador::
subscribeObjectClassAttributes(ObjectClassHandle theClass,
                               const AttributeHandleSet& attributeList,
                               RTI::Boolean active)
    throw (RTI::RTIinternalError, 
	   RTI::RestoreInProgress, RTI::SaveInProgress, RTI::ConcurrentAccessAttempted, 
	   RTI::FederateNotExecutionMember, RTI::AttributeNotDefined, 
	   RTI::ObjectClassNotDefined)
{
    Message req, rep ;

    req.type = Message::SUBSCRIBE_OBJECT_CLASS_ATTRIBUTES ;
    req.setObjectClass(theClass);
    req.setAHS(attributeList);
    req.setBoolean(active);

    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
// UnSubscribe Object Class Attribute
void
RTI::RTIambassador::unsubscribeObjectClass(ObjectClassHandle theClass)
    throw (RTI::RTIinternalError, RTI::RestoreInProgress, RTI::SaveInProgress, 
	   RTI::ConcurrentAccessAttempted, RTI::FederateNotExecutionMember, 
	   RTI::ObjectClassNotSubscribed, RTI::ObjectClassNotDefined)
{
    Message req, rep ;

    req.type = Message::UNSUBSCRIBE_OBJECT_CLASS ;
    req.setObjectClass(theClass);
    privateRefs->executeService(&req, &rep);
}


// ----------------------------------------------------------------------------
// Subscribe Interaction Class
void
RTI::RTIambassador::subscribeInteractionClass(InteractionClassHandle theClass,
                                         RTI::Boolean /*active*/)
    throw (RTI::RTIinternalError, RTI::RestoreInProgress, 
	   RTI::SaveInProgress, RTI::FederateLoggingServiceCalls, 
	   RTI::ConcurrentAccessAttempted, RTI::FederateNotExecutionMember, 
	   RTI::InteractionClassNotDefined)
{
    Message req, rep ;

    req.type = Message::SUBSCRIBE_INTERACTION_CLASS ;
    req.setInteractionClass(theClass);
    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
// UnSubscribe Interaction Class
void
RTI::RTIambassador::unsubscribeInteractionClass(InteractionClassHandle theClass)
    throw (RTI::RTIinternalError, RTI::RestoreInProgress, RTI::SaveInProgress, 
	   RTI::ConcurrentAccessAttempted, RTI::FederateNotExecutionMember, 
	   RTI::InteractionClassNotSubscribed, RTI::InteractionClassNotDefined)
{
    Message req, rep ;

    req.type = Message::UNSUBSCRIBE_INTERACTION_CLASS ;
    req.setInteractionClass(theClass);
    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
// Register Object
ObjectHandle
RTI::RTIambassador::registerObjectInstance(ObjectClassHandle theClass,
                                      const char *theObjectName)
    throw (RTI::RTIinternalError, RTI::RestoreInProgress, 
	   RTI::SaveInProgress, RTI::ConcurrentAccessAttempted, 
	   RTI::FederateNotExecutionMember, RTI::ObjectAlreadyRegistered, 
	   RTI::ObjectClassNotPublished, RTI::ObjectClassNotDefined)
{
    Message req, rep ;

    req.type = Message::REGISTER_OBJECT_INSTANCE ;
    req.setName(theObjectName);
    req.setObjectClass(theClass);
    privateRefs->executeService(&req, &rep);

    return rep.getObject();
}

// ----------------------------------------------------------------------------
ObjectHandle
RTI::RTIambassador::registerObjectInstance(ObjectClassHandle theClass)
    throw (RTI::RTIinternalError, RTI::RestoreInProgress, RTI::SaveInProgress, 
	   RTI::ConcurrentAccessAttempted, RTI::FederateNotExecutionMember, 
	   RTI::ObjectClassNotPublished, RTI::ObjectClassNotDefined)
{
    Message req, rep ;

    req.type = Message::REGISTER_OBJECT_INSTANCE ;
    req.setName("\0");
    req.setObjectClass(theClass);

    privateRefs->executeService(&req, &rep);

    return rep.getObject();
}

// ----------------------------------------------------------------------------
// Update Attribute Values
EventRetractionHandle
RTI::RTIambassador::
updateAttributeValues(ObjectHandle theObject,
                      const AttributeHandleValuePairSet& theAttributes,
                      const FedTime& theTime,
                      const char *theTag)
    throw (ObjectNotKnown,
           AttributeNotDefined,
           AttributeNotOwned,
           InvalidFederationTime,
           FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
{
    Message req, rep ;

    req.type = Message::UPDATE_ATTRIBUTE_VALUES ;
    req.setObject(theObject);
    req.setFedTime(theTime);
    req.setTag(theTag);
    req.setAHVPS(theAttributes);
    req.setBoolean(true);

    privateRefs->executeService(&req, &rep);

    return rep.getEventRetraction();
}

// ----------------------------------------------------------------------------
void
RTI::RTIambassador::updateAttributeValues(ObjectHandle the_object,
                                     const AttributeHandleValuePairSet& attrs,
                                     const char *the_tag)
    throw (RTI::RTIinternalError, RTI::RestoreInProgress, RTI::SaveInProgress, 
	   RTI::ConcurrentAccessAttempted, RTI::FederateNotExecutionMember, 
	   RTI::AttributeNotOwned, RTI::AttributeNotDefined, RTI::ObjectNotKnown)
{
    throw UnimplementedService("");

    Message req, rep ;

    req.type = Message::UPDATE_ATTRIBUTE_VALUES ;
    req.setObject(the_object);
    req.setTag(the_tag);
    req.setAHVPS(attrs);
    req.setBoolean(false);

    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
// Send Interaction
EventRetractionHandle
RTI::RTIambassador::sendInteraction(InteractionClassHandle theInteraction,
                               const ParameterHandleValuePairSet& theParameters,
                               const FedTime& theTime,
                               const char *theTag)
    throw (InteractionClassNotDefined,
           InteractionClassNotPublished,
           InteractionParameterNotDefined,
           InvalidFederationTime,
           FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
{
    Message req, rep ;

    req.type = Message::SEND_INTERACTION ;
    req.setInteractionClass(theInteraction);
    req.setFedTime(theTime);
    req.setTag(theTag);
    req.setPHVPS(theParameters);
    req.setRegion(0);
    req.setBoolean(true);
    
    privateRefs->executeService(&req, &rep);

    return rep.getEventRetraction();
}

// ----------------------------------------------------------------------------
void
RTI::RTIambassador::sendInteraction(InteractionClassHandle the_interaction,
                               const ParameterHandleValuePairSet &parameters,
                               const char *the_tag)
    throw (RTI::RTIinternalError, RTI::RestoreInProgress, RTI::SaveInProgress, 
	   RTI::ConcurrentAccessAttempted, RTI::FederateNotExecutionMember, 
	   RTI::InteractionParameterNotDefined, RTI::InteractionClassNotPublished, 
	   RTI::InteractionClassNotDefined)
{
    throw UnimplementedService("");

    Message req, rep ;

    req.type = Message::SEND_INTERACTION ;
    req.setInteractionClass(the_interaction);
    req.setTag(the_tag);
    req.setPHVPS(parameters);
    req.setRegion(0);
    req.setBoolean(false);

    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
// Delete Object
EventRetractionHandle
RTI::RTIambassador::deleteObjectInstance(ObjectHandle theObject,
                                    const FedTime& theTime,
                                    const char *theTag)
    throw (ObjectNotKnown,
           DeletePrivilegeNotHeld,
           InvalidFederationTime,
           FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
{
    Message req, rep ;

    req.type = Message::DELETE_OBJECT_INSTANCE ;
    req.setObject(theObject);
    req.setFedTime(theTime);
    req.setTag(theTag);

    privateRefs->executeService(&req, &rep);
    return rep.getEventRetraction();
}

// ----------------------------------------------------------------------------
void
RTI::RTIambassador::deleteObjectInstance(ObjectHandle theObject,
                                    const char *theTag)
    throw (RTI::RTIinternalError, RTI::RestoreInProgress, RTI::SaveInProgress, 
	   RTI::ConcurrentAccessAttempted, RTI::FederateNotExecutionMember, 
	   RTI::DeletePrivilegeNotHeld, RTI::ObjectNotKnown)
{
    throw UnimplementedService("");
    Message req, rep ;

    req.type = Message::DELETE_OBJECT_INSTANCE ;
    req.setObject(theObject);
    req.setTag(theTag);

    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
// Local Delete Object Instance
void
RTI::RTIambassador::localDeleteObjectInstance(ObjectHandle theObject)
    throw (RTI::RTIinternalError, RTI::RestoreInProgress, RTI::SaveInProgress, 
	   RTI::ConcurrentAccessAttempted, RTI::FederateNotExecutionMember, 
	   RTI::FederateOwnsAttributes, RTI::ObjectNotKnown)
{
    throw UnimplementedService("");
    Message req, rep ;

    req.type = Message::LOCAL_DELETE_OBJECT_INSTANCE ;
    req.setObject(theObject);

    privateRefs->executeService(&req, &rep);
}


// ----------------------------------------------------------------------------
// Change Attribute Transportation Type
void
RTI::RTIambassador::
changeAttributeTransportationType(ObjectHandle theObject,
                                  const AttributeHandleSet& theAttributes,
                                  TransportationHandle theType)
    throw (RTI::RTIinternalError, RTI::RestoreInProgress, RTI::SaveInProgress, 
	   RTI::ConcurrentAccessAttempted, RTI::FederateNotExecutionMember, 
	   RTI::InvalidTransportationHandle, RTI::AttributeNotOwned, 
	   RTI::AttributeNotDefined, RTI::ObjectNotKnown)
{
    Message req, rep ;

    req.type = Message::CHANGE_ATTRIBUTE_TRANSPORTATION_TYPE ;
    req.setObject(theObject);
    req.setTransportation(theType);
    req.setAHS(theAttributes);

    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
// Change Interaction Transportation Type
void
RTI::RTIambassador::
changeInteractionTransportationType(InteractionClassHandle theClass,
                                    TransportationHandle theType)
    throw (RTI::RTIinternalError, RTI::RestoreInProgress, 
	   RTI::SaveInProgress, RTI::ConcurrentAccessAttempted, 
	   RTI::FederateNotExecutionMember, RTI::InvalidTransportationHandle, 
	   RTI::InteractionClassNotPublished, RTI::InteractionClassNotDefined)
{
    Message req, rep ;

    req.type = Message::CHANGE_INTERACTION_TRANSPORTATION_TYPE ;
    req.setInteractionClass(theClass);
    req.setTransportation(theType);

    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
// Request Attribute Value Update
void
RTI::RTIambassador::requestObjectAttributeValueUpdate(ObjectHandle theObject,
                                                 const AttributeHandleSet &ahs)
    throw (RTI::RTIinternalError, 
	   RTI::RestoreInProgress, RTI::SaveInProgress, RTI::ConcurrentAccessAttempted, 
	   RTI::FederateNotExecutionMember, RTI::AttributeNotDefined, 
	   RTI::ObjectNotKnown)
{
    throw UnimplementedService("");
    Message req, rep ;

    req.type = Message::REQUEST_OBJECT_ATTRIBUTE_VALUE_UPDATE ;
    req.setObject(theObject);
    req.setAHS(ahs);

    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
void
RTI::RTIambassador::requestClassAttributeValueUpdate(ObjectClassHandle theClass,
                                                const AttributeHandleSet &attrs)
    throw (RTI::RTIinternalError, 
	   RTI::RestoreInProgress, RTI::SaveInProgress, RTI::ConcurrentAccessAttempted, 
	   RTI::FederateNotExecutionMember, RTI::AttributeNotDefined, 
	   RTI::ObjectClassNotDefined)
{
    throw UnimplementedService("");
    Message req, rep ;

    req.type = Message::REQUEST_CLASS_ATTRIBUTE_VALUE_UPDATE ;
    req.setObjectClass(theClass);
    req.setAHS(attrs);

    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
// UnConditional Attribute Ownership Divestiture
void
RTI::RTIambassador::
unconditionalAttributeOwnershipDivestiture(ObjectHandle theObject,
                                           const AttributeHandleSet &attrs)
    throw (ObjectNotKnown,
           AttributeNotDefined,
           AttributeNotOwned,
           FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
{
    Message req, rep ;

    req.type = Message::UNCONDITIONAL_ATTRIBUTE_OWNERSHIP_DIVESTITURE ;
    req.setObject(theObject);
    req.setAHS(attrs);

    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
// Negotiated Attribute Ownership Divestiture
void
RTI::RTIambassador::
negotiatedAttributeOwnershipDivestiture(ObjectHandle theObject,
                                        const AttributeHandleSet& attrs,
                                        const char *theTag)
    throw (ObjectNotKnown,
           AttributeNotDefined,
           AttributeNotOwned,
           AttributeAlreadyBeingDivested,
           FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
{
    Message req, rep ;

    req.type = Message::NEGOTIATED_ATTRIBUTE_OWNERSHIP_DIVESTITURE ;
    req.setObject(theObject);
    req.setTag(theTag);
    req.setAHS(attrs);

    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
// Attribute Ownership Acquisition
void
RTI::RTIambassador::
attributeOwnershipAcquisition(ObjectHandle theObject,
                              const AttributeHandleSet& desiredAttributes,
                              const char *theTag)
    throw (ObjectNotKnown,
           ObjectClassNotPublished,
           AttributeNotDefined,
           AttributeNotPublished,
           FederateOwnsAttributes,
           FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
{
    Message req, rep ;

    req.type = Message::ATTRIBUTE_OWNERSHIP_ACQUISITION ;
    req.setObject(theObject);
    req.setTag(theTag);
    req.setAHS(desiredAttributes);

    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
// Attribute Ownership Release Response
AttributeHandleSet*
RTI::RTIambassador::
attributeOwnershipReleaseResponse(ObjectHandle theObject,
                                  const AttributeHandleSet& attrs)
    throw (ObjectNotKnown,
           AttributeNotDefined,
           AttributeNotOwned,
           FederateWasNotAskedToReleaseAttribute,
           FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
{
    Message req, rep ;

    req.type = Message::ATTRIBUTE_OWNERSHIP_RELEASE_RESPONSE ;
    req.setObject(theObject);
    req.setAHS(attrs);

    privateRefs->executeService(&req, &rep);

    if (rep.getExceptionType() == e_NO_EXCEPTION) {
        return rep.getAHS();
    }

    return NULL ;
}

// ----------------------------------------------------------------------------
// Cancel Negotiated Attribute Ownership Divestiture
void
RTI::RTIambassador::
cancelNegotiatedAttributeOwnershipDivestiture(ObjectHandle theObject,
                                              const AttributeHandleSet& attrs)
    throw (ObjectNotKnown,
           AttributeNotDefined,
           AttributeNotOwned,
           AttributeDivestitureWasNotRequested,
           FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
{
    Message req, rep ;

    req.type = Message::CANCEL_NEGOTIATED_ATTRIBUTE_OWNERSHIP_DIVESTITURE ;
    req.setObject(theObject);
    req.setAHS(attrs);

    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
// Cancel Attribute Ownership Acquisition
void
RTI::RTIambassador::
cancelAttributeOwnershipAcquisition(ObjectHandle theObject,
                                    const AttributeHandleSet& attrs)
    throw (ObjectNotKnown,
           AttributeNotDefined,
           AttributeAlreadyOwned,
           AttributeAcquisitionWasNotRequested,
           FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
{
    Message req, rep ;

    req.type = Message::CANCEL_ATTRIBUTE_OWNERSHIP_ACQUISITION ;
    req.setObject(theObject);
    req.setAHS(attrs);

    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
// Attribute Ownership Acquisition If Available
void
RTI::RTIambassador::
attributeOwnershipAcquisitionIfAvailable(ObjectHandle theObject,
                                         const AttributeHandleSet& desired)
    throw (ObjectNotKnown,
           ObjectClassNotPublished,
           AttributeNotDefined,
           AttributeNotPublished,
           FederateOwnsAttributes,
           AttributeAlreadyBeingAcquired,
           FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
{
    Message req, rep ;

    req.type = Message::ATTRIBUTE_OWNERSHIP_ACQUISITION_IF_AVAILABLE ;
    req.setObject(theObject);
    req.setAHS(desired);

    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
// Query Attribute Ownership
void
RTI::RTIambassador::
queryAttributeOwnership(ObjectHandle theObject,
                        AttributeHandle theAttribute)
    throw (RTI::RTIinternalError, RTI::RestoreInProgress, 
	   RTI::SaveInProgress, RTI::ConcurrentAccessAttempted, 
	   RTI::FederateNotExecutionMember, RTI::AttributeNotDefined, 
	   RTI::ObjectNotKnown)
{
    Message req, rep ;

    req.type = Message::QUERY_ATTRIBUTE_OWNERSHIP ;
    req.setObject(theObject);
    req.setAttribute(theAttribute);

    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
// 5.16 Is Attribute Owned By Federate
RTI::Boolean
RTI::RTIambassador::isAttributeOwnedByFederate(ObjectHandle theObject,
                                          AttributeHandle theAttribute)
    throw (ObjectNotKnown,
           AttributeNotDefined,
           FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
{
    Message req, rep ;

    req.type = Message::IS_ATTRIBUTE_OWNED_BY_FEDERATE ;
    req.setObject(theObject);
    req.setAttribute(theAttribute);

    privateRefs->executeService(&req, &rep);

    return ((strcmp(rep.getTag(), "RTI_TRUE") == 0) ? RTI_TRUE : RTI_FALSE);
}

// ----------------------------------------------------------------------------
// Enable Time Regulation
void
RTI::RTIambassador::enableTimeRegulation(const FedTime& /*theFederateTime*/,
                                    const FedTime& /*theLookahead*/)
    throw (RTI::RTIinternalError, RTI::RestoreInProgress, 
	   RTI::SaveInProgress, RTI::FederateNotExecutionMember, 
	   RTI::ConcurrentAccessAttempted, RTI::InvalidLookahead, 
	   RTI::InvalidFederationTime, RTI::TimeAdvanceAlreadyInProgress, 
	   RTI::EnableTimeRegulationPending, RTI::TimeRegulationAlreadyEnabled)
{
    Message req, rep ;
    req.type = Message::ENABLE_TIME_REGULATION ;
    req.setBoolean(true);
    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
// Disable Time Regulation
void
RTI::RTIambassador::disableTimeRegulation()
    throw (RTI::RTIinternalError, 
	   RTI::RestoreInProgress, RTI::SaveInProgress, 
	   RTI::FederateNotExecutionMember, RTI::ConcurrentAccessAttempted, 
	   RTI::TimeRegulationWasNotEnabled)
{
    Message req, rep ;
    req.type = Message::DISABLE_TIME_REGULATION ;
    req.setBoolean(false);
    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
// Enable Time Constrained
void
RTI::RTIambassador::enableTimeConstrained()
    throw (TimeConstrainedAlreadyEnabled, //not implemented
           EnableTimeConstrainedPending, //not implemented
           TimeAdvanceAlreadyInProgress, //not implemented
           FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
{
    Message req, rep ;

    req.type = Message::ENABLE_TIME_CONSTRAINED ;
    req.setBoolean(true);
    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
// Disable Time Constrained
void
RTI::RTIambassador::disableTimeConstrained()
    throw (TimeConstrainedWasNotEnabled, //not implemented
           FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
{
    Message req, rep ;

    req.type = Message::DISABLE_TIME_CONSTRAINED ;
    req.setBoolean(false);
    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
// Time Advance Request
void
RTI::RTIambassador::timeAdvanceRequest(const FedTime& theTime)
    throw (TimeAdvanceAlreadyInProgress,
           FederationTimeAlreadyPassed,
           InvalidFederationTime,
           EnableTimeRegulationPending, //not implemented
           EnableTimeConstrainedPending, //not implemented
           FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
{
    Message req, rep ;

    req.type = Message::TIME_ADVANCE_REQUEST ;
    req.setFedTime(theTime);
    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
// Time Advance Request Available
void
RTI::RTIambassador::timeAdvanceRequestAvailable(const FedTime& theTime)
    throw (RTI::RTIinternalError, RTI::RestoreInProgress, RTI::SaveInProgress, 
	   RTI::ConcurrentAccessAttempted, RTI::FederateNotExecutionMember, 
	   RTI::EnableTimeConstrainedPending, RTI::EnableTimeRegulationPending, 
	   RTI::TimeAdvanceAlreadyInProgress, RTI::FederationTimeAlreadyPassed, 
	   RTI::InvalidFederationTime)
{
    throw RTIinternalError("Unimplemented Service");

    Message req, rep ;

    req.type = Message::TIME_ADVANCE_REQUEST_AVAILABLE ;
    req.setFedTime(theTime);

    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
// Next Event Request
void
RTI::RTIambassador::nextEventRequest(const FedTime& theTime)
    throw (TimeAdvanceAlreadyInProgress,
           FederationTimeAlreadyPassed,
           InvalidFederationTime,
           EnableTimeRegulationPending, //not implemented
           EnableTimeConstrainedPending, //not implemented
           FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
{
    Message req, rep ;

    req.type = Message::NEXT_EVENT_REQUEST ;
    req.setFedTime(theTime);
    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
// Next Event Request Available
void
RTI::RTIambassador::nextEventRequestAvailable(const FedTime& theTime)
    throw (RTI::RTIinternalError, RTI::RestoreInProgress, RTI::SaveInProgress, 
	   RTI::ConcurrentAccessAttempted, RTI::FederateNotExecutionMember, 
	   RTI::EnableTimeConstrainedPending, RTI::EnableTimeRegulationPending, 
	   RTI::TimeAdvanceAlreadyInProgress, RTI::FederationTimeAlreadyPassed, 
	   RTI::InvalidFederationTime)
{
    throw RTI::RTIinternalError("Unimplemented Service");
    Message req, rep ;

    req.type = Message::NEXT_EVENT_REQUEST ;
    req.setFedTime(theTime);
    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
// Flush Queue Request
void
RTI::RTIambassador::flushQueueRequest(const FedTime& theTime)
    throw (TimeAdvanceAlreadyInProgress,
           FederationTimeAlreadyPassed,
           InvalidFederationTime,
           EnableTimeRegulationPending, //not implemented
           EnableTimeConstrainedPending, //not implemented
           FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
{
    throw RTIinternalError("Unimplemented Service");
    Message req, rep ;

    req.type = Message::FLUSH_QUEUE_REQUEST ;
    req.setFedTime(theTime);

    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
// Enable Asynchronous Delivery
void
RTI::RTIambassador::enableAsynchronousDelivery()
    throw (RTI::RTIinternalError, RTI::RestoreInProgress, RTI::SaveInProgress, 
	   RTI::ConcurrentAccessAttempted, RTI::FederateNotExecutionMember, 
	   RTI::AsynchronousDeliveryAlreadyEnabled)
{
    throw AsynchronousDeliveryAlreadyEnabled("Default value (non HLA)");

    Message req, rep ;

    req.type = Message::ENABLE_ASYNCHRONOUS_DELIVERY ;

    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
// Disable Asynchronous Delivery
void
RTI::RTIambassador::disableAsynchronousDelivery()
    throw (RTI::RTIinternalError, RTI::RestoreInProgress, RTI::SaveInProgress, 
	   RTI::ConcurrentAccessAttempted, RTI::FederateNotExecutionMember, 
	   RTI::AsynchronousDeliveryAlreadyDisabled)
{
    throw RTIinternalError("Unimplemented Service");
    Message req, rep ;

    req.type = Message::DISABLE_ASYNCHRONOUS_DELIVERY ;

    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
// Query LBTS
void
RTI::RTIambassador::queryLBTS(FedTime& theTime)
    throw (FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
{
    Message req, rep ;

    req.type = Message::QUERY_LBTS ;
    privateRefs->executeService(&req, &rep);

    theTime = rep.getFedTime();
}

// ----------------------------------------------------------------------------
// Query Federate Time
void
RTI::RTIambassador::queryFederateTime(FedTime& theTime)
    throw (FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
{
    Message req, rep ;

    req.type = Message::QUERY_FEDERATE_TIME ;
    privateRefs->executeService(&req, &rep);

    theTime = rep.getFedTime();
}

// ----------------------------------------------------------------------------
// Query Minimum Next Event Time
void
RTI::RTIambassador::queryMinNextEventTime(FedTime& theTime)
    throw (FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
{
    theTime = RTIfedTime(0.0);
}

// ----------------------------------------------------------------------------
// Modify Lookahead
void
RTI::RTIambassador::modifyLookahead(const FedTime& theLookahead)
    throw (RTI::RTIinternalError, RTI::RestoreInProgress, RTI::SaveInProgress, 
	   RTI::ConcurrentAccessAttempted, RTI::FederateNotExecutionMember, 
	   RTI::InvalidLookahead)
{
    Message req, rep ;

    req.type = Message::MODIFY_LOOKAHEAD ;
    req.setLookahead(theLookahead);

    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
// Query Lookahead
void
RTI::RTIambassador::queryLookahead(FedTime &theTime)
    throw (FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
{
    Message req, rep ;

    req.type = Message::QUERY_LOOKAHEAD ;
    privateRefs->executeService(&req, &rep);

    try { 
        RTIfedTime &ret = dynamic_cast<RTIfedTime&>(theTime); 
        ret = RTIfedTime((Double) rep.getFederationTimeDelta());
    }
    catch (std::bad_cast) {
	throw RTIinternalError("theTime is not a RTIfedTime object");
    }
}

// ----------------------------------------------------------------------------
// Retract
void
RTI::RTIambassador::retract(EventRetractionHandle handle)
    throw (RTI::RTIinternalError, RTI::RestoreInProgress, RTI::SaveInProgress, 
	   RTI::ConcurrentAccessAttempted, RTI::FederateNotExecutionMember, 
	   RTI::InvalidRetractionHandle)
{
    throw RTIinternalError("Unimplemented Service");
    Message req, rep ;

    req.type = Message::RETRACT ;
    req.setEventRetraction(handle);

    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
// Change Attribute Order Type
void
RTI::RTIambassador::changeAttributeOrderType(ObjectHandle theObject,
                                        const AttributeHandleSet& attrs,
                                        OrderingHandle theType)
    throw (RTI::RTIinternalError, 
	   RTI::RestoreInProgress, RTI::SaveInProgress, RTI::ConcurrentAccessAttempted, 
	   RTI::FederateNotExecutionMember, RTI::InvalidOrderingHandle, 
	   RTI::AttributeNotOwned, RTI::AttributeNotDefined, RTI::ObjectNotKnown)
{
    Message req, rep ;

    req.type = Message::CHANGE_ATTRIBUTE_ORDER_TYPE ;
    req.setObject(theObject);
    req.setOrdering(theType);
    req.setAHS(attrs);

    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
// Change Interaction Order Type
void
RTI::RTIambassador::changeInteractionOrderType(InteractionClassHandle theClass,
                                          OrderingHandle theType)
    throw (RTI::RTIinternalError, RTI::RestoreInProgress, 
	   RTI::SaveInProgress, RTI::ConcurrentAccessAttempted, 
	   RTI::FederateNotExecutionMember, RTI::InvalidOrderingHandle, 
	   RTI::InteractionClassNotPublished, RTI::InteractionClassNotDefined)
{
    Message req, rep ;

    req.type = Message::CHANGE_INTERACTION_ORDER_TYPE ;
    req.setInteractionClass(theClass);
    req.setOrdering(theType);

    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
/** Create a routing region for data distribution management.
    \param space The space handle of the region
    \param nb_extents The number of extents
    \return A Region object, associated with the created region
 */
RTI::Region *
RTI::RTIambassador::createRegion(SpaceHandle space, ULong nb_extents)
    throw (SpaceNotDefined,
           InvalidExtents,
           FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
{
    Message req, rep ;
    req.setType(Message::DDM_CREATE_REGION);
    req.setSpace(space);
    req.setNumber(nb_extents);
    privateRefs->executeService(&req, &rep);
    Region *region = new FedRegion(rep.getRegion(), space,
				   std::vector<Extent>(nb_extents,
						       Extent(rep.getNumber())));

    assert(region->getNumberOfExtents() == nb_extents);
    return region ;
}

// ----------------------------------------------------------------------------
/** Notify about region modification. Applies the changes done through
    the region services to the RTI.
    \param r The region to commit to the RTI
 */
void
RTI::RTIambassador::notifyAboutRegionModification(Region &r)
    throw (RegionNotKnown,
           InvalidExtents,
           FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
{
    try {
	FedRegion &region = dynamic_cast<FedRegion &>(r);
	D[pdDebug] << "Notify About Region " << region.getHandle()
		   << " Modification" << endl ;
	Message req, rep ;
	
	req.setType(Message::DDM_MODIFY_REGION);
	req.setRegion(region.getHandle());
	req.setExtents(region.getExtents());
	
	privateRefs->executeService(&req, &rep);
	region.commit();
    }
    catch (std::bad_cast) {
	throw RegionNotKnown("");
    }
    catch (Exception &e) {
	throw ;
    }
}

// ----------------------------------------------------------------------------
/** Delete region. Correctly destroys the region (through the RTI).
    \attention Always use this function to destroy a region. Do NOT
    use the C++ delete operator.
 */
void
RTI::RTIambassador::deleteRegion(Region *region)
    throw (RegionNotKnown,
           RegionInUse,
           FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
{
    if (region == 0) {
        throw RegionNotKnown("");
    }

    Message req, rep ;

    req.setType(Message::DDM_DELETE_REGION);
    try {
	req.setRegion(dynamic_cast<FedRegion *>(region)->getHandle());
    }
    catch (std::bad_cast) {
	throw RegionNotKnown("");
    }
    privateRefs->executeService(&req, &rep);

    delete region ;
}

// ----------------------------------------------------------------------------
// Register Object Instance With Region
ObjectHandle
RTI::RTIambassador::registerObjectInstanceWithRegion(ObjectClassHandle object_class,
                                                const char *tag,
                                                AttributeHandle attrs[],
                                                Region *regions[],
                                                ULong nb)
    throw (ObjectClassNotDefined,
           ObjectClassNotPublished,
           AttributeNotDefined,
           AttributeNotPublished,
           RegionNotKnown,
           InvalidRegionContext,
           ObjectAlreadyRegistered,
           FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
{
    Message req, rep ;

    req.setType(Message::DDM_REGISTER_OBJECT);
    req.setObjectClass(object_class);
    req.setTag(tag);
    req.setAHS(attrs, nb);
    req.setRegions(build_region_handles(regions, nb));

    privateRefs->executeService(&req, &rep);

    return rep.getObject();
}

// ----------------------------------------------------------------------------
ObjectHandle
RTI::RTIambassador::registerObjectInstanceWithRegion(ObjectClassHandle object_class,
                                                AttributeHandle attrs[],
                                                Region *regions[],
                                                ULong nb)
    throw (ObjectClassNotDefined,
           ObjectClassNotPublished,
           AttributeNotDefined,
           AttributeNotPublished,
           RegionNotKnown,
           InvalidRegionContext,
           FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
{
    Message req, rep ;

    req.setType(Message::DDM_REGISTER_OBJECT);
    req.setObjectClass(object_class);
    req.setAHS(attrs, nb);
    req.setRegions(build_region_handles(regions, nb));

    privateRefs->executeService(&req, &rep);

    return rep.getObject();
}

// ----------------------------------------------------------------------------
/** Associate region for updates. Make attributes of an object
    be updated through a routing region.
    @param region Region to use for updates
    @param object Object to associate to the region
    @param attributes Handles of the involved attributes
    @sa unassociateRegionForUpdates
*/
void
RTI::RTIambassador::associateRegionForUpdates(Region &region,
                                         ObjectHandle object,
                                         const AttributeHandleSet &attributes)
    throw (ObjectNotKnown,
           AttributeNotDefined,
           InvalidRegionContext,
           RegionNotKnown,
           FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
{
    D[pdDebug] << "+ Associate Region for Updates" << endl ;

    Message req, rep ;
    
    req.type = Message::DDM_ASSOCIATE_REGION ;
    req.setObject(object);
    req.setRegion(get_handle(region));
    req.setAHS(attributes);

    privateRefs->executeService(&req, &rep);
    D[pdDebug] << "- Associate Region for Updates" << endl ;
}

// ----------------------------------------------------------------------------
/** Unassociate region for updates. Make attributes of an object be updated
    through the default region (ie. Declaration Management services)
    @param region Region to unassociate
    @param object Object to unassociate
    @see associateRegionForUpdates
 */
void
RTI::RTIambassador::unassociateRegionForUpdates(Region &region,
                                           ObjectHandle object)
    throw (ObjectNotKnown,
           InvalidRegionContext,
           RegionNotKnown,
           FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
{
    D[pdDebug] << "+ Unassociate Region for Updates" << endl ;
    Message req, rep ;

    req.type = Message::DDM_UNASSOCIATE_REGION ;
    req.setObject(object);
    req.setRegion(get_handle(region));

    privateRefs->executeService(&req, &rep);
    D[pdDebug] << "- Unassociate Region for Updates" << endl ;
}

// ----------------------------------------------------------------------------
/** Subscribe object class attributes with region.
    @param handle Object class handle
    @param region Region to subscribe with
    @param attributes Attributes involved in the subscription
    @sa unsubscribeObjectClassWithRegion
 */
void
RTI::RTIambassador::subscribeObjectClassAttributesWithRegion(
    ObjectClassHandle object_class,
    Region &region,
    const AttributeHandleSet &attributes,
    Boolean passive)
    throw (ObjectClassNotDefined,
           AttributeNotDefined,
           RegionNotKnown,
           InvalidRegionContext,
           FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
{
    D[pdDebug] << "+ Subscribe Object Class Attributes with Region" << endl ;
    Message req, rep ;

    req.type = Message::DDM_SUBSCRIBE_ATTRIBUTES ;
    req.setObjectClass(object_class);
    req.setRegion(get_handle(region));
    req.setAHS(attributes);
    req.setBoolean(passive);

    privateRefs->executeService(&req, &rep);
    D[pdDebug] << "- Subscribe Object Class Attributes with Region" << endl ;
}

// ----------------------------------------------------------------------------
/** Unsubscribe object class attributes with region.
    @param handle Object Class handle
    @param region Region to unsubscribe with
    @sa subscribeObjectClassAttributesWithRegion
 */
void
RTI::RTIambassador::unsubscribeObjectClassWithRegion(ObjectClassHandle object_class,
                                                Region &region)
    throw (ObjectClassNotDefined,
           RegionNotKnown,
           ObjectClassNotSubscribed,
           FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
{
    D[pdDebug] << "+ Unsubscribe Object Class " << object_class
	       << " with Region" << endl ;
    Message req, rep ;

    req.type = Message::DDM_UNSUBSCRIBE_ATTRIBUTES ;
    req.setObjectClass(object_class);
    req.setRegion(get_handle(region));

    privateRefs->executeService(&req, &rep);
    D[pdDebug] << "- Unsubscribe Object Class with Region" << endl ;
}

// ----------------------------------------------------------------------------
// Subscribe Interaction Class With Region
void
RTI::RTIambassador::subscribeInteractionClassWithRegion(InteractionClassHandle ic,
                                                   Region &region,
                                                   RTI::Boolean passive)
    throw (InteractionClassNotDefined,
           RegionNotKnown,
           InvalidRegionContext,
           FederateLoggingServiceCalls,
           FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
{
    Message req, rep ;

    req.type = Message::DDM_SUBSCRIBE_INTERACTION ;
    req.setInteractionClass(ic);
    req.setRegion(get_handle(region));
    req.setBoolean(passive);

    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
// UnSubscribe Interaction Class With Region
void
RTI::RTIambassador::unsubscribeInteractionClassWithRegion(InteractionClassHandle ic,
                                                     Region &region)
    throw (InteractionClassNotDefined,
           InteractionClassNotSubscribed,
           RegionNotKnown,
           FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
{
    Message req, rep ;

    req.type = Message::DDM_UNSUBSCRIBE_INTERACTION ;
    req.setInteractionClass(ic);
    req.setRegion(get_handle(region));

    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
// Send Interaction With Region
EventRetractionHandle
RTI::RTIambassador::sendInteractionWithRegion(InteractionClassHandle interaction,
                                         const ParameterHandleValuePairSet &par,
                                         const FedTime &time,
                                         const char *tag,
                                         const Region &region)
    throw (InteractionClassNotDefined,
           InteractionClassNotPublished,
           InteractionParameterNotDefined,
           InvalidFederationTime,
           RegionNotKnown,
           InvalidRegionContext,
           FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
{
    Message req, rep ;

    req.setType(Message::SEND_INTERACTION);
    req.setInteractionClass(interaction);
    req.setPHVPS(par);
    req.setFedTime(time);
    req.setTag(tag);
    req.setRegion(get_handle(region));

    privateRefs->executeService(&req, &rep);

    return rep.getEventRetraction();
}

// ----------------------------------------------------------------------------
void
RTI::RTIambassador::sendInteractionWithRegion(InteractionClassHandle interaction,
                                         const ParameterHandleValuePairSet &par,
                                         const char *tag,
                                         const Region &region)
    throw (InteractionClassNotDefined,
           InteractionClassNotPublished,
           InteractionParameterNotDefined,
           RegionNotKnown,
           InvalidRegionContext,
           FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
{
    Message req, rep ;

    req.setType(Message::SEND_INTERACTION);
    req.setInteractionClass(interaction);
    req.setPHVPS(par);
    req.setTag(tag);
    req.setRegion(get_handle(region));

    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
// Request Class Attribute Value Update With Region
void RTI::RTIambassador::
requestClassAttributeValueUpdateWithRegion(ObjectClassHandle /*object*/,
                                           const AttributeHandleSet &attrs,
                                           const Region &region)
    throw (RTI::RTIinternalError, RTI::RestoreInProgress, RTI::SaveInProgress, 
	   RTI::ConcurrentAccessAttempted, RTI::FederateNotExecutionMember, 
	   RTI::RegionNotKnown, RTI::AttributeNotDefined, RTI::ObjectClassNotDefined)
{
    throw UnimplementedService("");

    Message req, rep ;
    req.setType(Message::DDM_REQUEST_UPDATE);
    req.setAHS(attrs);
    req.setRegion(get_handle(region));
    privateRefs->executeService(&req, &rep);    
}

// ----------------------------------------------------------------------------
/** Get object class handle
    \param theName Name of the object class
 */
ObjectClassHandle
RTI::RTIambassador::getObjectClassHandle(const char *theName)
    throw (NameNotFound,
           FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           RTIinternalError)
{
    Message req, rep ;
    req.type = Message::GET_OBJECT_CLASS_HANDLE ;
    req.setName(theName);
    privateRefs->executeService(&req, &rep);
    return rep.getObjectClass();
}

// ----------------------------------------------------------------------------
/** Get object class name.
    \param handle Handle of the object class
    \return The class name associated with the handle, memory has to
    be freed by the caller.
*/
char *
RTI::RTIambassador::getObjectClassName(ObjectClassHandle handle)
    throw (ObjectClassNotDefined,
           FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           RTIinternalError)
{
    Message req, rep ;
    req.type = Message::GET_OBJECT_CLASS_NAME ;
    req.setObjectClass(handle);
    privateRefs->executeService(&req, &rep);
    return strdup(rep.getName());
}

// ----------------------------------------------------------------------------
/** Get attribute handle.
    \param theName Name of the attribute
    \param whichClass Handle of the attribute's class
 */
AttributeHandle
RTI::RTIambassador::getAttributeHandle(const char *theName,
                                  ObjectClassHandle whichClass)
    throw (ObjectClassNotDefined,
           NameNotFound,
           FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           RTIinternalError)

{
    Message req, rep ;
    req.type = Message::GET_ATTRIBUTE_HANDLE ;
    req.setName(theName);
    req.setObjectClass(whichClass);
    privateRefs->executeService(&req, &rep);
    return rep.getAttribute();
}

// ----------------------------------------------------------------------------
/** Get attribute name.
    \param theHandle Handle of the attribute
    \param whichClass Handle of the attribute's class
 */

char *
RTI::RTIambassador::getAttributeName(AttributeHandle theHandle,
                                ObjectClassHandle whichClass)
    throw (ObjectClassNotDefined,
           AttributeNotDefined,
           FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           RTIinternalError)
{
    Message req, rep ;
    req.type = Message::GET_ATTRIBUTE_NAME ;
    req.setAttribute(theHandle);
    req.setObjectClass(whichClass);
    privateRefs->executeService(&req, &rep);
    return strdup(rep.getName());
}

// ----------------------------------------------------------------------------
// Get Interaction Class Handle
InteractionClassHandle
RTI::RTIambassador::getInteractionClassHandle(const char *theName)
    throw (NameNotFound,
           FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           RTIinternalError)
{
    Message req, rep ;

    req.type = Message::GET_INTERACTION_CLASS_HANDLE ;
    req.setName(theName);

    privateRefs->executeService(&req, &rep);

    return rep.getInteractionClass();
}

// ----------------------------------------------------------------------------
// Get Interaction Class Name
char *
RTI::RTIambassador::getInteractionClassName(InteractionClassHandle theHandle)
    throw (InteractionClassNotDefined,
           FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           RTIinternalError)
{
    Message req, rep ;

    req.type = Message::GET_INTERACTION_CLASS_NAME ;
    req.setInteractionClass(theHandle);

    privateRefs->executeService(&req, &rep);

    return strdup(rep.getName());
}

// ----------------------------------------------------------------------------
// Get Parameter Handle
ParameterHandle
RTI::RTIambassador::getParameterHandle(const char *theName,
                                  InteractionClassHandle whichClass)
    throw (InteractionClassNotDefined,
           NameNotFound,
           FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           RTIinternalError)
{
    Message req, rep ;

    req.type = Message::GET_PARAMETER_HANDLE ;
    req.setName(theName);
    req.setInteractionClass(whichClass);

    privateRefs->executeService(&req, &rep);

    return rep.getParameter();
}

// ----------------------------------------------------------------------------
// Get Parameter Name
char *
RTI::RTIambassador::getParameterName(ParameterHandle theHandle,
                                InteractionClassHandle whichClass)
    throw (InteractionClassNotDefined,
           InteractionParameterNotDefined,
           FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           RTIinternalError)
{
    Message req, rep ;

    req.type = Message::GET_PARAMETER_NAME ;
    req.setParameter(theHandle);
    req.setInteractionClass(whichClass);

    privateRefs->executeService(&req, &rep);

    return strdup(rep.getName());
}

// ----------------------------------------------------------------------------
// Get Object Instance Handle
ObjectHandle
RTI::RTIambassador::getObjectInstanceHandle(const char *theName)
    throw (RTI::RTIinternalError, RTI::ConcurrentAccessAttempted, 
   RTI::FederateNotExecutionMember, RTI::ObjectNotKnown)
{
    Message req, rep ;

    req.type = Message::GET_OBJECT_INSTANCE_HANDLE ;
    req.setName(theName);

    privateRefs->executeService(&req, &rep);

    return rep.getObject();
}

// ----------------------------------------------------------------------------
// Get Object Instance Name
char *
RTI::RTIambassador::getObjectInstanceName(ObjectHandle theHandle)
    throw (RTI::RTIinternalError, RTI::ConcurrentAccessAttempted, 
	   RTI::FederateNotExecutionMember, RTI::ObjectNotKnown)
{
    Message req, rep ;

    req.type = Message::GET_OBJECT_INSTANCE_NAME ;
    req.setObject(theHandle);

    privateRefs->executeService(&req, &rep);

    return strdup(rep.getName());
}

// ----------------------------------------------------------------------------
/** Get routing space handle
    \param rs_name Name of the routing space
 */
SpaceHandle
RTI::RTIambassador::getRoutingSpaceHandle(const char *rs_name)
    throw (NameNotFound,
           FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           RTIinternalError)
{
    D[pdDebug] << "Get routing space handle: " << rs_name << endl ;
    Message req, rep ;
    req.type = Message::GET_SPACE_HANDLE ;
    req.setName(rs_name);
    privateRefs->executeService(&req, &rep);
    return rep.getSpace();
}

// ----------------------------------------------------------------------------
/** Get routing space name
    \param handle Handle of the routing space
 */
char *
RTI::RTIambassador::getRoutingSpaceName(SpaceHandle handle)
    throw (SpaceNotDefined,
           FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           RTIinternalError)
{
    Message req, rep ;
    req.type = Message::GET_SPACE_NAME ;
    req.setSpace(handle);
    privateRefs->executeService(&req, &rep);
    return strdup(rep.getName());
}

// ----------------------------------------------------------------------------
/** Get dimension handle
    \parameter dimension Name of the dimension
    \parameter space The dimension's routing space handle
 */
DimensionHandle
RTI::RTIambassador::getDimensionHandle(const char *dimension,
                                  SpaceHandle space)
    throw (SpaceNotDefined,
           NameNotFound,
           FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           RTIinternalError)
{
    Message req, rep ;
    req.type = Message::GET_DIMENSION_HANDLE ;
    req.setName(dimension);
    req.setSpace(space);
    privateRefs->executeService(&req, &rep);
    return rep.getDimension();
}

// ----------------------------------------------------------------------------
/** Get dimension name
    \param dimension Handle of the dimension
    \param space The dimension's routing space handle
 */
char *
RTI::RTIambassador::getDimensionName(DimensionHandle dimension,
                                SpaceHandle space)
    throw (SpaceNotDefined,
           DimensionNotDefined,
           FederateNotExecutionMember,
           ConcurrentAccessAttempted,
           RTIinternalError)
{
    Message req, rep ;
    req.type = Message::GET_DIMENSION_NAME ;
    req.setDimension(dimension);
    req.setSpace(space);
    privateRefs->executeService(&req, &rep);
    return strdup(rep.getName());
}

// ----------------------------------------------------------------------------
/** Get attribute routing space handle
    \param attribute The attribute handle
    \param object_class The attribute's class handle
    \return The associated routing space handle
 */
SpaceHandle
RTI::RTIambassador::getAttributeRoutingSpaceHandle(AttributeHandle attribute,
                                              ObjectClassHandle object_class)
    throw (ObjectClassNotDefined,
	   AttributeNotDefined,
           FederateNotExecutionMember,
	   ConcurrentAccessAttempted,
           RTIinternalError)
{
    Message req, rep ;
    req.type = Message::GET_ATTRIBUTE_SPACE_HANDLE ;
    req.setAttribute(attribute);
    req.setObjectClass(object_class);
    privateRefs->executeService(&req, &rep);
    return rep.getSpace();
}

// ----------------------------------------------------------------------------
// Get Object Class
ObjectClassHandle
RTI::RTIambassador::getObjectClass(ObjectHandle theObject)
    throw (RTI::RTIinternalError, RTI::ConcurrentAccessAttempted, 
	   RTI::FederateNotExecutionMember, RTI::ObjectNotKnown)
{
    Message req, rep ;
    req.type = Message::GET_OBJECT_CLASS ;
    req.setObject(theObject);
    privateRefs->executeService(&req, &rep);
    return rep.getObjectClass();
}

// ----------------------------------------------------------------------------
/** Get interaction routing space handle
    \param inter The interaction handle
    \return The associated routing space
 */
SpaceHandle
RTI::RTIambassador::getInteractionRoutingSpaceHandle(InteractionClassHandle inter)
    throw (InteractionClassNotDefined,
	   FederateNotExecutionMember,
           ConcurrentAccessAttempted,
	   RTIinternalError)
{
    Message req, rep ;
    req.type = Message::GET_INTERACTION_SPACE_HANDLE ;
    req.setInteractionClass(inter);
    this->privateRefs->executeService(&req, &rep);
    return rep.getSpace();
}

// ----------------------------------------------------------------------------
// Get Transportation Handle
RTI::TransportationHandle
RTI::RTIambassador::getTransportationHandle(const char *theName)
    throw (RTI::RTIinternalError, RTI::ConcurrentAccessAttempted, 
	   RTI::FederateNotExecutionMember, RTI::NameNotFound)
{
    throw UnimplementedService("");
    Message req, rep ;
    req.type = Message::GET_TRANSPORTATION_HANDLE ;
    req.setName(theName);
    privateRefs->executeService(&req, &rep);
    return rep.getTransportation();
}

// ----------------------------------------------------------------------------
// Get Transportation Name
char *
RTI::RTIambassador::getTransportationName(TransportationHandle theHandle)
    throw (RTI::RTIinternalError, RTI::ConcurrentAccessAttempted, 
	   RTI::FederateNotExecutionMember, RTI::InvalidTransportationHandle)
{
    throw UnimplementedService("");
    Message req, rep ;
    req.type = Message::GET_TRANSPORTATION_NAME ;
    req.setTransportation(theHandle);
    privateRefs->executeService(&req, &rep);
    return(strdup(rep.getName()));
}

// ----------------------------------------------------------------------------
// Get Ordering Handle
RTI::OrderingHandle
RTI::RTIambassador::getOrderingHandle(const char *theName)
    throw (RTI::RTIinternalError, RTI::ConcurrentAccessAttempted, 
	   RTI::FederateNotExecutionMember, RTI::NameNotFound)
{
    throw UnimplementedService("");
    Message req, rep ;
    req.type = Message::GET_ORDERING_HANDLE ;
    req.setName(theName);
    privateRefs->executeService(&req, &rep);
    return rep.getOrdering();
}


// ----------------------------------------------------------------------------
// Get Ordering Name
char *
RTI::RTIambassador::getOrderingName(OrderingHandle theHandle)
    throw (RTI::RTIinternalError, RTI::ConcurrentAccessAttempted, 
	   RTI::FederateNotExecutionMember, RTI::InvalidOrderingHandle)
{
    throw UnimplementedService("");
    Message req, rep ;
    req.type = Message::GET_ORDERING_NAME ;
    req.setOrdering(theHandle);
    privateRefs->executeService(&req, &rep);
    return(strdup(rep.getName()));
}

// ----------------------------------------------------------------------------
// Enable Class Relevance Advisory Switch
void
RTI::RTIambassador::enableClassRelevanceAdvisorySwitch()
    throw (RTI::RTIinternalError, RTI::RestoreInProgress, RTI::SaveInProgress, 
	   RTI::ConcurrentAccessAttempted, RTI::FederateNotExecutionMember)
{
    throw UnimplementedService("");
    Message req, rep ;
    req.type = Message::ENABLE_CLASS_RELEVANCE_ADVISORY_SWITCH ;
    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
// Disable Class Relevance Advisory Switch
void
RTI::RTIambassador::disableClassRelevanceAdvisorySwitch()
    throw (RTI::RTIinternalError, RTI::RestoreInProgress, RTI::SaveInProgress, 
	   RTI::ConcurrentAccessAttempted, RTI::FederateNotExecutionMember)
{
    throw UnimplementedService("");
    Message req, rep ;
    req.type = Message::DISABLE_CLASS_RELEVANCE_ADVISORY_SWITCH ;
    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
// Enable Attribute Relevance Advisory Switch
void
RTI::RTIambassador::enableAttributeRelevanceAdvisorySwitch()
    throw (RTI::RTIinternalError, RTI::RestoreInProgress, RTI::SaveInProgress, 
	   RTI::ConcurrentAccessAttempted, RTI::FederateNotExecutionMember)
{
    throw UnimplementedService("");
    Message req, rep ;
    req.type = Message::ENABLE_ATTRIBUTE_RELEVANCE_ADVISORY_SWITCH ;
    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
// Disable Attribute Relevance Advisory Switch
void
RTI::RTIambassador::disableAttributeRelevanceAdvisorySwitch()
    throw (RTI::RTIinternalError, RTI::RestoreInProgress, RTI::SaveInProgress, 
	   RTI::ConcurrentAccessAttempted, RTI::FederateNotExecutionMember)
{
    throw UnimplementedService("");
    Message req, rep ;
    req.type = Message::DISABLE_ATTRIBUTE_RELEVANCE_ADVISORY_SWITCH ;
    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
// Enable Attribute Scope Advisory Switch
void RTI::RTIambassador::enableAttributeScopeAdvisorySwitch()
    throw (RTI::RTIinternalError, RTI::RestoreInProgress, RTI::SaveInProgress, 
	   RTI::ConcurrentAccessAttempted, RTI::FederateNotExecutionMember)
{
    throw UnimplementedService("");
    Message req, rep ;
    req.type = Message::ENABLE_ATTRIBUTE_SCOPE_ADVISORY_SWITCH ;
    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
// Disable Attribute Scope Advisory Switch
void
RTI::RTIambassador::disableAttributeScopeAdvisorySwitch()
    throw (RTI::RTIinternalError, RTI::RestoreInProgress, RTI::SaveInProgress, 
	   RTI::ConcurrentAccessAttempted, RTI::FederateNotExecutionMember)
{
    throw UnimplementedService("");
    Message req, rep ;
    req.type = Message::DISABLE_ATTRIBUTE_SCOPE_ADVISORY_SWITCH ;
    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
// Enable Interaction Relevance Advisory Switch
void
RTI::RTIambassador::enableInteractionRelevanceAdvisorySwitch()
    throw (RTI::RTIinternalError, RTI::RestoreInProgress, RTI::SaveInProgress, 
	   RTI::ConcurrentAccessAttempted, RTI::FederateNotExecutionMember)
{
    throw UnimplementedService("");
    Message req, rep ;
    req.type = Message::ENABLE_INTERACTION_RELEVANCE_ADVISORY_SWITCH ;
    privateRefs->executeService(&req, &rep);
}

// ----------------------------------------------------------------------------
// Disable Interaction Relevance Advisory Switch
void
RTI::RTIambassador::disableInteractionRelevanceAdvisorySwitch()
    throw (RTI::RTIinternalError, RTI::RestoreInProgress, RTI::SaveInProgress, 
	   RTI::ConcurrentAccessAttempted, RTI::FederateNotExecutionMember)
{
    throw UnimplementedService("");
    Message req, rep ;
    req.type = Message::DISABLE_INTERACTION_RELEVANCE_ADVISORY_SWITCH ;
    privateRefs->executeService(&req, &rep);
}

// $Id: RTIambassador.cc,v 3.37.2.1 2007/02/02 14:19:35 rousse Exp $
