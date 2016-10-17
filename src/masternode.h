#ifndef MASTERNODE_H
#define MASTERNODE_H

#include <string>
#include <vector>
#include "uint256.h"
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
class CMasterNode
{    
private:
    std::vector<std::string> masterNodeAddresses; // the list of all addresses in a wallet that have a balance that counts towards masternodes, default is all
    int64_t lastReceived; /// the last time an update was received for this flynode
    CAddress addr;
    uint256 lastNonce;

public:
    CMasterNode();
    CMasterNode(CAddress address);

    IMPLEMENT_SERIALIZE
    (
        READWRITE(masterNodeAddresses);
        READWRITE(lastReceived);
        READWRITE(addr);
        READWRITE(lastNonce);
    )

    friend bool operator==(const CMasterNode& a, const CMasterNode& b)
    {
        return (a.getAddr() == b.getAddr());
    }
    void addAddressToList(std::string addr);
    bool IsMe();
    void updateLastTime(int64_t time);
    std::vector<std::string> getNodeAddresses();
    void setNodeAddress(std::string addr);
    uint256 getNonce();
    void updateLastNonce(uint256 nonce);
    CAddress getAddr();
};




///This is the class that controls and maintains all masternode information for this client. there should only ever be one of these active at a given time.
class MasterNodeControl
{
private:
    std::vector<CMasterNode> masterNodeList;/// the list of all known active masterNodes


public:
    static const int64_t minimumForMaster = 500 * COIN;
    MasterNodeControl();
    void addToMasterNodeList(CMasterNode node, int64_t time, uint256 nonce);
    bool removeFromMasterNodeList(CMasterNode node);
    std::vector<CMasterNode> getMasterNodeList();
    CMasterNode selectRandomMasterNode();
    bool contains(CMasterNode node);
    unsigned int getlocation(CMasterNode node);
    void updateNodeTime(unsigned int location, int64_t newTime);
    void updateNodeNonce(unsigned int location, uint256 nonce);
    uint256 getNodesNonce(unsigned int loc);
};

#endif // MASTERNODE_H
