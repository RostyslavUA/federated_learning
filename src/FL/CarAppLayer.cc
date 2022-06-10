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

#include "TraCIDemo11pMessage_m.h"
#include "CarAppLayer.h"

using namespace veins;
using namespace omnetpp;  // tmp

Define_Module(veins::CarAppLayer);

void CarAppLayer::initialize(int stage)
{
    DemoBaseApplLayer::initialize(stage);

    pyCon = FindModule<PythonConnection*>::findGlobalModule();
    ASSERT(pyCon);
    transmData = FindModule<TransmissionData*>::findGlobalModule();
    ASSERT(transmData);

    if (stage == 0) {
        //delaySignal = registerSignal("delay");
        sentMessage = false;
        lastDroveAt = simTime();
        currentSubscribedServiceId = -1;
        transmissionType = "broadcast";  // broadcast, unicast, anycast and multicast are available
        timer = new TraCIDemo11pMessage();
        //initial->setDemoData("Initial");
        scheduleAt(simTime()+45, timer);  // Give some time to initialize. TODO: is there a way to check, when the initialization is over?
        delayVector.setName("Delay");
        asyncDelayVector.setName("Async Sending Time");
        par("NNPars").setStringValue(findHost()->getFullName());
        //Car's socket has the identity of the module name
        pyCon->createCarSocket(findHost()->getFullName());
    }
}

void CarAppLayer::saveMsg(cMessage* msg) {
    TraCIDemo11pMessage* wsm = check_and_cast<TraCIDemo11pMessage*>(msg);
    EV << "Delay is " << simTime().dbl() - wsm->getMessageSendingTime() << "\n";
    delayVector.record(simTime().dbl()-wsm->getMessageSendingTime());
    replyComplete.push_back(wsm->getDemoData());
    // Save the bandwidth consumption in bytes
    replyComplete.push_back(std::to_string(wsm->getByteLength()));
    replyComplete.push_back(std::to_string(simTime().dbl()-wsm->getMessageSendingTime()));
    replyComplete.push_back("STATS"); // Statistics will be returned to Python
    pyCon->sendReplyPy(replyComplete, findHost()->getIndex());
    replyComplete.clear(); // Clean up for the further use
    // Check if any messages are scheduled
    cFutureEventSet* fes = getSimulation()->getFES();
    for (int i=0; i<fes->getLength(); ++i) {
//        EV << "Next event is " << fes->get(i) << "\n";
//        EV << "It is a message " << fes->get(i)->isMessage() << "\n";
        if ((strcmp(fes->get(i)->getClassName(), wsm->getClassName()))==0) {
            noMsgScheduled = false;
        }
    }
    if (noMsgScheduled) {
        // No messages scheduled. Can inform about it Python and conclude the simulation
        std::string end = "END";
        std::vector<std::string> endFrame = {pyCon->getSockIdentity().c_str(), end.c_str()};
        pyCon->sendReplyPy(endFrame, findHost()->getIndex());
        endSimulation();
    }

}

void CarAppLayer::handleSelfMsg(cMessage* msg)
{
    TraCIDemo11pMessage* wsm = check_and_cast<TraCIDemo11pMessage*>(msg);
    EV << "Got message " << wsm->getDemoData() << "at node " << findHost()->getFullName() << "\n";
    // Receive the Python message from the car's socket
    pyCon->sendReplyPy({"UPDATE_REQ"}, findHost()->getIndex()); // Request an update for the next round
    requestComplete = pyCon->getRequestPy(findHost()->getIndex()); // Add an option for multicast, anycast. Remember about the requirement on destination nodes.
    // Throw an error if necessary (see previous implementations above)
    if (strcmp(requestComplete[0].c_str(), "ACK\0") == 0) {
        EV << "Got Python ACK" << '\n';
    }
    else if (strcmp(requestComplete[0].c_str(), "ENDSIM\0") == 0) {
        endSimulation();
    }
    else if (strcmp(requestComplete[0].c_str(), "CON\0") == 0) {
      // Continue without NN Pars
        EV << findHost()->getFullName() << "continuing without NN Pars\n";
    }
    else {
        // Transmit NN Pars
        NNParsMsg = new TraCIDemo11pMessage();
        NNParsMsg->setDemoData(requestComplete[0].c_str());
        NNParsMsg->setSenderIdx(findHost()->getIndex());
        NNParsMsg->setMessageSendingTime(simTime().dbl());
        populateWSM(NNParsMsg);
        // Add the bytelength of the payload
        NNParsMsg->setByteLength(NNParsMsg->getByteLength() + requestComplete.size());
        sendDelayedDown(NNParsMsg, computeAsynchronousSendingTime(1, ChannelType::service)-simTime());
    }
    pyCon->clearRequestPy(); // Empty the container for the upcoming message
    // Get Python updates every 10 sec.
    scheduleAt(simTime()+10, wsm);
}

void CarAppLayer::sendMessage(BaseFrame1609_4* frame, std::string transmissionType) {
    TraCIDemo11pMessage* wsm = check_and_cast<TraCIDemo11pMessage*>(frame);

    for (int i=0; i<transmData->getRequest().size(); i+=transmData->getParNr()) {  // idx 0-src idx, idx 1-NN Pars, idx 2 (if present)-dest idx
        // Got request from Python
        if (findHost()->getIndex()==stoi(transmData->getRequest()[i].substr(1)) && !transmData->getSentReply() && !sentMessage) {
            sentMessage=true;
            wsm->setSenderIdx(stoi(transmData->getRequest()[i].substr(1)));
            // Put the string from Python as a content to the message that will be transmitted between the cars
            wsm->setDemoData(transmData->getRequest()[i+1].c_str());
            wsm->setMessageSendingTime(simTime().dbl());
            if (transmData->getParNr()>2) {
                // If the destination parameter is present
                std::stringstream ss;
                for (size_t j=0; j<transmData->getParNr()-2; ++j) {
                    // Concatenate destination nodes into a string delimited by comma
                    if (j!=0)
                        ss << ",";
                    //EV << "The destination requested by Python is " << pyCon->getRequest()[i+j+2] << "\n";
                    ss << transmData->getRequest()[i+j+2].substr(1);
                }
                // Set destination
                wsm->setDestIdcs(ss.str().c_str());  // TODO: Probably there is a way to directly cast to c_str()
            }

            if (transmissionType=="anycast") {
                // Randomly select a destination node
                int dest = uniform(0, mobility->getManager()->getManagedHosts().size());
                while (dest == findHost()->getIndex()) {
                    // Make sure that SenderIdx!=DestIdx
                    dest = uniform(0, mobility->getManager()->getManagedHosts().size());
                }
                wsm->setDestIdcs(std::to_string(dest).c_str());
            }
            // Set the channel number, user priority, ... and *the bytelength of the header*
            populateWSM(wsm);
            // Add the bytelength of the payload
            wsm->setByteLength(wsm->getByteLength() + transmData->getRequest()[i+1].size());
            EV <<"Sending Delayed Down " << findHost()->getFullName() << " This message " << wsm->getDemoData()<<"\n";

            // Broadcasting
            // asyncDelayVector.record(computeAsynchronousSendingTime(1, ChannelType::service));
            sendDelayedDown(wsm, computeAsynchronousSendingTime(1, ChannelType::service)-simTime());
            //sendDown(wsm);
            // In Veins all messages are broadcasted. The transmission configuration e.g., "unicast" is achieved via
            // filtering the messages at the destination nodes
            transmData->insertSentFrom(std::to_string(findHost()->getIndex()));
            }
        }
}



void CarAppLayer::onWSM(BaseFrame1609_4* frame) {
    // This function is called when the message arrives at the destination node
    TraCIDemo11pMessage* wsm = check_and_cast<TraCIDemo11pMessage*>(frame);

    // Highlight the node that received the message in green color
    findHost()->getDisplayString().setTagArg("i", 1, "green");
    EV << "Got a message at " << findHost()->getFullName() << " Sending to Python \n";
    receiveMessage(frame, transmissionType);
}

void CarAppLayer::receiveMessage(BaseFrame1609_4* frame, std::string transmissionType) {
    TraCIDemo11pMessage* wsm = check_and_cast<TraCIDemo11pMessage*>(frame);
    if(transmissionType=="broadcast" && !(wsm->isSelfMessage() && transmData->getSentReply())){
        //How many nodes have already got the message
        transmData->insertReplied(findHost()->getIndex());
        // Process received message and save it into module TransmissionData that serves as a global variable
        saveMsg(wsm);
        if (transmData->getReplied().size() == mobility->getManager()->getManagedHosts().size()-1){
            // When the messages from some node have already been delivered to all destination nodes
            EV << "Messages from " <<std::to_string(wsm->getSenderIdx())<<" Are successfully delivered\n";
            transmData->insertDeliveredFrom(std::to_string(wsm->getSenderIdx()));
            transmData->clearReplied();
            EV << "Control check. The saved data is \n";
            for (std::vector<std::string>::iterator it = replyComplete.begin(); it!= replyComplete.end(); it++) {
                EV << *it << " ";
            }
        }
    }
    else {
        // anycast, unicast, multicast
        // Decode the string with the dest idcs, delimited by comma
        std::string _destIdcs = wsm->getDestIdcs();
        std::string deltr = ",";  // delimiter
        std::size_t deltrPos = 0;
        std::string destIdx;
        if ((deltrPos = _destIdcs.find(deltr)) != std::string::npos) {
            // if a msg has multiple destinations i.e., multicast
            while((deltrPos = _destIdcs.find(deltr)) != std::string::npos) {
                destIdx = _destIdcs.substr(0, deltrPos);
                if (findHost()->getIndex() == stoi(destIdx)) {
                    saveMsg(wsm);
                }
                _destIdcs.erase(0, deltrPos+deltr.length());
            }
            // duplicate, since _destIdcs contains last dest idx
            if (findHost()->getIndex() == stoi(_destIdcs)) {
                saveMsg(wsm);
            }
        }
        else {
            // if a msg has a single destination i.e., unicast, anycast
            if (findHost()->getIndex() == stoi(_destIdcs)) {
                saveMsg(wsm);
            }
        }
    }
    if(wsm->isSelfMessage()) {
        EV << "Got self message at " << findHost()->getFullName() << "\n";
        cancelAndDelete(wsm);
    }
}

void CarAppLayer::handlePositionUpdate(cObject* obj)
{
    DemoBaseApplLayer::handlePositionUpdate(obj);
}
