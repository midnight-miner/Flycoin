#include "masternode.h"
#include "base58.h"
#include "init.h"

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <algorithm>
#ifdef WIN32
#include <windows.h>
#include <wininet.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#endif

CNode* myNode = NULL;
CMasterNode* myMasterNode = NULL;
MasterNodeControl* pMasterNodeMain = NULL;

void createMyNode()
{
    ///this sets our own local stats for a CNode

    CNetAddr addrLocalHost;
    if (GetMyExternalIP(addrLocalHost))
    {
        printf("GetMyExternalIP() returned %s\n", addrLocalHost.ToStringIP().c_str());
    }
    myNode = new CNode(INVALID_SOCKET, CAddress(CService(addrLocalHost, GetListenPort()), nLocalServices));
}

bool setupFlyNetwork()
{
    createMyNode();
    pMasterNodeMain = new MasterNodeControl();
    ///check to see if we have enough to be a masternode
    int64_t ourBalance = pwalletMain->GetBalance();
    printf("Our balance is %" PRId64 " satoshi \n", ourBalance);
    printf("Minimum balance for masternode is %" PRId64 " satoshi \n", pMasterNodeMain->minimumForMaster);
    if(ourBalance > pMasterNodeMain->minimumForMaster)
    {
        printf("We are broadcasting as a master node \n");
        /// only setup our masterNode if we qualify to run as a masterNode
        myMasterNode = new CMasterNode(myNode->addr);
        //Get all our addresses with a balance and add them to our masternodes list of addresses
        BOOST_FOREACH(std::set<CTxDestination> grouping, pwalletMain->GetAddressGroupings())
        {
            BOOST_FOREACH(CTxDestination address, grouping)
            {
                myMasterNode->addAddressToList(CBitcoinAddress(address).ToString());
            }
        }
    }
    return true;
}

CMasterNode::CMasterNode()
{
    masterNodeAddresses.clear();
    lastReceived = 0;
    addr = CAddress();
    lastNonce = 0;
}

CMasterNode::CMasterNode(CAddress address)
{
    masterNodeAddresses.clear();
    lastReceived = 0;
    addr = address;
    lastNonce = 0;
}

void CMasterNode::addAddressToList(std::string addr)
{
    masterNodeAddresses.push_back(addr);
}

// this checks to see if the node is me
bool CMasterNode::IsMe()
{
    for(auto &nodeAddr : masterNodeAddresses)
    {
        if(IsMine(*pwalletMain, CBitcoinAddress(nodeAddr).Get()))
        {
            return true;
        }
    }
}

void CMasterNode::updateLastTime(int64_t time)
{
    this->lastReceived = time;
}

std::vector<std::string> CMasterNode::getNodeAddresses()
{
    return masterNodeAddresses;
}

void CMasterNode::setNodeAddress(std::string addr)
{
    masterNodeAddresses.emplace_back(addr);
}

uint256 CMasterNode::getNonce()
{
    return lastNonce;
}

void CMasterNode::updateLastNonce(uint256 nonce)
{
    this->lastNonce = nonce;
}

CAddress CMasterNode::getAddr()
{
    return addr;
}





MasterNodeControl::MasterNodeControl()
{
    this->masterNodeList.clear();
}


void MasterNodeControl::addToMasterNodeList(CMasterNode node, int64_t time, uint256 nonce)
{
    node.updateLastNonce(nonce);
    node.updateLastTime(time);
    masterNodeList.emplace_back(node);
}

bool MasterNodeControl::removeFromMasterNodeList(CMasterNode node)
{
    bool hadNode = false;
    std::vector<CMasterNode> listCopy = masterNodeList;
    masterNodeList.clear();
    for(unsigned int i = 0; i < listCopy.size(); ++i)
    {
        CMasterNode curNode = listCopy[i];
        if(curNode.getAddr() == node.getAddr()) //compare ip addresses
        {
            hadNode = true; /// shows that we had the node in the first place
            /// do nothing, not adding to new list
        }
        else
        {
            masterNodeList.emplace_back(curNode);
        }
    }
    return hadNode;
}

std::vector<CMasterNode> MasterNodeControl::getMasterNodeList()
{
    return this->masterNodeList;
}

CMasterNode MasterNodeControl::selectRandomMasterNode()
{
    begin:
    unsigned int totalKnownNodes = this->masterNodeList.size();
    int random = GetRandInt(2147483646); // one less than the max value possible for an int.
    /// use mod math so random is guarenteed to be in the acceptable range
    int nodeIndex = random%(totalKnownNodes - 1); // must subtract one from the total because indicies start at 0 not 1
    CMasterNode selectedNode = this->masterNodeList[nodeIndex];

    //Perform a series of checks to see if the selected node is valid
    if(selectedNode.IsMe())
    {
        //cant send coins to myself for a masternode reward
        goto begin; /// you lovin that goto swag arent u? yeah. uk u are
    }

    return selectedNode;
}

bool MasterNodeControl::contains(CMasterNode node)
{
    for( unsigned int i = 0; i < masterNodeList.size(); ++i)
    {
        CMasterNode curNode = masterNodeList[i];
        if(curNode.getAddr() == node.getAddr())
        {
            return true;
        }
    }
    return false;
}

unsigned int MasterNodeControl::getlocation(CMasterNode node)
{
    for( unsigned int i = 0; i < masterNodeList.size(); ++i)
    {
        CMasterNode curNode = masterNodeList[i];
        if(curNode.getAddr() == node.getAddr())
        {
            return i;
        }
    }
    return -1;
}

void MasterNodeControl::updateNodeTime(unsigned int location, int64_t newTime)
{
    masterNodeList[location].updateLastTime(newTime);
}

void MasterNodeControl::updateNodeNonce(unsigned int location, uint256 nonce)
{
    masterNodeList[location].updateLastNonce(nonce);
}

uint256 MasterNodeControl::getNodesNonce(unsigned int loc)
{
    return masterNodeList[loc].getNonce();
}
