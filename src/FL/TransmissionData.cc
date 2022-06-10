/*
 * TransmissionData.cc
 *
 *  Created on: Apr 12, 2022
 *      Author: ross
 */

#include "TransmissionData.h"
#include "TraCIDemo11pMessage_m.h"

Define_Module(TransmissionData);

void TransmissionData::initialize() {
    EV << "TransmissionData, initialized \n";
}

int TransmissionData::returnDigit() {
    return 25;
}

void TransmissionData::setRequest(std::vector<std::string> req) {
    request = req;
}

std::vector<std::string> TransmissionData::getRequest(void) {
    return request;
}

void TransmissionData::setReply(std::vector<std::string> rep) {
    reply = rep;
}

std::vector<std::string> TransmissionData::getReply(void) {
    return reply;
}

void TransmissionData::setParNr(int _parNr) {
    parNr = _parNr;
}

int TransmissionData::getParNr(void) {
    return parNr;
}

void TransmissionData::insertSentFrom(std::string srcNode) {
    sentFrom.insert(srcNode);
}
std::set<std::string> TransmissionData::getSentFrom(void) {
    return sentFrom;
}

void TransmissionData::insertDeliveredFrom(std::string srcNode) {
    deliveredFrom.insert(srcNode);
}
std::set<std::string> TransmissionData::getDeliveredFrom(void) {
    return deliveredFrom;
}

void TransmissionData::insertReplied(int node) {
    replied.insert(node);
}
void TransmissionData::clearReplied() {
    replied.clear();
}
std::set<int> TransmissionData::getReplied(void) {
    return replied;
}

void TransmissionData::setSentReply(bool sentRep) {
    sentReply = sentRep;
}
bool TransmissionData::getSentReply(void) {
    return sentReply;
}

