//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "inet/protocolelement/transceiver/PacketTransmitter.h"

namespace inet {

Define_Module(PacketTransmitter);

void PacketTransmitter::handleMessageWhenUp(cMessage *message)
{
    if (message == txEndTimer)
        endTx();
    else
        PacketTransmitterBase::handleMessageWhenUp(message);
    updateDisplayString();
}

void PacketTransmitter::handleStopOperation(LifecycleOperation *operation)
{
    ASSERT(!isTransmitting());
}

void PacketTransmitter::handleCrashOperation(LifecycleOperation *operation)
{
    ASSERT(!isTransmitting());
}

void PacketTransmitter::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    startTx(packet);
    updateDisplayString();
}

namespace {
bool canSendOnGate(cGate *gate)
{
    cGate *g;
    for (g = gate; g->getNextGate() != nullptr; g = g->getNextGate()) {
        if (g->getChannel()->isDisabled())
            return false;
    }
    return (g != gate && g->getOwnerModule()->isSimple());
}
}

void PacketTransmitter::startTx(Packet *packet)
{
    // 1. check current state
    ASSERT(!isTransmitting());
    if (!canSendOnGate(outputGate)) {
        // TODO packet dropped signal
        EV_ERROR << "output gate disconnected or channel disabled, packet dropped:" << EV_FORMAT_OBJECT(packet) << EV_ENDL;
        if (producer != nullptr) {
            producer->handlePushPacketProcessed(packet, inputGate->getPathStartGate(), true);
            producer->handleCanPushPacketChanged(inputGate->getPathStartGate());
        }
        delete packet;
        return;
    }
    // 2. store transmission progress
    txDatarate = bps(*dataratePar);
    txStartTime = simTime();
    txStartClockTime = getClockTime();
    // 3. create signal
    auto signal = encodePacket(packet);
    txSignal = signal->dup();
    // 4. send signal start and notify subscribers
    emit(transmissionStartedSignal, signal);
    prepareSignal(signal);
    send(signal, SendOptions().duration(signal->getDuration()), outputGate);
    // 5. schedule transmission end timer
    scheduleTxEndTimer(txSignal);
}

void PacketTransmitter::endTx()
{
    // 1. check current state
    ASSERT(isTransmitting());
    // 2. notify subscribers
    emit(transmissionEndedSignal, txSignal);
    auto packet = check_and_cast<Packet *>(txSignal->decapsulate());
    handlePacketProcessed(packet);
    // 3. clear internal state
    delete txSignal;
    txSignal = nullptr;
    txStartTime = -1;
    txStartClockTime = -1;
    // 4. notify producer
    if (producer != nullptr) {
        producer->handlePushPacketProcessed(packet, inputGate->getPathStartGate(), true);
        producer->handleCanPushPacketChanged(inputGate->getPathStartGate());
    }
    delete packet;
}

void PacketTransmitter::scheduleTxEndTimer(Signal *signal)
{
    scheduleClockEventAfter(SIMTIME_AS_CLOCKTIME(signal->getDuration()), txEndTimer);
}

} // namespace inet

