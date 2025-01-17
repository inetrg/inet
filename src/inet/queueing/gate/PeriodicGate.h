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

#ifndef __INET_PERIODICGATE_H
#define __INET_PERIODICGATE_H

#include "inet/common/clock/ClockUserModuleMixin.h"
#include "inet/queueing/base/PacketGateBase.h"

namespace inet {
namespace queueing {

class INET_API PeriodicGate : public ClockUserModuleMixin<PacketGateBase>
{
  protected:
    int index = 0;
    clocktime_t offset;
    cValueArray *durations = nullptr;

    ClockEvent *changeTimer = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;
    virtual void handleParameterChange(const char *name) override;
    virtual bool canPacketFlowThrough(Packet *packet) const override;

    virtual void initializeGating();
    virtual void scheduleChangeTimer();
    virtual void processChangeTimer();

  public:
    virtual ~PeriodicGate() { cancelAndDelete(changeTimer); }

    virtual bool getInitiallyOpen() const { return par("initiallyOpen"); }
    virtual const cValueArray *getDurations() const { return durations; }
};

} // namespace queueing
} // namespace inet

#endif

