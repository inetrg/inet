//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004-2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


#include "IPvXTrafGen.h"

#include "ModuleAccess.h"
#include "NodeOperations.h"
#include "AddressResolver.h"
#include "IPSocket.h"
#include "IPv4ControlInfo.h"
#include "IPv6ControlInfo.h"


Define_Module(IPvXTrafGen);

simsignal_t IPvXTrafGen::sentPkSignal = SIMSIGNAL_NULL;

IPvXTrafGen::IPvXTrafGen()
{
    timer = NULL;
    nodeStatus = NULL;
    packetLengthPar = NULL;
    sendIntervalPar = NULL;
}

IPvXTrafGen::~IPvXTrafGen()
{
    cancelAndDelete(timer);
}

void IPvXTrafGen::initialize(int stage)
{
    IPvXTrafSink::initialize(stage);
    // because of IPvXAddressResolver, we need to wait until interfaces are registered,
    // address auto-assignment takes place etc.
    if (stage != 3)
        return;

    sentPkSignal = registerSignal("sentPk");

    protocol = par("protocol");
    numPackets = par("numPackets");
    startTime = par("startTime");
    stopTime = par("stopTime");
    if (stopTime >= SIMTIME_ZERO && stopTime < startTime)
        error("Invalid startTime/stopTime parameters");

    packetLengthPar = &par("packetLength");
    sendIntervalPar = &par("sendInterval");

    numSent = 0;
    WATCH(numSent);

    timer = new cMessage("sendTimer");
    nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));

    IPSocket ipSocket(gate("ipOut"));
    ipSocket.registerProtocol(protocol);
    IPSocket ipv6Socket(gate("ipv6Out"));
    ipv6Socket.registerProtocol(protocol);

    if (isNodeUp() && isEnabled())
        scheduleNextPacket(-1);
}

void IPvXTrafGen::handleMessage(cMessage *msg)
{
    if (!isNodeUp())
        throw cRuntimeError("Application is not running");
    if (msg == timer)
    {
        if (msg->getKind() == START)
        {
            destAddresses.clear();
            const char *destAddrs = par("destAddresses");
            cStringTokenizer tokenizer(destAddrs);
            const char *token;
            while ((token = tokenizer.nextToken()) != NULL)
            {
                Address result;
                AddressResolver().tryResolve(token, result);
                if (result.isUnspecified())
                    EV << "cannot resolve destination address: " << token << endl;
                else
                    destAddresses.push_back(result);
            }
        }
        if (!destAddresses.empty())
        {
            sendPacket();
            if (isEnabled())
                scheduleNextPacket(simTime());
        }
    }
    else
        processPacket(PK(msg));

    if (ev.isGUI())
    {
        char buf[40];
        sprintf(buf, "rcvd: %d pks\nsent: %d pks", numReceived, numSent);
        getDisplayString().setTagArg("t", 0, buf);
    }
}

bool IPvXTrafGen::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if (stage == NodeStartOperation::STAGE_APPLICATION_LAYER && isEnabled())
            scheduleNextPacket(-1);
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if (stage == NodeShutdownOperation::STAGE_APPLICATION_LAYER)
            cancelNextPacket();
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if (stage == NodeCrashOperation::STAGE_CRASH)
            cancelNextPacket();
    }
    else throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName());
    return true;
}

void IPvXTrafGen::scheduleNextPacket(simtime_t previous)
{
    simtime_t next;
    if (previous == -1)
    {
        next = simTime() <= startTime ? startTime : simTime();
        timer->setKind(START);
    }
    else
    {
        next = previous + sendIntervalPar->doubleValue();
        timer->setKind(NEXT);
    }
    if (stopTime < SIMTIME_ZERO || next < stopTime)
        scheduleAt(next, timer);
}

void IPvXTrafGen::cancelNextPacket()
{
    cancelEvent(timer);
}

bool IPvXTrafGen::isNodeUp()
{
    return !nodeStatus || nodeStatus->getState() == NodeStatus::UP;
}

bool IPvXTrafGen::isEnabled()
{
    return (numPackets == -1 || numSent < numPackets);
}

Address IPvXTrafGen::chooseDestAddr()
{
    int k = intrand(destAddresses.size());
    return destAddresses[k];
}

void IPvXTrafGen::sendPacket()
{
    char msgName[32];
    sprintf(msgName, "appData-%d", numSent);

    cPacket *payload = new cPacket(msgName);
    payload->setByteLength(packetLengthPar->longValue());

    Address destAddr = chooseDestAddr();
    const char *gate;

    if (!destAddr.isIPv6())
    {
        // send to IPv4
        IPv4ControlInfo *controlInfo = new IPv4ControlInfo();
        controlInfo->setDestAddr(destAddr.toIPv4());
        controlInfo->setProtocol(protocol);
        payload->setControlInfo(controlInfo);
        gate = "ipOut";
    }
    else
    {
        // send to IPv6
        IPv6ControlInfo *controlInfo = new IPv6ControlInfo();
        controlInfo->setDestAddr(destAddr.toIPv6());
        controlInfo->setProtocol(protocol);
        payload->setControlInfo(controlInfo);
        gate = "ipv6Out";
    }
    EV << "Sending packet: ";
    printPacket(payload);
    emit(sentPkSignal, payload);
    send(payload, gate);
    numSent++;
}
