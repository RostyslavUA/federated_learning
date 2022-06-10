/*
 * PythonConnection.cc
 *
 *  Created on: Feb 6, 2022
 *      Author: ross
 */

//#include "../FL/PythonConnection.h"
#include "PythonConnection.h"
#include <utility>

Define_Module(PythonConnection);

void PythonConnection::initialize()
{
    //EV<< "Initializing PythonConnection \n";
}

void PythonConnection::createCarSocket(std::string ident) {
    std::string host = par("host");
    int port = par("port");
    EV_INFO << "Conecting to client 'tcp://" << host << ":" << port << "'\n";
    sockets.push_back(zmq::socket_t(context, zmq::socket_type::dealer));
    if (!sockets.empty()) {
        // calling back() on empty vector causes undefined behavior
        sockets.back().setsockopt(ZMQ_IDENTITY, ident.c_str(), ident.size());
        sockets.back().connect("tcp://" + host + ":" + std::to_string(port));
        std::string ready = "ready";
        sockets.back().send(zmq::message_t(ready.data(), ready.size()), zmq::send_flags::none);  // Message and its bytelength
    }
    gotRequest=false;
}

std::vector<std::string> PythonConnection::getRequestPy(int socket_idx)
{
    // Receive a message on ROUTER socket
    int ii = 0;  // Prefix is DEALER identity & empty delimiter
    while(true) {
        zmq::message_t request_msg;
        zmq::recv_result_t rc = sockets[socket_idx].recv(request_msg);

        std::string request(static_cast<char*>(request_msg.data()), request_msg.size());
        EV << "Reading Python message " << request.c_str() << "\n";
        std::string end = "END";
        if (strcmp(request.c_str(), end.c_str()) == 0)
            break;
        requestPy.push_back(request);
    }
    // Used to indicate whether the iteration through the unique set of parameters (src, NN Pars, dest) is completed
    bool iterParOver = false;
    // How many unique parameters in the Python message were encountered
    int parNrCount=0;
    for (std::string msg_row: requestPy) {
        if (msg_row.substr(0, 1) == "s") {
            if (!iterParOver) {
                iterParOver=true;
            }
            else {
                setParNr(parNrCount);
                break;
            }

        };
        parNrCount +=1;

    }

    return requestPy;
}

void PythonConnection::clearRequestPy(void) {
    requestPy.clear();
}

void PythonConnection::sendReplyPy(std::vector <std::string> response, int socket_idx)
{
    for (int i=0; i<response.size(); i++) {
        if (response.size()-i==1) {
            sockets[socket_idx].send(zmq::message_t(response[i].data(), response[i].size()), zmq::send_flags::none);

        }
        else {
            sockets[socket_idx].send(zmq::message_t(response[i].data(), response[i].size()), zmq::send_flags::sndmore);
        }
    }
}

void PythonConnection::setGotRequest(bool gotReq) {
    gotRequest = gotReq;
}
bool PythonConnection::getGotRequest(void) {
    return gotRequest;
}

void PythonConnection::setParNr(int _parNr) {
    parNr = _parNr;
}
int PythonConnection::getParNr(void) {
    return parNr;
}

std::string PythonConnection::getSockIdentity(void) {
    return sockIdentity;
}

