//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_NETWORKINTERFACE_H
#define __INET_NETWORKINTERFACE_H

#include <vector>
#include "inet/common/INETDefs.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/packet/tag/TagSet.h"
#include "inet/common/Protocol.h"
#include "inet/common/Simsignals.h"
#include "inet/common/TagBase.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/networklayer/common/InterfaceToken.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/queueing/contract/IPacketProcessor.h"
#include "inet/queueing/contract/IPassivePacketSink.h"

namespace inet {

// Forward declarations. Do NOT #include the corresponding header files
// since that would create dependence on Ipv4 and Ipv6 stuff!
class NetworkInterface;
class IInterfaceTable;

enum McastSourceFilterMode { MCAST_INCLUDE_SOURCES, MCAST_EXCLUDE_SOURCES };

class INET_API MacEstimateCostProcess
{
  public:
    virtual ~MacEstimateCostProcess() {};
    virtual double getCost(int, MacAddress&) = 0;
    virtual double getNumCost() = 0;
    virtual int getNumNeighbors() = 0;
    virtual int getNeighbors(MacAddress[]) = 0;
};

/**
 * Base class for protocol-specific data on an interface.
 * Notable subclasses are Ipv4InterfaceData and Ipv6InterfaceData.
 */
class INET_API InterfaceProtocolData : public TagBase
{
    friend class NetworkInterface;    //only this guy is allowed to set ownerp

  protected:
    NetworkInterface *ownerp = nullptr;    // the interface entry this object belongs to
    int id;

  protected:
    // fires notification with the given signalID, and the interface entry as obj
    virtual void changed(simsignal_t signalID, int fieldId);

  public:
    InterfaceProtocolData(int id) : id(id) { }

    /**
     * Returns the NetworkInterface that contains this data object, or nullptr
     */
    NetworkInterface *getNetworkInterface() const { return ownerp; }
};

class INET_API NetworkInterfaceChangeDetails : public cObject
{
    NetworkInterface *ie;
    int field;

  public:
    NetworkInterfaceChangeDetails(NetworkInterface *ie, int field) : ie(ie), field(field) { ASSERT(ie); }
    NetworkInterface *getNetworkInterface() const { return ie; }
    int getFieldId() const { return field; }
    virtual std::string str() const override;
};

/**
 * Interface entry for the interface table in IInterfaceTable.
 *
 * @see IInterfaceTable
 */
class INET_API NetworkInterface : public cSimpleModule, public queueing::IPassivePacketSink, public queueing::IPacketProcessor, public ILifecycle
{
    friend class InterfaceProtocolData;    // to call protocolDataChanged()

  public:
    enum State { UP, DOWN, GOING_UP, GOING_DOWN };

  protected:
    cGate *upperLayerOut = nullptr;
    queueing::IPassivePacketSink *consumer = nullptr;

    const Protocol *protocol = nullptr;
    IInterfaceTable *ownerp = nullptr;    ///< IInterfaceTable that contains this interface, or nullptr
    int interfaceId = -1;    ///< identifies the interface in the IInterfaceTable
    std::string interfaceName;
    int nodeOutputGateId = -1;    ///< id of the output gate of this host/router (or -1 if this is a virtual interface)
    int nodeInputGateId = -1;    ///< id of the input gate of this host/router (or -1 if this is a virtual interface)
    int mtu = 0;    ///< Maximum Transmission Unit (e.g. 1500 on Ethernet); 0 means infinite (i.e. never fragment)
    State state = DOWN;    ///< requested interface state, similar to Linux ifup/ifdown
    bool carrier = false;    ///< current state (up/down) of the physical layer, e.g. Ethernet cable
    bool broadcast = false;    ///< interface supports broadcast
    bool multicast = false;    ///< interface supports multicast
    bool pointToPoint = false;    ///< interface is point-to-point link
    bool loopback = false;    ///< interface is loopback interface
    double datarate = 0;    ///< data rate in bit/s
    MacAddress macAddr;    ///< link-layer address (for now, only IEEE 802 MAC addresses are supported)
    InterfaceToken token;    ///< for Ipv6 stateless autoconfig (RFC 1971), interface identifier (RFC 2462)

    TagSet protocolDataSet;
    std::vector<MacEstimateCostProcess *> estimateCostProcessArray;

  private:
    // copying not supported: following are private and also left undefined
    NetworkInterface(const NetworkInterface& obj);
    NetworkInterface& operator=(const NetworkInterface& obj);

  public:
    // field ids for change notifications
    enum {
        F_CARRIER, F_STATE,
        F_NAME, F_NODE_IN_GATEID, F_NODE_OUT_GATEID, F_NETW_GATEIDX,
        F_LOOPBACK, F_BROADCAST, F_MULTICAST, F_POINTTOPOINT,
        F_DATARATE, F_MTU, F_MACADDRESS, F_TOKEN,
        F_IPV4_DATA, F_IPV6_DATA, F_NEXTHOP_DATA, F_ISIS_DATA, F_TRILL_DATA, F_IEEE8021D_DATA, F_CLNS_DATA
    };

  protected:
    // change notifications
    virtual void configChanged(int fieldId) { changed(interfaceConfigChangedSignal, fieldId); }
    virtual void stateChanged(int fieldId) { changed(interfaceStateChangedSignal, fieldId); }
    virtual void changed(simsignal_t signalID, int fieldId);

    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;
    virtual void updateDisplayString() const;

  public:
    // internal: to be invoked from InterfaceTable only!
    virtual void setInterfaceTable(IInterfaceTable *t) { ownerp = t; }
    virtual void setInterfaceId(int id) { interfaceId = id; }
    virtual void resetInterface();

  protected:
    virtual std::string getFullPath() const override { return cModule::getFullPath(); }
    virtual const char *getName() const override { return cModule::getName(); }

    virtual void arrived(cMessage *message, cGate *gate, const SendOptions& options, simtime_t time) override;

  public:
    NetworkInterface();
    virtual ~NetworkInterface();
    virtual std::string str() const override;
    virtual std::string getInterfaceFullPath() const;

    virtual bool supportsPacketSending(cGate *gate) const override { return true; }
    virtual bool supportsPacketPushing(cGate *gate) const override { return true; }
    virtual bool supportsPacketPulling(cGate *gate) const override { return false; }
    virtual bool supportsPacketPassing(cGate *gate) const override { return true; }
    virtual bool supportsPacketStreaming(cGate *gate) const override { return false; }
    virtual bool canPushSomePacket(cGate *gate) const override { return true; }
    virtual bool canPushPacket(Packet *packet, cGate *gate) const override { return true; }
    virtual void pushPacket(Packet *packet, cGate *gate) override;
    virtual void pushPacketStart(Packet *packet, cGate *gate, bps datarate) override { throw cRuntimeError("Invalid operation"); }
    virtual void pushPacketEnd(Packet *packet, cGate *gate) override { throw cRuntimeError("Invalid operation"); }
    virtual void pushPacketProgress(Packet *packet, cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("Invalid operation"); }
    virtual b getPushPacketProcessedLength(Packet *packet, cGate *gate) override { throw cRuntimeError("Invalid operation"); }

    /**
     * Returns the IInterfaceTable this interface is in, or nullptr
     */
    IInterfaceTable *getInterfaceTable() const { return ownerp; }

    /**
     * Returns the requested state of this interface.
     */
    State getState() const { return state; }

    /**
     * Returns the combined state of the carrier and the interface requested state.
     */
    bool isUp() const { return getState() == UP && hasCarrier(); }

    const ModuleIdAddress getModuleIdAddress() const { return ModuleIdAddress(getId()); }
    const ModulePathAddress getModulePathAddress() const { return ModulePathAddress(getId()); }
    const L3Address getNetworkAddress() const;
    virtual bool hasNetworkAddress(const L3Address& address) const;

    /** @name Field getters. Note they are non-virtual and inline, for performance reasons. */
    //@{
    const Protocol *getProtocol() const { return protocol; }
    int getInterfaceId() const { return interfaceId; }
    const char *getInterfaceName() const { return interfaceName.c_str(); }
    int getNodeOutputGateId() const { return nodeOutputGateId; }
    int getNodeInputGateId() const { return nodeInputGateId; }
    int getMtu() const { return mtu; }
    bool hasCarrier() const { return carrier; }
    bool isBroadcast() const { return broadcast; }
    bool isMulticast() const { return multicast; }
    bool isPointToPoint() const { return pointToPoint; }
    bool isLoopback() const { return loopback; }
    double getDatarate() const { return datarate; }
    const MacAddress& getMacAddress() const { return macAddr; }
    bool matchesMacAddress(const MacAddress& address) const;
    const InterfaceToken& getInterfaceToken() const { return token; }
    //@}

    /** @name Field setters */
    //@{
    virtual void setProtocol(const Protocol *protocol) { this->protocol = protocol; }
    virtual void setInterfaceName(const char *s) { interfaceName = s; configChanged(F_NAME); }
    virtual void setNodeOutputGateId(int i) { if (nodeOutputGateId != i) { nodeOutputGateId = i; configChanged(F_NODE_OUT_GATEID); } }
    virtual void setNodeInputGateId(int i) { if (nodeInputGateId != i) { nodeInputGateId = i; configChanged(F_NODE_IN_GATEID); } }
    virtual void setMtu(int m) { if (mtu != m) { mtu = m; configChanged(F_MTU); } }
    virtual void setState(State s);
    virtual void setCarrier(bool b);
    virtual void setBroadcast(bool b) { if (broadcast != b) { broadcast = b; configChanged(F_BROADCAST); } }
    virtual void setMulticast(bool b) { if (multicast != b) { multicast = b; configChanged(F_MULTICAST); } }
    virtual void setPointToPoint(bool b) { if (pointToPoint != b) { pointToPoint = b; configChanged(F_POINTTOPOINT); } }
    virtual void setLoopback(bool b) { if (loopback != b) { loopback = b; configChanged(F_LOOPBACK); } }
    virtual void setDatarate(double d) { if (datarate != d) { datarate = d; configChanged(F_DATARATE); } }
    virtual void setMacAddress(const MacAddress& addr) { if (macAddr != addr) { macAddr = addr; configChanged(F_MACADDRESS); } }
    virtual void setInterfaceToken(const InterfaceToken& t) { token = t; configChanged(F_TOKEN); }
    //@}

    /** @name Tag related functions */
    //@{
    /**
     * Returns the protocol data at the given index.
     */
    const InterfaceProtocolData* getProtocolData(int index) const {
        return static_cast<const InterfaceProtocolData *>(protocolDataSet.getTag(index));
    }

    /**
     * Clears the set of protocol data objects.
     */
    void clearProtocolDataSet();

    /**
     * Returns the protocol data for the provided type or returns nullptr if no such protocol data is found.
     */
    template<typename T> const T *findProtocolData() const {
        return protocolDataSet.findTag<T>();
    }

    /**
     * Returns the protocol data for the provided type or returns nullptr if no such protocol data is found.
     */
    template<typename T> T *findProtocolDataForUpdate() {
        return protocolDataSet.findTagForUpdate<T>();
    }

    /**
     * Returns the protocol data for the provided type or throws an exception if no such protocol data is found.
     */
    template<typename T> const T *getProtocolData() const {
        return protocolDataSet.getTag<T>();
    }

    /**
     * Returns the protocol data for the provided type or throws an exception if no such protocol data is found.
     */
    template<typename T> T *getProtocolDataForUpdate() {
        return protocolDataSet.getTagForUpdate<T>();
    }

    /**
     * Returns a newly added protocol data for the provided type, or throws an exception if such a protocol data is already present.
     */
    template<typename T> T *addProtocolData() {
        auto t = protocolDataSet.addTag<T>();
        auto ipd = static_cast<InterfaceProtocolData *>(t);
        ipd->ownerp = this;
        changed(interfaceConfigChangedSignal, ipd->id);
        return t;
    }

    /**
     * Returns a newly added protocol data for the provided type if absent, or returns the protocol data that is already present.
     */
    template<typename T> T *addProtocolDataIfAbsent() {
        auto t = protocolDataSet.addTagIfAbsent<T>();
        auto ipd = static_cast<InterfaceProtocolData *>(t);
        if (ipd->ownerp != this) {
            ipd->ownerp = this;
            changed(interfaceConfigChangedSignal, ipd->id);
        }
        return t;
    }

    /**
     * Removes the protocol data for the provided type, or throws an exception if no such protocol data is found.
     */
    template<typename T> T *removeProtocolData() {
        auto t = protocolDataSet.removeTag<T>();
        check_and_cast<InterfaceProtocolData *>(t)->ownerp = nullptr;
        changed(interfaceConfigChangedSignal, t->id);
        return t;
    }

    /**
     * Removes the protocol data for the provided type if present, or returns nullptr if no such protocol data is found.
     */
    template<typename T> T *removeProtocolDataIfPresent() {
        auto t = protocolDataSet.removeTagIfPresent<T>();
        if (t != nullptr) {
            check_and_cast<InterfaceProtocolData *>(t)->ownerp = nullptr;
            changed(interfaceConfigChangedSignal, t->id);
        }
        return t;
    }
    //@}

    /** @name Accessing protocol-specific interface data. Note methods are non-virtual, for performance reasons. */
    //@{
    Ipv4Address getIpv4Address() const;
    Ipv4Address getIpv4Netmask() const;
    //@}

    virtual void joinMulticastGroup(const L3Address& address);
    virtual void changeMulticastGroupMembership(const L3Address& multicastAddress,
            McastSourceFilterMode oldFilterMode, const std::vector<L3Address>& oldSourceList,
            McastSourceFilterMode newFilterMode, const std::vector<L3Address>& newSourceList);

    /** @name access to the cost process estimation  */
    //@{
    virtual bool setEstimateCostProcess(int, MacEstimateCostProcess *p);
    virtual MacEstimateCostProcess *getEstimateCostProcess(int);
    //@}

    // for lifecycle:
    //@{
    virtual bool handleOperationStage(LifecycleOperation *operation, IDoneCallback *doneCallback) override;
    virtual void handleStartOperation(LifecycleOperation *operation);
    virtual void handleStopOperation(LifecycleOperation *operation);
    virtual void handleCrashOperation(LifecycleOperation *operation);
    //@}
};

std::ostream& operator <<(std::ostream& o, NetworkInterface::State);

} // namespace inet

#endif // ifndef __INET_NETWORKINTERFACE_H
