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

#include "inet/linklayer/ieee8021ae/Ieee8021aeTagEpdHeaderChecker.h"

#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/EtherType_m.h"
#include "inet/linklayer/common/UserPriorityTag_m.h"
#include "inet/linklayer/common/VlanTag_m.h"
#include "inet/linklayer/ieee8021ae/Ieee8021aeTagHeader_m.h"

namespace inet {

Define_Module(Ieee8021aeTagEpdHeaderChecker);

void Ieee8021aeTagEpdHeaderChecker::processPacket(Packet *packet)
{
    // TODO this code is incomplete
    const auto& header = packet->popAtFront<Ieee8021aeTagEpdHeader>();
    auto typeOrLength = header->getTypeOrLength();
    const Protocol *protocol;
    if (isIeee8023Length(typeOrLength))
        protocol = &Protocol::ieee8022llc;
    else
        protocol = ProtocolGroup::ethertype.getProtocol(typeOrLength);
    auto packetProtocolTag = packet->addTagIfAbsent<PacketProtocolTag>();
    packetProtocolTag->setProtocol(protocol);
    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(protocol);
}

void Ieee8021aeTagEpdHeaderChecker::dropPacket(Packet *packet)
{
    EV_WARN << "Dropping packet because protocol is not found" << EV_FIELD(packet) << EV_ENDL;
    PacketFilterBase::dropPacket(packet, NO_PROTOCOL_FOUND);
}

bool Ieee8021aeTagEpdHeaderChecker::matchesPacket(const Packet *packet) const
{
    const auto& header = packet->peekAtFront<Ieee8021aeTagEpdHeader>();
    auto typeOrLength = header->getTypeOrLength();
    if (!isIeee8023Length(typeOrLength) && ProtocolGroup::ethertype.findProtocol(typeOrLength) == nullptr)
        return false;
    else
        return true;
}

} // namespace inet

