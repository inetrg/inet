//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
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
/**
 * @author Jan Bloudicek (jbloudicek@gmail.com)
 * @author Vit Rek (rek@kn.vutbr.cz)
 * @author Vladimir Vesely (ivesely@fit.vutbr.cz)
 * @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
 */

#include "inet/routing/eigrp/tables/EigrpInterfaceTable.h"
#include <algorithm>

namespace inet{
namespace eigrp{

Define_Module(EigrpInterfaceTable);

std::ostream& operator<<(std::ostream& out, const EigrpInterface& iface)
{
    out << iface.getInterfaceName() << "(" << iface.getInterfaceId() << ")";
    out << "  Peers:" << iface.getNumOfNeighbors();
    out << "  Passive:";
    if (iface.isPassive()) out << "enabled";
    else out << "disabled";
    //out << "  stubs:" << iface.getNumOfStubs();
    out << "  HelloInt:" << iface.getHelloInt();
    out << "  HoldInt:" << iface.getHoldInt();
    out << "  SplitHorizon:" ;
    if (iface.isSplitHorizonEn()) out << "enabled";
    else out << "disabled";
    /*out << "  bw:" << iface.getBandwidth();
    out << "  dly:" << iface.getDelay();
    out << "  rel:" << iface.getReliability() << "/255";
    out << "  rLoad:" << iface.getLoad() << "/255";
    out << "  pendingMsgs:" << iface.getPendingMsgs();*/
    return out;
}

EigrpInterface::~EigrpInterface()
{
    hellot = nullptr;
}

EigrpInterface::EigrpInterface(InterfaceEntry *iface, int networkId, bool enabled) :
               interfaceId(iface->getInterfaceId()), networkId(networkId), enabled(enabled)
{
    hellot = NULL;
    neighborCount = 0;
    stubCount = 0;
    splitHorizon = true;
    passive = false;
    mtu = iface->getMtu();
    this->setInterfaceDatarate(iface->getDatarate());
    load=1;
    reliability = 255;
    interfaceName = iface->getInterfaceName();
    relMsgs = 0;
    pendingMsgs = 0;

    if (!iface->isMulticast() && bandwidth <= 1544)
    { // Non-broadcast Multi Access interface (no multicast) with bandwidth equal or lower than T1 link
        helloInt = 60;
        holdInt = 180;
    }
    else
    {
        helloInt = 5;
        holdInt = 15;
    }
}

bool EigrpInterface::isMulticastAllowedOnIface(InterfaceEntry *iface)
{
    if (iface->isMulticast())
        if (getNumOfNeighbors() > 1)
            return true;

    return false;
}

void EigrpInterface::setInterfaceDatarate(double datarate){

    interfaceDatarate = datarate;

    switch ((long)datarate)
        {
        case 64000: //56k modem
            bandwidth = 64;
            delay = 20000;
            break;
        case 56000: //56k modem
            bandwidth = 56;
            delay = 20000;
            break;
        case 1544000: //T1
            bandwidth = 1544;
            delay = 20000;
            break;
        case 10000000: //Eth10
            bandwidth = 10000;
            delay = 1000;
            break;
        default: //>Eth10
            bandwidth = 100000;
            delay = 100;
            break;
        }
}


EigrpInterfaceTable::~EigrpInterfaceTable()
{
    int cnt = eigrpInterfaces.size();
    EigrpInterface *iface;

    for (int i = 0; i < cnt; i++)
    {
        iface = eigrpInterfaces[i];

        // Must be there
        //cancelHelloTimer(iface);

        eigrpInterfaces[i] = NULL;
        delete iface;
    }
    eigrpInterfaces.clear();
}

void EigrpInterfaceTable::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage == INITSTAGE_ROUTING_PROTOCOLS)
    {
        WATCH_PTRVECTOR(eigrpInterfaces);
    }
}

void EigrpInterfaceTable::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
}

void EigrpInterfaceTable::cancelHelloTimer(EigrpInterface *iface)
{
    EigrpTimer *timer;

    if ((timer = iface->getHelloTimer()) != NULL)
    {
        if (timer->isScheduled()) {
            cancelEvent(timer);
        }
    }
}

void EigrpInterfaceTable::addInterface(EigrpInterface *interface)
{
    //TODO check duplicity
    eigrpInterfaces.push_back(interface);
}

EigrpInterface *EigrpInterfaceTable::removeInterface(EigrpInterface *iface)
{
    InterfaceVector::iterator it;
    it = std::find(eigrpInterfaces.begin(), eigrpInterfaces.end(), iface);

    if (it != eigrpInterfaces.end())
    {
        eigrpInterfaces.erase(it);
        return iface;
    }

    return NULL;
}

EigrpInterface *EigrpInterfaceTable::findInterfaceById(int ifaceId)
{
    InterfaceVector::iterator it;
    EigrpInterface * iface;

    for (it = eigrpInterfaces.begin(); it != eigrpInterfaces.end(); it++)
    {
        iface = *it;
        if (iface->getInterfaceId() == ifaceId)
        {
            return iface;
        }
    }

    return NULL;
}


} //namespace eigrp
} //namespace inet