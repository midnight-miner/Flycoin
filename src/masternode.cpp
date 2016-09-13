#include "masternode.h"
#include "base58.h"
#include "init.h"

#include <iostream>
#include <cstring>
#include <unistd.h>
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
    int64_t minimumForMaster = 500 * COIN;
    printf("Minimum balance for masternode is %" PRId64 " satoshi \n", minimumForMaster);
    if(ourBalance > minimumForMaster)
    {
        printf("We are broadcasting as a master node \n");
        /// only setup our masterNode if we qualify to run as a masterNode
        myMasterNode = new CMasterNode(*myNode);
        //Get all our addresses with a balance and add them to our masternodes list of addresses
        BOOST_FOREACH(set<CTxDestination> grouping, pwalletMain->GetAddressGroupings())
        {
            BOOST_FOREACH(CTxDestination address, grouping)
            {
                myMasterNode->addAddressToList(CBitcoinAddress(address).ToString());
            }
        }
    }
    return true;
}
//SOCKET hSocketIn, CAddress addrIn, std::string addrNameIn = "", bool fInboundIn=false) : vSend(SER_NETWORK, MIN_PROTO_VERSION), vRecv(SER_NETWORK, MIN_PROTO_VERSION
CMasterNode::CMasterNode(const CNode& node) : CNode(node.hSocket, node.addr, node.addrName, node.fInbound)
{
    masterNodeAddresses.clear();
    lastReceived = 0;

    nServices = node.nServices;
    nLastSend = node.nLastSend;
    nLastRecv = node.nLastRecv;
    nLastSendEmpty = node.nLastSendEmpty;
    nTimeConnected = node.nTimeConnected;
    nHeaderStart = node.nHeaderStart;
    nMessageStart = node.nMessageStart;
    nVersion = node.nVersion;
    strSubVer = node.strSubVer;
    fOneShot = node.fOneShot;
    fClient = node.fClient; // set by version message
    fNetworkNode = node.fNetworkNode;
    fSuccessfullyConnected = node.fSuccessfullyConnected;
    fDisconnect = node.fDisconnect;
    nRefCount = node.nRefCount;
    nReleaseTime = node.nReleaseTime;
    hashContinue = node.hashContinue;
    pindexLastGetBlocksBegin = node.pindexLastGetBlocksBegin;
    hashLastGetBlocksEnd = node.hashLastGetBlocksEnd;
    nStartingHeight = node.nStartingHeight;
    fGetAddr = node.fGetAddr;
    nMisbehavior = node.getMisbehavior();
    hashCheckpointKnown = node.hashCheckpointKnown;
    setInventoryKnown.max_size(SendBufferSize() / 1000);
    setInventoryKnown = node.setInventoryKnown;
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

void CMasterNode::updateLastTime()
{

}


MasterNodeControl::MasterNodeControl()
{
    this->masterNodeList.clear();
}

bool MasterNodeControl::updateMasterNodeList()
{

}

std::vector<CMasterNode*> MasterNodeControl::getMasterNodeList()
{
    return this->masterNodeList;
}

CMasterNode* MasterNodeControl::selectRandomMasterNode()
{
    begin:
    unsigned int totalKnownNodes = this->masterNodeList.size();
    int random = GetRandInt(2147483646); // one less than the max value possible for an int.
    /// use mod math so random is guarenteed to be in the acceptable range
    int nodeIndex = random%(totalKnownNodes - 1); // must subtract one from the total because indicies start at 0 not 1
    CMasterNode* selectedNode = this->masterNodeList[nodeIndex];

    //Perform a series of checks to see if the selected node is valid
    if(selectedNode->IsMe())
    {
        //cant send coins to myself for a masternode reward
        goto begin; /// you lovin that goto swag arent u? yeah. uk u are
    }
    if(selectedNode->getMisbehavior() > 10)
    {
        //select a new node if the selected one is being naughty.
        goto begin;
    }

    return selectedNode;
}
