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
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_STREAMTHROUGHTRANSMITTER_H
#define __INET_STREAMTHROUGHTRANSMITTER_H

#include "inet/protocol/transceiver/base/PacketTransmitterBase.h"

namespace inet {

class INET_API StreamThroughTransmitter : public PacketTransmitterBase
{
  protected:
    clocktime_t txStartTime = -1;

  protected:
    virtual void handleMessageWhenUp(cMessage *message) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    virtual void startTx(Packet *packet);
    virtual void endTx();

    virtual void scheduleTxEndTimer(Signal *signal);

  public:
    virtual bool supportsPacketStreaming(cGate *gate) const override { return true; }

    virtual bool canPushSomePacket(cGate *gate) const override { return txSignal == nullptr; };
    virtual bool canPushPacket(Packet *packet, cGate *gate) const override { return txSignal == nullptr; };
    virtual void pushPacket(Packet *packet, cGate *gate) override;
    virtual void pushPacketStart(Packet *packet, cGate *gate, bps datarate) override;
    virtual void pushPacketEnd(Packet *packet, cGate *gate) override;
    virtual void pushPacketProgress(Packet *packet, cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override;
    virtual b getPushPacketProcessedLength(Packet *packet, cGate *gate) override;
};

} // namespace inet

#endif // ifndef __INET_STREAMTHROUGHTRANSMITTER_H
