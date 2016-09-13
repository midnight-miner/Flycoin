#ifndef MASTERNODE_H
#define MASTERNODE_H

#include <string>
#include <vector>
#include "net.h"

class CMasterNode;
class MasterNodeControl;

extern CNode* myNode;
extern CMasterNode* myMasterNode;
extern MasterNodeControl* pMasterNodeMain;
extern bool setupFlyNetwork();
extern void createMyNode();


/// The MasterNode is an extension off of a regular CNode peer that keeps the info needed for syncing masternode lists among the network
/// Our MasterNode information is held in the myMasterNode variable;
class CMasterNode : public CNode
{    
private:
    std::vector<std::string> masterNodeAddresses; // the list of all addresses in a wallet that have a balance that counts towards masternodes, default is all
    int64_t lastReceived; /// the last time a payment was received for this master node

public:
    CMasterNode(const CNode &node);
    void addAddressToList(std::string addr);
    bool IsMe();
    void updateLastTime();
};




///This is the class that controls and maintains all masternode information for this client. there should only ever be one of these active at a given time.
class MasterNodeControl
{
private:
    std::vector<CMasterNode*> masterNodeList;/// the list of all known active masterNodes


public:
    MasterNodeControl();
    bool updateMasterNodeList();
    std::vector<CMasterNode*> getMasterNodeList();
    CMasterNode* selectRandomMasterNode();

};

#endif // MASTERNODE_H
