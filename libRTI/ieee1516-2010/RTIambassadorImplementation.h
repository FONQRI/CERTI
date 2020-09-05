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

#include <certi.hh>

namespace certi {
class RTI_EXPORT RTI1516ambassador : rti1516e::RTIambassador {
public:
    virtual ~RTI1516ambassador();

    // 4.2
    virtual void connect(rti1516e::FederateAmbassador& federateAmbassador,
                         rti1516e::CallbackModel theCallbackModel,
                         std::wstring const& localSettingsDesignator);

    // 4.3
    virtual void disconnect();
    // 4.5
    virtual void createFederationExecution(std::wstring const& federationExecutionName,
                                           std::wstring const& fomModule,
                                           std::wstring const& logicalTimeImplementationName
                                           = L"");

    virtual void createFederationExecution(std::wstring const& federationExecutionName,
                                           std::vector<std::wstring> const& fomModules,
                                           std::wstring const& logicalTimeImplementationName
                                           = L"");

    virtual void createFederationExecutionWithMIM(std::wstring const& federationExecutionName,
                                                  std::vector<std::wstring> const& fomModules,
                                                  std::wstring const& mimModule,
                                                  std::wstring const& logicalTimeImplementationName
                                                  = L"");

    // 4.3
    virtual void destroyFederationExecution(std::wstring const& federationExecutionName);

    virtual void listFederationExecutions();

    // 4.9
    virtual rti1516e::FederateHandle joinFederationExecution(
        std::wstring const& federateType,
        std::wstring const& federationExecutionName,
        std::vector<std::wstring> const& additionalFomModules);

    virtual rti1516e::FederateHandle joinFederationExecution(
        std::wstring const& federateName,
        std::wstring const& federateType,
        std::wstring const& federationExecutionName,
        std::vector<std::wstring> const& additionalFomModules);

    // 4.10
    virtual void
    resignFederationExecution(rti1516e::ResignAction resignAction);

    // 4.11
    virtual void registerFederationSynchronizationPoint(
        std::wstring const& label,
        rti1516e::VariableLengthData const& theUserSuppliedTag);

    virtual void registerFederationSynchronizationPoint(
        std::wstring const& label,
        rti1516e::VariableLengthData const& theUserSuppliedTag,
        rti1516e::FederateHandleSet const& syncSet);

    // 4.14
    virtual void synchronizationPointAchieved(std::wstring const& label,
                                              bool successfully);

    // 4.16
    virtual void requestFederationSave(std::wstring const& label);

    virtual void requestFederationSave(std::wstring const& label,
                                       rti1516e::LogicalTime const& theTime);

    // 4.18
    virtual void federateSaveBegun();

    // 4.19
    virtual void federateSaveComplete();

    virtual void federateSaveNotComplete();

    // 4.21
    virtual void abortFederationSave();

    // 4.22
    virtual void queryFederationSaveStatus();

    // 4.24
    virtual void requestFederationRestore(std::wstring const& label);

    // 4.28
    virtual void federateRestoreComplete();

    virtual void federateRestoreNotComplete();
    // 4.30
    virtual void abortFederationRestore();
    // 4.31
    virtual void queryFederationRestoreStatus();

    /////////////////////////////////////
    // Declaration Management Services //
    /////////////////////////////////////

    // 5.2
    virtual void publishObjectClassAttributes(
        rti1516e::ObjectClassHandle theClass,
        rti1516e::AttributeHandleSet const& attributeList);

    // 5.3
    virtual void unpublishObjectClass(rti1516e::ObjectClassHandle theClass);

    virtual void unpublishObjectClassAttributes(
        rti1516e::ObjectClassHandle theClass,
        rti1516e::AttributeHandleSet const& attributeList);

    // 5.4
    virtual void
    publishInteractionClass(rti1516e::InteractionClassHandle theInteraction);

    // 5.5
    virtual void unpublishInteractionClass(rti1516e::InteractionClassHandle theInteraction);

    // 5.6
    virtual void
    subscribeObjectClassAttributes(rti1516e::ObjectClassHandle theClass,
                                   rti1516e::AttributeHandleSet const& attributeList,
                                   bool active,
                                   std::wstring const& updateRateDesignator);

    // 5.7
    virtual void
    unsubscribeObjectClass(rti1516e::ObjectClassHandle theClass);

    virtual void unsubscribeObjectClassAttributes(
        rti1516e::ObjectClassHandle theClass,
        rti1516e::AttributeHandleSet const& attributeList);

    // 5.8
    virtual void subscribeInteractionClass(rti1516e::InteractionClassHandle theClass,
                                           bool active
                                           = true);

    // 5.9
    virtual void
    unsubscribeInteractionClass(rti1516e::InteractionClassHandle theClass);

    ////////////////////////////////
    // Object Management Services //
    ////////////////////////////////

    // 6.2
    virtual void
    reserveObjectInstanceName(std::wstring const& theObjectInstanceName);

    // 6.4
    virtual void
    releaseObjectInstanceName(std::wstring const& theObjectInstanceName);

    // 6.5
    virtual void reserveMultipleObjectInstanceName(std::set<std::wstring> const& theObjectInstanceNames) ;

    // 6.7
    virtual void releaseMultipleObjectInstanceName(std::set<std::wstring> const& theObjectInstanceNames) ;

    // 6.8
    virtual rti1516e::ObjectInstanceHandle
    registerObjectInstance(rti1516e::ObjectClassHandle theClass);

    virtual rti1516e::ObjectInstanceHandle
    registerObjectInstance(rti1516e::ObjectClassHandle theClass,
                           std::wstring const& theObjectInstanceName);

    // 6.10
    virtual void updateAttributeValues(
        rti1516e::ObjectInstanceHandle theObject,
        rti1516e::AttributeHandleValueMap const& theAttributeValues,
        rti1516e::VariableLengthData const& theUserSuppliedTag);

    virtual rti1516e::MessageRetractionHandle
    updateAttributeValues(rti1516e::ObjectInstanceHandle theObject,
                          rti1516e::AttributeHandleValueMap const& theAttributeValues,
                          rti1516e::VariableLengthData const& theUserSuppliedTag,
                          rti1516e::LogicalTime const& theTime);

    // 6.12
    virtual void sendInteraction(
        rti1516e::InteractionClassHandle theInteraction,
        rti1516e::ParameterHandleValueMap const& theParameterValues,
        rti1516e::VariableLengthData const& theUserSuppliedTag);

    virtual rti1516e::MessageRetractionHandle
    sendInteraction(rti1516e::InteractionClassHandle theInteraction,
                    rti1516e::ParameterHandleValueMap const& theParameterValues,
                    rti1516e::VariableLengthData const& theUserSuppliedTag,
                    rti1516e::LogicalTime const& theTime);

    // 6.14
    virtual void deleteObjectInstance(
        rti1516e::ObjectInstanceHandle theObject,
        rti1516e::VariableLengthData const& theUserSuppliedTag);

    virtual rti1516e::MessageRetractionHandle
    deleteObjectInstance(rti1516e::ObjectInstanceHandle theObject,
                         rti1516e::VariableLengthData const& theUserSuppliedTag,
                         rti1516e::LogicalTime const& theTime);

    // 6.16
    virtual void
    localDeleteObjectInstance(rti1516e::ObjectInstanceHandle theObject);

    // 6.19
    virtual void requestAttributeValueUpdate(
        rti1516e::ObjectInstanceHandle theObject,
        rti1516e::AttributeHandleSet const& theAttributes,
        rti1516e::VariableLengthData const& theUserSuppliedTag);

    virtual void requestAttributeValueUpdate(
        rti1516e::ObjectClassHandle theClass,
        rti1516e::AttributeHandleSet const& theAttributes,
        rti1516e::VariableLengthData const& theUserSuppliedTag);

    // 6.23
    virtual void requestAttributeTransportationTypeChange(
        rti1516e::ObjectInstanceHandle theObject,
        rti1516e::AttributeHandleSet const& theAttributes,
        rti1516e::TransportationType theType);

    // 6.25
    virtual void
    queryAttributeTransportationType(rti1516e::ObjectInstanceHandle theObject,
                                     rti1516e::AttributeHandle theAttribute);

    // 6.27
    virtual void requestInteractionTransportationTypeChange(
        rti1516e::InteractionClassHandle theClass,
        rti1516e::TransportationType theType);

    // 6.29
    virtual void queryInteractionTransportationType(
        rti1516e::FederateHandle theFederate,
        rti1516e::InteractionClassHandle theInteraction);

    ///////////////////////////////////
    // Ownership Management Services //
    ///////////////////////////////////
    // 7.2
    virtual void unconditionalAttributeOwnershipDivestiture(
        rti1516e::ObjectInstanceHandle theObject,
        rti1516e::AttributeHandleSet const& theAttributes);

    // 7.3
    virtual void negotiatedAttributeOwnershipDivestiture(
        rti1516e::ObjectInstanceHandle theObject,
        rti1516e::AttributeHandleSet const& theAttributes,
        rti1516e::VariableLengthData const& theUserSuppliedTag);

    // 7.6
    virtual void confirmDivestiture(
        rti1516e::ObjectInstanceHandle theObject,
        rti1516e::AttributeHandleSet const& confirmedAttributes,
        rti1516e::VariableLengthData const& theUserSuppliedTag);

    // 7.8
    virtual void attributeOwnershipAcquisition(
        rti1516e::ObjectInstanceHandle theObject,
        rti1516e::AttributeHandleSet const& desiredAttributes,
        rti1516e::VariableLengthData const& theUserSuppliedTag);

    // 7.9
    virtual void attributeOwnershipAcquisitionIfAvailable(
        rti1516e::ObjectInstanceHandle theObject,
        rti1516e::AttributeHandleSet const& desiredAttributes);

    // 7.12
    virtual void attributeOwnershipReleaseDenied(
        rti1516e::ObjectInstanceHandle theObject,
        rti1516e::AttributeHandleSet const& theAttributes);

    // 7.13
    virtual void
    attributeOwnershipDivestitureIfWanted(rti1516e::ObjectInstanceHandle theObject,
                                          rti1516e::AttributeHandleSet const& theAttributes,
                                          rti1516e::AttributeHandleSet& theDivestedAttributes); // filled by RTI

    // 7.14
    virtual void cancelNegotiatedAttributeOwnershipDivestiture(
        rti1516e::ObjectInstanceHandle theObject,
        rti1516e::AttributeHandleSet const& theAttributes);

    // 7.15
    virtual void cancelAttributeOwnershipAcquisition(
        rti1516e::ObjectInstanceHandle theObject,
        rti1516e::AttributeHandleSet const& theAttributes);

    // 7.17
    virtual void
    queryAttributeOwnership(rti1516e::ObjectInstanceHandle theObject,
                            rti1516e::AttributeHandle theAttribute);

    // 7.18
    virtual bool
    isAttributeOwnedByFederate(rti1516e::ObjectInstanceHandle theObject,
                               rti1516e::AttributeHandle theAttribute);

    //////////////////////////////
    // Time Management Services //
    //////////////////////////////

    // 8.2
    virtual void enableTimeRegulation(rti1516e::LogicalTimeInterval const& theLookahead) ;

    // 8.4
    virtual void disableTimeRegulation();

    // 8.5
    virtual void enableTimeConstrained();

    // 8.7
    virtual void disableTimeConstrained();

    // 8.8
    virtual void
    timeAdvanceRequest(rti1516e::LogicalTime const& theTime);

    // 8.9
    virtual void
    timeAdvanceRequestAvailable(rti1516e::LogicalTime const& theTime);

    // 8.10
    virtual void
    nextMessageRequest(rti1516e::LogicalTime const& theTime);

    // 8.11
    virtual void
    nextMessageRequestAvailable(rti1516e::LogicalTime const& theTime);

    // 8.12
    virtual void
    flushQueueRequest(rti1516e::LogicalTime const& theTime);

    // 8.14
    virtual void enableAsynchronousDelivery();

    // 8.15
    virtual void disableAsynchronousDelivery();

    // 8.16
    virtual bool queryGALT(rti1516e::LogicalTime& theTime);

    // 8.17
    virtual void queryLogicalTime(rti1516e::LogicalTime& theTime);

    // 8.18
    virtual bool queryLITS(rti1516e::LogicalTime& theTime);

    // 8.19
    virtual void
    modifyLookahead(rti1516e::LogicalTimeInterval const& theLookahead);

    // 8.20
    virtual void queryLookahead(rti1516e::LogicalTimeInterval& interval);

    // 8.21
    virtual void retract(rti1516e::MessageRetractionHandle theHandle);

    // 8.23
    virtual void changeAttributeOrderType(rti1516e::ObjectInstanceHandle theObject,
                                          rti1516e::AttributeHandleSet const& theAttributes,
                                          rti1516e::OrderType theType);

    // 8.24
    virtual void changeInteractionOrderType(rti1516e::InteractionClassHandle theClass,
                                            rti1516e::OrderType theType);

    //////////////////////////////////
    // Data Distribution Management //
    //////////////////////////////////

    // 9.2
    virtual rti1516e::RegionHandle
    createRegion(rti1516e::DimensionHandleSet const& theDimensions);

    // 9.3
    virtual void commitRegionModifications(rti1516e::RegionHandleSet const& theRegionHandleSet);

    // 9.4
    virtual void
    deleteRegion(rti1516e::RegionHandle const& theRegion);

    // 9.5
    virtual rti1516e::ObjectInstanceHandle registerObjectInstanceWithRegions(
        rti1516e::ObjectClassHandle theClass,
        rti1516e::AttributeHandleSetRegionHandleSetPairVector const&
            theAttributeHandleSetRegionHandleSetPairVector);

    virtual rti1516e::ObjectInstanceHandle registerObjectInstanceWithRegions(
        rti1516e::ObjectClassHandle theClass,
        rti1516e::AttributeHandleSetRegionHandleSetPairVector const& theAttributeHandleSetRegionHandleSetPairVector,
        std::wstring const& theObjectInstanceName);

    // 9.6
    virtual void associateRegionsForUpdates(
        rti1516e::ObjectInstanceHandle theObject,
        rti1516e::AttributeHandleSetRegionHandleSetPairVector const&
            theAttributeHandleSetRegionHandleSetPairVector);

    // 9.7
    virtual void unassociateRegionsForUpdates(
        rti1516e::ObjectInstanceHandle theObject,
        rti1516e::AttributeHandleSetRegionHandleSetPairVector const&
            theAttributeHandleSetRegionHandleSetPairVector);

    // 9.8
    virtual void subscribeObjectClassAttributesWithRegions(
        rti1516e::ObjectClassHandle theClass,
        rti1516e::AttributeHandleSetRegionHandleSetPairVector const& theAttributeHandleSetRegionHandleSetPairVector,
        bool active,
        std::wstring const& updateRateDesignator);

    // 9.9
    virtual void unsubscribeObjectClassAttributesWithRegions(
        rti1516e::ObjectClassHandle theClass,
        rti1516e::AttributeHandleSetRegionHandleSetPairVector const&
            theAttributeHandleSetRegionHandleSetPairVector);

    // 9.10
    virtual void
    subscribeInteractionClassWithRegions(rti1516e::InteractionClassHandle theClass,
                                         rti1516e::RegionHandleSet const& theRegionHandleSet,
                                         bool active
                                         = true);

    // 9.11
    virtual void unsubscribeInteractionClassWithRegions(
        rti1516e::InteractionClassHandle theClass,
        rti1516e::RegionHandleSet const& theRegionHandleSet);

    // 9.12
    virtual void sendInteractionWithRegions(
        rti1516e::InteractionClassHandle theInteraction,
        rti1516e::ParameterHandleValueMap const& theParameterValues,
        rti1516e::RegionHandleSet const& theRegionHandleSet,
        rti1516e::VariableLengthData const& theUserSuppliedTag);

    virtual rti1516e::MessageRetractionHandle
    sendInteractionWithRegions(rti1516e::InteractionClassHandle theInteraction,
                               rti1516e::ParameterHandleValueMap const& theParameterValues,
                               rti1516e::RegionHandleSet const& theRegionHandleSet,
                               rti1516e::VariableLengthData const& theUserSuppliedTag,
                               rti1516e::LogicalTime const& theTime);

    // 9.13
    virtual void requestAttributeValueUpdateWithRegions(
        rti1516e::ObjectClassHandle theClass,
        rti1516e::AttributeHandleSetRegionHandleSetPairVector const& theSet,
        rti1516e::VariableLengthData const& theUserSuppliedTag);

    //////////////////////////
    // RTI Support Services //
    //////////////////////////
    // 10.2
    virtual rti1516e::ResignAction getAutomaticResignDirective();

    // 10.3
    virtual void
    setAutomaticResignDirective(rti1516e::ResignAction resignAction);

    // 10.4
    virtual rti1516e::FederateHandle
    getFederateHandle(std::wstring const& theName);

    // 10.5
    virtual std::wstring getFederateName(rti1516e::FederateHandle theHandle);

    // 10.6
    virtual rti1516e::ObjectClassHandle
    getObjectClassHandle(std::wstring const& theName);

    // 10.7
    virtual std::wstring
    getObjectClassName(rti1516e::ObjectClassHandle theHandle);

    // 10.8
    rti1516e::ObjectClassHandle
    getKnownObjectClassHandle(rti1516e::ObjectInstanceHandle theObject);

    // 10.9
    rti1516e::ObjectInstanceHandle
    getObjectInstanceHandle(std::wstring const& theName);

    // 10.10
    std::wstring
    getObjectInstanceName(rti1516e::ObjectInstanceHandle theHandle);

    // 10.11
    virtual rti1516e::AttributeHandle
    getAttributeHandle(rti1516e::ObjectClassHandle whichClass,
                       std::wstring const& theAttributeName);

    // 10.12
    virtual std::wstring
    getAttributeName(rti1516e::ObjectClassHandle whichClass,
                     rti1516e::AttributeHandle theHandle);

    // 10.13
    virtual double
    getUpdateRateValue(std::wstring const& updateRateDesignator);

    // 10.14
    virtual double
    getUpdateRateValueForAttribute(rti1516e::ObjectInstanceHandle theObject,
                                   rti1516e::AttributeHandle theAttribute);

    // 10.15
    virtual rti1516e::InteractionClassHandle
    getInteractionClassHandle(std::wstring const& theName);

    // 10.16
    virtual std::wstring
    getInteractionClassName(rti1516e::InteractionClassHandle theHandle);

    // 10.17
    virtual rti1516e::ParameterHandle
    getParameterHandle(rti1516e::InteractionClassHandle whichClass,
                       std::wstring const& theName);

    // 10.18
    virtual std::wstring
    getParameterName(rti1516e::InteractionClassHandle whichClass,
                     rti1516e::ParameterHandle theHandle);

    // 10.20
    virtual rti1516e::OrderType getOrderType(std::wstring const& orderName);

    // 10.21
    virtual std::wstring getOrderName(rti1516e::OrderType orderType);

    // 10.18
    virtual rti1516e::TransportationType
    getTransportationType(std::wstring const& transportationName);

    // 10.19
    virtual std::wstring
    getTransportationName(rti1516e::TransportationType transportationType);

    // 10.15
    virtual rti1516e::DimensionHandleSet getAvailableDimensionsForClassAttribute(
        rti1516e::ObjectClassHandle theClass,
        rti1516e::AttributeHandle theHandle);

    // 10.17
    virtual rti1516e::DimensionHandleSet getAvailableDimensionsForInteractionClass(
        rti1516e::InteractionClassHandle theClass);

    // 10.12
    virtual rti1516e::DimensionHandle
    getDimensionHandle(std::wstring const& theName);

    // 10.13
    virtual std::wstring
    getDimensionName(rti1516e::DimensionHandle theHandle);

    // 10.14
    virtual unsigned long
    getDimensionUpperBound(rti1516e::DimensionHandle theHandle);

    // 10.30
    virtual rti1516e::DimensionHandleSet
    getDimensionHandleSet(rti1516e::RegionHandle theRegionHandle);

    // 10.31
    virtual rti1516e::RangeBounds
    getRangeBounds(rti1516e::RegionHandle theRegionHandle,
                   rti1516e::DimensionHandle theDimensionHandle);

    // 10.32
    virtual void
    setRangeBounds(rti1516e::RegionHandle theRegionHandle,
                   rti1516e::DimensionHandle theDimensionHandle,
                   rti1516e::RangeBounds const& theRangeBounds);

    // 10.33
    virtual unsigned long
    normalizeFederateHandle(rti1516e::FederateHandle theFederateHandle);

    // 10.34
    virtual unsigned long
    normalizeServiceGroup(rti1516e::ServiceGroup theServiceGroup);

    // 10.22
    virtual void enableObjectClassRelevanceAdvisorySwitch();

    // 10.23
    virtual void disableObjectClassRelevanceAdvisorySwitch();

    // 10.24
    virtual void enableAttributeRelevanceAdvisorySwitch();

    // 10.25
    virtual void disableAttributeRelevanceAdvisorySwitch();

    // 10.26
    virtual void enableAttributeScopeAdvisorySwitch();

    // 10.27
    virtual void disableAttributeScopeAdvisorySwitch();

    // 10.28
    virtual void enableInteractionRelevanceAdvisorySwitch();

    // 10.29
    virtual void disableInteractionRelevanceAdvisorySwitch();

    // 10.41
    virtual bool evokeCallback(double approximateMinimumTimeInSeconds);

    // 10.38
    virtual bool
    evokeMultipleCallbacks(double approximateMinimumTimeInSeconds,
                           double approximateMaximumTimeInSeconds);

    // 10.39
    virtual void
    enableCallbacks();

    // 10.40
    virtual void
    disableCallbacks();

    // Return instance of time factory being used by the federation
    virtual std::auto_ptr<rti1516e::LogicalTimeFactory> getTimeFactory() const;

    virtual rti1516e::FederateHandle decodeFederateHandle(rti1516e::VariableLengthData const& encodedValue) const;

    virtual rti1516e::ObjectClassHandle decodeObjectClassHandle(rti1516e::VariableLengthData const& encodedValue) const;

    virtual rti1516e::InteractionClassHandle
    decodeInteractionClassHandle(rti1516e::VariableLengthData const& encodedValue) const;

    virtual rti1516e::ObjectInstanceHandle
    decodeObjectInstanceHandle(rti1516e::VariableLengthData const& encodedValue) const;

    virtual rti1516e::AttributeHandle decodeAttributeHandle(rti1516e::VariableLengthData const& encodedValue) const;

    virtual rti1516e::ParameterHandle decodeParameterHandle(rti1516e::VariableLengthData const& encodedValue) const;

    virtual rti1516e::DimensionHandle decodeDimensionHandle(rti1516e::VariableLengthData const& encodedValue) const;

    virtual rti1516e::MessageRetractionHandle
    decodeMessageRetractionHandle(rti1516e::VariableLengthData const& encodedValue) const;

    virtual rti1516e::RegionHandle decodeRegionHandle(rti1516e::VariableLengthData const& encodedValue) const;

protected:
    RTI1516ambassador() noexcept;

private:
    struct Private;
    std::auto_ptr<Private> p{nullptr};

    // Helper functions
    template <typename T>
    void assignAHSAndExecuteService(const rti1516e::AttributeHandleSet& AHS, T& req, T& rep);
    template <typename T>
    void assignPHVMAndExecuteService(const rti1516e::ParameterHandleValueMap& PHVM, T& req, T& rep);
    template <typename T>
    void assignAHVMAndExecuteService(const rti1516e::AttributeHandleValueMap& AHVM, T& req, T& rep);
    // Helper function for CallBacks

    /** Generic callback evocation (CERTI extension).
    *  Blocks up to "minimum" seconds until a callback delivery and then evokes a
    *  single callback.
    *  @return true if additional callbacks pending, false otherwise
    */
    bool __tick_kernel(bool multiple, TickTime minimum, TickTime maximum);

    friend std::auto_ptr<rti1516e::RTIambassador>
    rti1516e::RTIambassadorFactory::createRTIambassador();
};
}

#endif // RTI_RTI1516ambassador_h
