// ----------------------------------------------------------------------------
// CERTI - HLA RunTime Infrastructure
// Copyright (C) 2002-2014  ONERA
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
// ----------------------------------------------------------------------------
// This interface is used to access the services of the RTI. 

#ifndef RTIambassador_h
#define RTIambassador_h

#include <RTI/RTIambassador.h>
#include <RTI/RTIambassadorFactory.h>
#include "RTIambassadorImplementation.h"
#include "RTIambPrivateRefs.h"

namespace certi
{
class RTI_EXPORT RTI1516ambassador : rti1516::RTIambassador
{
    friend std::auto_ptr< rti1516::RTIambassador >
    rti1516::RTIambassadorFactory::createRTIambassador(std::vector< std::wstring > & args);

private:
    RTI1516ambPrivateRefs* privateRefs ;

    // Helper functions
    template<typename T> void
    assignAHSAndExecuteService(const rti1516::AttributeHandleSet &AHS, T &req, T &rep);
    template<typename T> void
    assignPHVMAndExecuteService(const rti1516::ParameterHandleValueMap &PHVM, T &req, T &rep);
    template<typename T> void
    assignAHVMAndExecuteService(const rti1516::AttributeHandleValueMap &AHVM, T &req, T &rep);
    // Helper function for CallBacks
    bool __tick_kernel(bool, TickTime, TickTime);

protected:
    RTI1516ambassador()
        noexcept;

public:
    virtual
    ~RTI1516ambassador();
    // noexcept

    // 4.2
    virtual void createFederationExecution
    (std::wstring const & federationExecutionName,
            std::wstring const & fullPathNameToTheFDDfile,
            std::wstring const & logicalTimeImplementationName = L"");

    // 4.3
    virtual void destroyFederationExecution
    (std::wstring const & federationExecutionName);

    // 4.4
    virtual rti1516::FederateHandle joinFederationExecution
    (std::wstring const & federateType,
           std::wstring const & federationExecutionName,
           rti1516::FederateAmbassador & federateAmbassador);

    // 4.5
    virtual void resignFederationExecution
    (rti1516::ResignAction resignAction);

    // 4.6
    virtual void registerFederationSynchronizationPoint
    (std::wstring const & label,
            rti1516::VariableLengthData const & theUserSuppliedTag);

    virtual void registerFederationSynchronizationPoint
    (std::wstring const & label,
            rti1516::VariableLengthData const & theUserSuppliedTag,
            rti1516::FederateHandleSet const & syncSet);

    // 4.9
    virtual void synchronizationPointAchieved
    (std::wstring const & label);

    // 4.11
    virtual void requestFederationSave
    (std::wstring const & label);

    virtual void requestFederationSave
    (std::wstring const & label,
            rti1516::LogicalTime const & theTime);

    // 4.13
    virtual void federateSaveBegun ();

    // 4.14
    virtual void federateSaveComplete ();

    virtual void federateSaveNotComplete();

    // 4.16
    virtual void queryFederationSaveStatus ();

    // 4.18
    virtual void requestFederationRestore
    (std::wstring const & label);

    // 4.22
    virtual void federateRestoreComplete ();

    virtual void federateRestoreNotComplete ();

    // 4.24
    virtual void queryFederationRestoreStatus ();

    /////////////////////////////////////
    // Declaration Management Services //
    /////////////////////////////////////

    // 5.2
    virtual void publishObjectClassAttributes
    (rti1516::ObjectClassHandle theClass,
            rti1516::AttributeHandleSet const & attributeList);

    // 5.3
    virtual void unpublishObjectClass
    (rti1516::ObjectClassHandle theClass);

    virtual void unpublishObjectClassAttributes
    (rti1516::ObjectClassHandle theClass,
            rti1516::AttributeHandleSet const & attributeList);

    // 5.4
    virtual void publishInteractionClass
    (rti1516::InteractionClassHandle theInteraction);

    // 5.5
    virtual void unpublishInteractionClass
    (rti1516::InteractionClassHandle theInteraction);

    // 5.6
    virtual void subscribeObjectClassAttributes
    (rti1516::ObjectClassHandle theClass,
            rti1516::AttributeHandleSet const & attributeList,
            bool active = true);

    // 5.7
    virtual void unsubscribeObjectClass
    (rti1516::ObjectClassHandle theClass);

    virtual void unsubscribeObjectClassAttributes
    (rti1516::ObjectClassHandle theClass,
            rti1516::AttributeHandleSet const & attributeList);

    // 5.8
    virtual void subscribeInteractionClass
    (rti1516::InteractionClassHandle theClass,
            bool active = true);

    // 5.9
    virtual void unsubscribeInteractionClass
    (rti1516::InteractionClassHandle theClass);

    ////////////////////////////////
    // Object Management Services //
    ////////////////////////////////

    // 6.2
    virtual void reserveObjectInstanceName
    (std::wstring const & theObjectInstanceName);

    // 6.4
    virtual rti1516::ObjectInstanceHandle registerObjectInstance
    (rti1516::ObjectClassHandle theClass);

    virtual rti1516::ObjectInstanceHandle registerObjectInstance
    (rti1516::ObjectClassHandle theClass,
            std::wstring const & theObjectInstanceName);

    // 6.6
    virtual void updateAttributeValues
    (rti1516::ObjectInstanceHandle theObject,
            rti1516::AttributeHandleValueMap const & theAttributeValues,
            rti1516::VariableLengthData const & theUserSuppliedTag);

    virtual rti1516::MessageRetractionHandle updateAttributeValues
    (rti1516::ObjectInstanceHandle theObject,
            rti1516::AttributeHandleValueMap const & theAttributeValues,
            rti1516::VariableLengthData const & theUserSuppliedTag,
            rti1516::LogicalTime const & theTime);

    // 6.8
    virtual void sendInteraction
    (rti1516::InteractionClassHandle theInteraction,
            rti1516::ParameterHandleValueMap const & theParameterValues,
            rti1516::VariableLengthData const & theUserSuppliedTag);

    virtual rti1516::MessageRetractionHandle sendInteraction
    (rti1516::InteractionClassHandle theInteraction,
            rti1516::ParameterHandleValueMap const & theParameterValues,
            rti1516::VariableLengthData const & theUserSuppliedTag,
            rti1516::LogicalTime const & theTime);

    // 6.10
    virtual void deleteObjectInstance
    (rti1516::ObjectInstanceHandle theObject,
            rti1516::VariableLengthData const & theUserSuppliedTag);

    virtual rti1516::MessageRetractionHandle deleteObjectInstance
    (rti1516::ObjectInstanceHandle theObject,
            rti1516::VariableLengthData const & theUserSuppliedTag,
            rti1516::LogicalTime  const & theTime);

    // 6.12
    virtual void localDeleteObjectInstance
    (rti1516::ObjectInstanceHandle theObject);

    // 6.13
    virtual void changeAttributeTransportationType
    (rti1516::ObjectInstanceHandle theObject,
            rti1516::AttributeHandleSet const & theAttributes,
            rti1516::TransportationType theType);

    // 6.14
    virtual void changeInteractionTransportationType
    (rti1516::InteractionClassHandle theClass,
            rti1516::TransportationType theType);

    // 6.17
    virtual void requestAttributeValueUpdate
    (rti1516::ObjectInstanceHandle theObject,
            rti1516::AttributeHandleSet const & theAttributes,
            rti1516::VariableLengthData const & theUserSuppliedTag);

    virtual void requestAttributeValueUpdate
    (rti1516::ObjectClassHandle theClass,
            rti1516::AttributeHandleSet const & theAttributes,
            rti1516::VariableLengthData const & theUserSuppliedTag);

    ///////////////////////////////////
    // Ownership Management Services //
    ///////////////////////////////////
    // 7.2
    virtual void unconditionalAttributeOwnershipDivestiture
    (rti1516::ObjectInstanceHandle theObject,
            rti1516::AttributeHandleSet const & theAttributes);

    // 7.3
    virtual void negotiatedAttributeOwnershipDivestiture
    (rti1516::ObjectInstanceHandle theObject,
            rti1516::AttributeHandleSet const & theAttributes,
            rti1516::VariableLengthData const & theUserSuppliedTag);

    // 7.6
    virtual void confirmDivestiture
    (rti1516::ObjectInstanceHandle theObject,
            rti1516::AttributeHandleSet const & confirmedAttributes,
            rti1516::VariableLengthData const & theUserSuppliedTag);

    // 7.8
    virtual void attributeOwnershipAcquisition
    (rti1516::ObjectInstanceHandle theObject,
            rti1516::AttributeHandleSet const & desiredAttributes,
            rti1516::VariableLengthData const & theUserSuppliedTag);

    // 7.9
    virtual void attributeOwnershipAcquisitionIfAvailable
    (rti1516::ObjectInstanceHandle theObject,
            rti1516::AttributeHandleSet const & desiredAttributes);

    // 7.12
    virtual void attributeOwnershipDivestitureIfWanted
    (rti1516::ObjectInstanceHandle theObject,
            rti1516::AttributeHandleSet const & theAttributes,
            rti1516::AttributeHandleSet & theDivestedAttributes); // filled by RTI

    // 7.13
    virtual void cancelNegotiatedAttributeOwnershipDivestiture
    (rti1516::ObjectInstanceHandle theObject,
            rti1516::AttributeHandleSet const & theAttributes);

    // 7.14
    virtual void cancelAttributeOwnershipAcquisition
    (rti1516::ObjectInstanceHandle theObject,
            rti1516::AttributeHandleSet const & theAttributes);

    // 7.16
    virtual void queryAttributeOwnership
    (rti1516::ObjectInstanceHandle theObject,
            rti1516::AttributeHandle theAttribute);

    // 7.18
    virtual bool isAttributeOwnedByFederate
    (rti1516::ObjectInstanceHandle theObject,
            rti1516::AttributeHandle theAttribute);

    //////////////////////////////
    // Time Management Services //
    //////////////////////////////

    // 8.2
    virtual void enableTimeRegulation
    (rti1516::LogicalTimeInterval const & theLookahead);

    // 8.4
    virtual void disableTimeRegulation ();

    // 8.5
    virtual void enableTimeConstrained ();

    // 8.7
    virtual void disableTimeConstrained ();

    // 8.8
    virtual void timeAdvanceRequest
    (rti1516::LogicalTime const & theTime);

    // 8.9
    virtual void timeAdvanceRequestAvailable
    (rti1516::LogicalTime const & theTime);

    // 8.10
    virtual void nextMessageRequest
    (rti1516::LogicalTime const & theTime);

    // 8.11
    virtual void nextMessageRequestAvailable
    (rti1516::LogicalTime const & theTime);

    // 8.12
    virtual void flushQueueRequest
    (rti1516::LogicalTime const & theTime);

    // 8.14
    virtual void enableAsynchronousDelivery ();

    // 8.15
    virtual void disableAsynchronousDelivery ();

    // 8.16
    virtual bool queryGALT (rti1516::LogicalTime & theTime);

    // 8.17
    virtual void queryLogicalTime (rti1516::LogicalTime & theTime);

    // 8.18
    virtual bool queryLITS (rti1516::LogicalTime & theTime);

    // 8.19
    virtual void modifyLookahead
    (rti1516::LogicalTimeInterval const & theLookahead);

    // 8.20
    virtual void queryLookahead (rti1516::LogicalTimeInterval & interval);

    // 8.21
    virtual void retract
    (rti1516::MessageRetractionHandle theHandle);

    // 8.23
    virtual void changeAttributeOrderType
    (rti1516::ObjectInstanceHandle theObject,
            rti1516::AttributeHandleSet const & theAttributes,
            rti1516::OrderType theType);

    // 8.24
    virtual void changeInteractionOrderType
    (rti1516::InteractionClassHandle theClass,
            rti1516::OrderType theType);

    //////////////////////////////////
    // Data Distribution Management //
    //////////////////////////////////

    // 9.2
    virtual rti1516::RegionHandle createRegion
    (rti1516::DimensionHandleSet const & theDimensions);

    // 9.3
    virtual void commitRegionModifications
    (rti1516::RegionHandleSet const & theRegionHandleSet);

    // 9.4
    virtual void deleteRegion
    (rti1516::RegionHandle theRegion);

    // 9.5
    virtual rti1516::ObjectInstanceHandle registerObjectInstanceWithRegions
    (rti1516::ObjectClassHandle theClass,
            rti1516::AttributeHandleSetRegionHandleSetPairVector const &
            theAttributeHandleSetRegionHandleSetPairVector);

    virtual rti1516::ObjectInstanceHandle registerObjectInstanceWithRegions
    (rti1516::ObjectClassHandle theClass,
            rti1516::AttributeHandleSetRegionHandleSetPairVector const &
            theAttributeHandleSetRegionHandleSetPairVector,
            std::wstring const & theObjectInstanceName);

    // 9.6
    virtual void associateRegionsForUpdates
    (rti1516::ObjectInstanceHandle theObject,
            rti1516::AttributeHandleSetRegionHandleSetPairVector const &
            theAttributeHandleSetRegionHandleSetPairVector);

    // 9.7
    virtual void unassociateRegionsForUpdates
    (rti1516::ObjectInstanceHandle theObject,
            rti1516::AttributeHandleSetRegionHandleSetPairVector const &
            theAttributeHandleSetRegionHandleSetPairVector);

    // 9.8
    virtual void subscribeObjectClassAttributesWithRegions
    (rti1516::ObjectClassHandle theClass,
            rti1516::AttributeHandleSetRegionHandleSetPairVector const &
            theAttributeHandleSetRegionHandleSetPairVector,
            bool active = true);

    // 9.9
    virtual void unsubscribeObjectClassAttributesWithRegions
    (rti1516::ObjectClassHandle theClass,
            rti1516::AttributeHandleSetRegionHandleSetPairVector const &
            theAttributeHandleSetRegionHandleSetPairVector);

    // 9.10
    virtual void subscribeInteractionClassWithRegions
    (rti1516::InteractionClassHandle theClass,
            rti1516::RegionHandleSet const & theRegionHandleSet,
            bool active = true);

    // 9.11
    virtual void unsubscribeInteractionClassWithRegions
    (rti1516::InteractionClassHandle theClass,
            rti1516::RegionHandleSet const & theRegionHandleSet);

    // 9.12
    virtual void sendInteractionWithRegions
    (rti1516::InteractionClassHandle theInteraction,
            rti1516::ParameterHandleValueMap const & theParameterValues,
            rti1516::RegionHandleSet const & theRegionHandleSet,
            rti1516::VariableLengthData const & theUserSuppliedTag);

    virtual rti1516::MessageRetractionHandle sendInteractionWithRegions
    (rti1516::InteractionClassHandle theInteraction,
            rti1516::ParameterHandleValueMap const & theParameterValues,
            rti1516::RegionHandleSet const & theRegionHandleSet,
            rti1516::VariableLengthData const & theUserSuppliedTag,
            rti1516::LogicalTime const & theTime);

    // 9.13
    virtual void requestAttributeValueUpdateWithRegions
    (rti1516::ObjectClassHandle theClass,
            rti1516::AttributeHandleSetRegionHandleSetPairVector const & theSet,
            rti1516::VariableLengthData const & theUserSuppliedTag);

    //////////////////////////
    // RTI Support Services //
    //////////////////////////

    // 10.2
    virtual rti1516::ObjectClassHandle getObjectClassHandle
    (std::wstring const & theName);

    // 10.3
    virtual std::wstring getObjectClassName
    (rti1516::ObjectClassHandle theHandle);

    // 10.4
    virtual rti1516::AttributeHandle getAttributeHandle
    (rti1516::ObjectClassHandle whichClass,
            std::wstring const & theAttributeName);

    // 10.5
    virtual std::wstring getAttributeName
    (rti1516::ObjectClassHandle whichClass,
            rti1516::AttributeHandle theHandle);

    // 10.6
    virtual rti1516::InteractionClassHandle getInteractionClassHandle
    (std::wstring const & theName);

    // 10.7
    virtual std::wstring getInteractionClassName
    (rti1516::InteractionClassHandle theHandle);

    // 10.8
    virtual rti1516::ParameterHandle getParameterHandle
    (rti1516::InteractionClassHandle whichClass,
            std::wstring const & theName);

    // 10.9
    virtual std::wstring getParameterName
    (rti1516::InteractionClassHandle whichClass,
            rti1516::ParameterHandle theHandle);

    // 10.10
    virtual rti1516::ObjectInstanceHandle getObjectInstanceHandle
    (std::wstring const & theName);

    // 10.11
    virtual std::wstring getObjectInstanceName
    (rti1516::ObjectInstanceHandle theHandle);

    // 10.12
    virtual rti1516::DimensionHandle getDimensionHandle
    (std::wstring const & theName);

    // 10.13
    virtual std::wstring getDimensionName
    (rti1516::DimensionHandle theHandle);

    // 10.14
    virtual unsigned long getDimensionUpperBound
    (rti1516::DimensionHandle theHandle);

    // 10.15
    virtual rti1516::DimensionHandleSet getAvailableDimensionsForClassAttribute
    (rti1516::ObjectClassHandle theClass,
            rti1516::AttributeHandle theHandle);

    // 10.16
    virtual rti1516::ObjectClassHandle getKnownObjectClassHandle
    (rti1516::ObjectInstanceHandle theObject);

    // 10.17
    virtual rti1516::DimensionHandleSet getAvailableDimensionsForInteractionClass
    (rti1516::InteractionClassHandle theClass);

    // 10.18
    virtual rti1516::TransportationType getTransportationType
    (std::wstring const & transportationName);

    // 10.19
    virtual std::wstring getTransportationName
    (rti1516::TransportationType transportationType);

    // 10.20
    virtual rti1516::OrderType getOrderType
    (std::wstring const & orderName);

    // 10.21
    virtual std::wstring getOrderName
    (rti1516::OrderType orderType);

    // 10.22
    virtual void enableObjectClassRelevanceAdvisorySwitch ();

    // 10.23
    virtual void disableObjectClassRelevanceAdvisorySwitch ();

    // 10.24
    virtual void enableAttributeRelevanceAdvisorySwitch ();

    // 10.25
    virtual void disableAttributeRelevanceAdvisorySwitch ();

    // 10.26
    virtual void enableAttributeScopeAdvisorySwitch ();

    // 10.27
    virtual void disableAttributeScopeAdvisorySwitch ();

    // 10.28
    virtual void enableInteractionRelevanceAdvisorySwitch ();

    // 10.29
    virtual void disableInteractionRelevanceAdvisorySwitch ();

    // 10.30
    virtual
    rti1516::DimensionHandleSet getDimensionHandleSet
    (rti1516::RegionHandle theRegionHandle);

    // 10.31
    virtual
    rti1516::RangeBounds getRangeBounds
    (rti1516::RegionHandle theRegionHandle,
            rti1516::DimensionHandle theDimensionHandle);

    // 10.32
    virtual void setRangeBounds
    (rti1516::RegionHandle theRegionHandle,
            rti1516::DimensionHandle theDimensionHandle,
            rti1516::RangeBounds const & theRangeBounds);

    // 10.33
    virtual unsigned long normalizeFederateHandle
    (rti1516::FederateHandle theFederateHandle);

    // 10.34
    virtual unsigned long normalizeServiceGroup
    (rti1516::ServiceGroupIndicator theServiceGroup);

    // 10.37
    virtual bool evokeCallback(double approximateMinimumTimeInSeconds);

    // 10.38
    virtual bool evokeMultipleCallbacks(double approximateMinimumTimeInSeconds,
            double approximateMaximumTimeInSeconds);

    // 10.39
    virtual void enableCallbacks ();

    // 10.40
    virtual void disableCallbacks ();

    virtual rti1516::FederateHandle decodeFederateHandle(
            rti1516::VariableLengthData const & encodedValue) const;

    virtual rti1516::ObjectClassHandle decodeObjectClassHandle(
            rti1516::VariableLengthData const & encodedValue) const;

    virtual rti1516::InteractionClassHandle decodeInteractionClassHandle(
            rti1516::VariableLengthData const & encodedValue) const;

    virtual rti1516::ObjectInstanceHandle decodeObjectInstanceHandle(
            rti1516::VariableLengthData const & encodedValue) const;

    virtual rti1516::AttributeHandle decodeAttributeHandle(
            rti1516::VariableLengthData const & encodedValue) const;

    virtual rti1516::ParameterHandle decodeParameterHandle(
            rti1516::VariableLengthData const & encodedValue) const;

    virtual rti1516::DimensionHandle decodeDimensionHandle(
            rti1516::VariableLengthData const & encodedValue) const;

    virtual rti1516::MessageRetractionHandle decodeMessageRetractionHandle(
            rti1516::VariableLengthData const & encodedValue) const;

    virtual rti1516::RegionHandle decodeRegionHandle(
            rti1516::VariableLengthData const & encodedValue) const;

};
}

#endif // RTI_RTI1516ambassador_h
