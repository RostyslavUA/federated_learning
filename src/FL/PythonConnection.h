#pragma once

#include <zmq.hpp>
#include <omnetpp.h>
#include "zhelpers.hpp"


class PythonConnection : public omnetpp::cSimpleModule {
public:
    void initialize() override;
    /** Create a DEALER socket for every car. Conventionally, socket's identity is set to the
     *  combination of module name and index i.e., node[0], node[1], ...
     */
    void createCarSocket(std::string);
    /**
     * Obtain a request on a car's socket
     */
    std::vector<std::string>getRequestPy(int);
    void clearRequestPy(void);
    void sendReplyPy(std::vector <std::string>, int);
    void setGotRequest(bool);
    bool getGotRequest(void);
    void setParNr(int);
    int getParNr(void);
    std::string getSockIdentity(void);

private:
    zmq::context_t context = zmq::context_t(1);
    zmq::socket_t socket = zmq::socket_t(context, zmq::socket_type::rep);

    zmq::socket_t socket2 = zmq::socket_t(context, zmq::socket_type::router);  // rename socket2 to socket after DEALER-ROUTER model works
    zmq::socket_t socket3 = zmq::socket_t(context, zmq::socket_type::dealer);  // rename to socket when OMNET ROUTER works
    std::vector<zmq::socket_t> sockets; // Every node is associated with one socket
    std::vector<std::string> requestPy;
    std::string sockIdentity;
    // Number of parameters obtained from Python. 1st Par - sender idx, 2nd - NN Pars, 3rd - dest idx
    // 3-rd idx is unnecessary in case of broadcast and anycast transmission schemes
    int parNr = 2;
    bool gotRequest;
};
