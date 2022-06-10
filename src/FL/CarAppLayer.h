//
// Copyright (C) 2006-2011 Christoph Sommer <christoph.sommer@uibk.ac.at>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

//#pragma once



#ifndef FL_CARAPPLAYER_H_
#define FL_CARAPPLAYER_H_

#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"
#include <zmq.hpp>
#include <sstream>  // Check if really needed
#include <zhelpers.hpp>  // has been copied from https://github.com/booksbyus/zguide/blob/master/examples/C%2B%2B/zhelpers.hpp

#include "PythonConnection.h"
#include "TransmissionData.h"


namespace veins {

class VEINS_API CarAppLayer : public DemoBaseApplLayer {
public:
    void initialize(int stage) override;
    int idx_shift = 0;

protected:
    std::vector <std::string> requestComplete;
    std::vector <std::string> replyComplete;

    simtime_t lastDroveAt;
    bool sentMessage;
    int currentSubscribedServiceId;
    std::string transmissionType;
    TraCIDemo11pMessage* timer;
    TraCIDemo11pMessage* NNParsMsg; // Message to transmit the NN Pars between the cars
    PythonConnection* pyCon;
    TransmissionData* transmData;

protected:
    void onWSM(BaseFrame1609_4* wsm) override;
    //void onWSA(DemoServiceAdvertisment* wsa) override;

    void handleSelfMsg(cMessage* msg) override;
    void handlePositionUpdate(cObject* obj) override;
    void sendMessage(BaseFrame1609_4* frame, std::string transmissionType);
    void receiveMessage(BaseFrame1609_4* frame, std::string transmissionType);
    void saveMsg(cMessage* msg);

private:
    //simsignal_t delaySignal;
    cOutVector delayVector;
    cOutVector asyncDelayVector;
    zmq::context_t context = zmq::context_t(1);
    zmq::socket_t socket = zmq::socket_t(context, ZMQ_REP);
    bool noMsgScheduled = true; // if no message is scheduled, will send "END" to Python and conclude the simulation
    std::vector<std::string> reqst;

};
} // namespace veins
#endif /* FL_CARAPPLAYER_H_ */
