#include "main.h"
#include "base58.h"
#include "stakereward.h"
#include <boost/assign/list_of.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>

using namespace boost;

int static StakeReward::generateMTRandom(unsigned int s, int range)
{
    random::mt19937 gen(s);
    random::uniform_int_distribution<> dist(0, range);
    return dist(gen);
}

//////////////////////////////////////////////////////////////////////////////
//
// Fork Comparison Functions by Griffith
//

/// this function is used to determine if the time of a TX was before a fork point
/// (if the fork point is the nHeight of the chain at a fork)
bool StakeReward::IsBeforeBlock(unsigned int nTime, int nHeightOfFork)
{
    /// this is ok to check against pindexBest because once it actually gets to that point it will use another check
    /// not using a second check after pindexBest has passed the fork height is how the chain broke - Griffith
    if(nHeightOfFork > pindexBest->nHeight) // can also use nBestHeight here but they SHOULD be the same value
    {
        /// return true here because we havent hit that block yet
        return true;
    }
    BlockFind:
    /// start at the top of the chain to find the place we are actially looking for
    CBlockIndex* pindexFinder = pindexBest;
    /// while we havent found the block we are looking for, keep going back until we find it
    while(pindexFinder->nHeight > nHeightOfFork)
    {
        if(pindexFinder->pprev == pindexGenesisBlock && nHeightOfFork !=0)
        {
            /// this should NEVER happen, it will happen if a negative number is entered but again,
            ///  that should never be the case
            assert(false);
        }
        pindexFinder = pindexFinder->pprev;
    }
    /// once we are here pindexFiner height should be the same as the height of the fork
    if(pindexFinder->nHeight != nHeightOfFork)
    {
        /// this should not happen, if it does something fucked up or your chain is corrupt
        printf("ERROR FINDING BLOCK HEIGHT RETRYING \n");
        goto BlockFind;
    }
    /// this return determines if we our current time is less than the time of the fork
    if(pindexFinder->nTime > nTime)
    {
        /// we are before the fork so return true
        return true;
    }
    /// orr we arent
    return false;
}

/// this function is used to determine if the time of a TX was after a fork point
/// this is a partner funcion to IsBeforeBlock
/// (if the fork point is the nHeight of the chain at a fork)
bool StakeReward::IsAfterBlock(unsigned int nTime, int nHeightOfFork)
{
    if(nHeightOfFork > pindexBest->nHeight)
    {
        /// return false here because we havent hit that block yet
        return false;
    }
    Recheck:
    CBlockIndex* pindexFinder = pindexBest;
    while(pindexFinder->nHeight > nHeightOfFork)
    {
        if(pindexFinder->pprev == pindexGenesisBlock && nHeightOfFork !=0)
        {
            /// this should NEVER happen
            assert(false);
        }
        pindexFinder = pindexFinder->pprev;
    }
    /// once we are here pindexFiner height should be the same as the height of the fork
    if(pindexFinder->nHeight != nHeightOfFork)
    {
        /// this should not happen, if it does something fucked up or your chain is corrupt
        printf("ERROR FINDING BLOCK HEIGHT RETRYING \n");
        goto Recheck;
    }
    if(pindexFinder->nTime < nTime)
    {
        /// our time is after the forks time
        return true;
    }
    /// or we arent after the fork yet.
    return false;
}



std::string StakeReward::GetStakeLevelName(StakeLevel stakeLevel)
{
    switch(stakeLevel)
    {
        case SuperFly:
            return "SuperFly";
        default:
            return "Standard";
    }
}

StakeLevel StakeReward::GetStakeLevel(CTxDestination address)
{
    //SuperFly wallet addresses
    std::map<CTxDestination, std::string> superflyList = boost::assign::map_list_of
    (CTxDestination(CBitcoinAddress("FLBAFCtU1YeAJXCB5rBgcuAmBVok4A4KbY").Get()), "SuperFly1")
    (CTxDestination(CBitcoinAddress("FGparLdeydNbZgMhP9WQGnwMySQh73ADgN").Get()), "SuperFly2")
    (CTxDestination(CBitcoinAddress("FKcbRi4z1PjWkAxYH1ScbzhQH19JLg6aNu").Get()), "SuperFly3")
    (CTxDestination(CBitcoinAddress("FHppgcMjtKq8jHN7wzxQBSptCukJGBujzH").Get()), "SuperFly4")
    (CTxDestination(CBitcoinAddress("FN1ec52r9XFVASURyd8yw9nuU3uNDbY1cb").Get()), "SuperFly5");

    if (superflyList.count(address))
        return SuperFly;

    return Standard;
}

int64_t StakeReward::GetStakeInterest(CTxDestination address, int height)
{
    if (height >= FORK_HEIGHT_13)
        return MAX_MINT_PROOF_OF_STAKE_3;

    if (height >= FORK_HEIGHT_12)
        return GetStakeInterestV12(address, height);

    if (height >= FORK_HEIGHT_11)
        return MAX_MINT_PROOF_OF_STAKE_2; // Standard 35% annual interest

    if (height >= FORK_HEIGHT_10)
        return MAX_MINT_PROOF_OF_STAKE_2; // Standard 35% annual interest

    if (height >= FORK_HEIGHT_9)
        return MAX_MINT_PROOF_OF_STAKE_2; // Standard 35% annual interest

    else
        return  MAX_MINT_PROOF_OF_STAKE_1;  // Standard 50% annual interest


}

int64_t StakeReward::GetStakeInterestV9(CTxDestination address, int height)
{
            return MAX_MINT_PROOF_OF_STAKE_2; // Standard 35% annual interest
}

int64_t StakeReward::GetStakeInterestV10(CTxDestination address, int height)
{
            return MAX_MINT_PROOF_OF_STAKE_2; // Standard 35% annual interest
}

int64_t StakeReward::GetStakeInterestV11(CTxDestination address, int height)
{
    return MAX_MINT_PROOF_OF_STAKE_2; // Standard 35% annual interest
}


int64_t StakeReward::GetStakeInterestV12(CTxDestination address, int height)
{
    StakeLevel stakeLevel = GetStakeLevel(address);

    switch(stakeLevel)
    {

        case SuperFly:
            return MAX_MINT_PROOF_OF_STAKE_SUPERFLY; 	// SuperFly 200% annual interest
        default:
            return MAX_MINT_PROOF_OF_STAKE_3; // Standard 10% annual interest

    }
}


int64_t StakeReward::GetMaxMintProofOfStake(unsigned int time)
{
    if(IsBeforeBlock(time, FORK_HEIGHT_9))
    {
        return MAX_MINT_PROOF_OF_STAKE_1;
    }
    else if (IsBeforeBlock(time, FORK_HEIGHT_12))
    {
        return MAX_MINT_PROOF_OF_STAKE_2;
    }
    return MAX_MINT_PROOF_OF_STAKE_3;
}

int64_t StakeReward::GetBonusReward(unsigned int nTime, int64_t nValueIn, uint256 prevHash)
{
    int64_t nBonusSubsidy = 0;

    if(nTime < FORK_TIME) //old superblock reward
    {
//		int64_t nSubsidy = nCoinAge * nRewardCoinYear / 365 / COIN;
        //super block calculations from breakcoin
        std::string cseed_str = prevHash.ToString().substr(7,7);
        const char* cseed = cseed_str.c_str();
        long seed = hex2long(cseed);
        int rand1 = generateMTRandom(seed, 100);
        int rand2 = generateMTRandom(seed+1, 100);
        int rand3 = generateMTRandom(seed+2, 100);
        int rand4 = generateMTRandom(seed+3, 100);
        int rand5 = generateMTRandom(seed+4, 100);
        int64_t inputcoins = nValueIn / COIN;

        if (inputcoins >= 25000)
        {
            if(rand1 <= 5)
                nBonusSubsidy += 1 * COIN;
            if(rand2 <= 4)
                nBonusSubsidy += 2 * COIN;
            if(rand3 <= 3)
                nBonusSubsidy += 3 * COIN;
            if(rand4 <= 2)
                nBonusSubsidy += 5 * COIN;
            if(rand5 <= 1)
                nBonusSubsidy += 10 * COIN;
        }

        return nBonusSubsidy;
    }

    //super block calculations from breakcoin
    std::string cseed_str = prevHash.ToString().substr(7,7);
    const char* cseed = cseed_str.c_str();
    long seed = hex2long(cseed);
    int rand1 = generateMTRandom(seed, 1000000);

    nBonusMultiplier = 1;


    if (nTime < FORK_TIME_2) //new superblock reward
    {
//        CBigNum bnSubsidy = CBigNum(nCoinAge) * nRewardCoinYear / 365 / COIN;
//        int64_t nSubsidy = bnSubsidy.getuint64();
//        nBonusMultiplier = 1;

        //super block calculations from breakcoin
//        std::string cseed_str = prevHash.ToString().substr(7,7);
//        const char* cseed = cseed_str.c_str();
//        long seed = hex2long(cseed);
//        int rand1 = generateMTRandom(seed, 1000000);

        if(rand1 <= 500) // 0.5% chance
            nBonusMultiplier = 2;
        if(rand1 <= 400) // 0.4% chance
            nBonusMultiplier = 3;
        if(rand1 <= 90) // 0.09% chance
            nBonusMultiplier = 5;
        if(rand1 <= 9) // 0.009% chance
            nBonusMultiplier = 10;
        if(rand1 <= 1) // 0.0001% chance
            nBonusMultiplier = 20;

        return nBonusMultiplier;
    }

    else if(IsBeforeBlock(nTime, FORK_HEIGHT_9))
    {
//        CBigNum bnSubsidy = CBigNum(nCoinAge) * nRewardCoinYear / 365 / COIN;
//        int64_t nSubsidy = bnSubsidy.getuint64();
//        nBonusMultiplier = 1;

        //super block calculations from breakcoin
//        std::string cseed_str = prevHash.ToString().substr(7,7);
//        const char* cseed = cseed_str.c_str();
//        long seed = hex2long(cseed);
//        int rand1 = generateMTRandom(seed, 1000000);

        if(rand1 <= 500 * 2) // 1% chance
            nBonusMultiplier = 2;
        if(rand1 <= 400 * 2) // 0.8% chance
            nBonusMultiplier = 3;
        if(rand1 <= 90 * 2) // 0.18% chance
            nBonusMultiplier = 5;
        if(rand1 <= 9 * 2) // 0.018% chance
            nBonusMultiplier = 10;
        if(rand1 <= 1 * 2) // 0.0002% chance
            nBonusMultiplier = 20;

        return nBonusMultiplier;
    }
    else if (pindexBest->nHeight <= FORK_HEIGHT_11)
    {
//        CBigNum bnSubsidy = CBigNum(nCoinAge) * nRewardCoinYear / 365 / COIN;
//        int64_t nSubsidy = bnSubsidy.getuint64();
//        nBonusMultiplier = 1;

        //super block calculations from breakcoin
//        std::string cseed_str = prevHash.ToString().substr(7,7);
//        const char* cseed = cseed_str.c_str();
//        long seed = hex2long(cseed);
//        int rand1 = generateMTRandom(seed, 1000000);

        if(rand1 <= 5000 * 2) // 10% chance
            nBonusMultiplier = 2;
        if(rand1 <= 2500 * 2) // 5% chance
            nBonusMultiplier = 3;
        if(rand1 <= 1500 * 2) // ~3% chance
            nBonusMultiplier = 5;
        if(rand1 <= 500 * 2) // 1% chance
            nBonusMultiplier = 10;
        if(rand1 <= 250 * 2) // 0.5% chance
            nBonusMultiplier = 20;
        if(rand1 <= 125 * 2) // 0.25% chance
            nBonusMultiplier = 50;
        if(rand1 <= 5 * 2) // 0.01% chance
            nBonusMultiplier = 100;

        return nBonusMultiplier;
    }
    else if(IsBeforeBlock(nTime, FORK_HEIGHT_12))
    {
        int64_t nMinimumStakeHours = nStakeMinAge / 60 / 60;

        int64_t nSubsidy = nCoinAge * MAX_MINT_PROOF_OF_STAKE_PARTICIPATION / 365 / nMinimumStakeHours;
        nBonusMultiplier = 1;

        //super block calculations from breakcoin
//        std::string cseed_str = prevHash.ToString().substr(7,7);
//        const char* cseed = cseed_str.c_str();
//        long seed = hex2long(cseed);
//        int rand1 = generateMTRandom(seed, 1000000);

        if(rand1 <= 5000 * 2) // 10% chance
            nBonusMultiplier = 2;
        if(rand1 <= 2500 * 2) // 5% chance
            nBonusMultiplier = 3;
        if(rand1 <= 1500 * 2) // ~3% chance
            nBonusMultiplier = 5;
        if(rand1 <= 500 * 2) // 1% chance
            nBonusMultiplier = 10;
        if(rand1 <= 250 * 2) // 0.5% chance
            nBonusMultiplier = 20;
        if(rand1 <= 125 * 2) // 0.25% chance
            nBonusMultiplier = 50;
        if(rand1 <= 5 * 2) // 0.01% chance
            nBonusMultiplier = 100;


        return nBonusMultiplier;
    }
    else
    {
        CBigNum bnSubsidy = CBigNum(nCoinAge) * nRewardCoinYear / 365 / COIN;
        int64_t nSubsidy = bnSubsidy.getuint64();
        nBonusMultiplier = 1;

        //super block calculations from breakcoin
//        std::string cseed_str = prevHash.ToString().substr(7,7);
//        const char* cseed = cseed_str.c_str();
//        long seed = hex2long(cseed);
//        int rand1 = generateMTRandom(seed, 1000000);

        if(rand1 <= 400000) // 40% chance
            nBonusMultiplier = 2;
        if(rand1 <= 200000) // 20% chance
            nBonusMultiplier = 3;
        if(rand1 <= 120000) // 12% chance
            nBonusMultiplier = 5;
        if(rand1 <= 40000) // 4% chance
            nBonusMultiplier = 10;
        if(rand1 <= 20000) // 2% chance
            nBonusMultiplier = 20;
        if(rand1 <= 10000) // 1% chance
            nBonusMultiplier = 50;
        if(rand1 <= 400) // 0.04% chance
            nBonusMultiplier = 100;

        return nBonusMultiplier;
    }



}
