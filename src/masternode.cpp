#include "masternode.h"
#include "base58.h"

CMasterNode* myMasterNode;

CMasterNode::CMasterNode()
{

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


MasterNodeControl::MasterNodeControl()
{
    this->masterNodeList.clear();
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
    if(selectedNode->nMisbehavior > 10)
    {
        //select a new node if the selected one is being naughty.
        goto begin;
    }

    return selectedNode;
}
