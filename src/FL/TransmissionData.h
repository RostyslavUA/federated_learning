/*
 * TransmissionData.h
 *
 *  Created on: Apr 12, 2022
 *      Author: ross
 */


#ifndef FL_TRANSMISSIONDATA_H_
#define FL_TRANSMISSIONDATA_H_

#include <omnetpp.h>  // confirm if needed

class TransmissionData : public omnetpp::cSimpleModule {
private:
    std::vector<std::string> request;
    std::vector<std::string> reply;

    int parNr;
    std::set<std::string> sentFrom;
    std::set<std::string> deliveredFrom; // Indicates from which node the message was successfully delivered to its destination nodes
    std::set<int> replied; // if the node replies, its index is added to this set
    bool sentReply = false;  // confirm that initial value of false is correct

    //cOutVector delayVector;
public:
    void initialize();
    int returnDigit();

    void setRequest(std::vector<std::string>);
    std::vector<std::string> getRequest(void);

    void setReply(std::vector<std::string>);
    std::vector<std::string> getReply(void);

    void setParNr(int _parNr);
    int getParNr(void);

    void insertSentFrom(std::string);
    std::set<std::string> getSentFrom(void);

    void insertDeliveredFrom(std::string);
    std::set<std::string> getDeliveredFrom(void);

    void insertReplied(int node);
    void clearReplied(void);
    std::set<int> getReplied(void);

    void setSentReply(bool);
    bool getSentReply(void);
};
#endif /* FL_TRANSMISSIONDATA_H_ */


