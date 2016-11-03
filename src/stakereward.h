#ifndef STAKEREWARD_H
#define STAKEREWARD_H

static const int64_t MAX_MINT_PROOF_OF_STAKE_1 = 50 * CENT; // 50% per year
static const int64_t MAX_MINT_PROOF_OF_STAKE_2 = 35 * CENT; // 35% per year
static const int64_t MAX_MINT_PROOF_OF_STAKE_3 = 10 * CENT; // 10% per year
static const int64_t MAX_MINT_PROOF_OF_STAKE_PARTICIPATION = 350; // 35% per year
static const int64_t MAX_MINT_PROOF_OF_STAKE_SUPERFLY = 200 * CENT; // SuperFly earns 200% from block 90k - 110k

static const unsigned int BLOCK_SWITCH_TIME = 1435708800; // 07/01/2015 @ 12:00am (UTC)
static const unsigned int FORK_TIME = 1444752000; // (GMT): Tue, 13 Oct 2015 16:00:00 GMT
static const unsigned int FORK_TIME_2 = 1446915600; // Sat, 07 Nov 2015 17:00:00 GMT
static const unsigned int FORK_TIME_3 = 1447278900; // Wednesday, 11 Nov 2015 21:55:00 GMT //keesdewit
static const unsigned int FORK_TIME_4 = 1448211600; // Sunday, 22 Nov 2015 17:00:00 GMT //keesdewit
static const unsigned int FORK_TIME_5 = 1450046160; // Sunday, 13 Dec 2015 22:36:00 GMT //keesdewit
static const int FORK_HEIGHT_9 = 67000;
static const int FORK_HEIGHT_10 = 70000;
static const int FORK_HEIGHT_11 = 77000;
static const int FORK_HEIGHT_12 = 90000; /// pos change to 10%, superblock probability increase. max age to 90 days, zap wallet fix
static const int FORK_HEIGHT_13 = 110000; /// end of Superfly fund 200% boost.



enum StakeLevel { SuperFly, Standard };

class StakeReward
{
public:
    static int64_t GetStakeInterest(CTxDestination address, int height);
    static int64_t GetStakeInterestV9(CTxDestination address, int height);
    static int64_t GetStakeInterestV10(CTxDestination address, int height);
    static int64_t GetStakeInterestV11(CTxDestination address, int height);
    static int64_t GetStakeInterestV12(CTxDestination address, int height);
    static StakeLevel GetStakeLevel(CTxDestination address);
    static StakeLevel GetStakeLevel2(CTxDestination address);
    static std::string GetStakeLevelName(StakeLevel stakeLevel);
    static int64_t GetMaxMintProofOfStake(unsigned int time);
    static int64_t GetBonusReward(unsigned int nTime, int64_t nValueIn, uint256 prevHash);

// signals:

// public slots:
private:
    static int generateMTRandom(unsigned int s, int range);
    static bool IsBeforeBlock(unsigned int nTime, int nHeightOfFork);
    static bool IsAfterBlock(unsigned int nTime, int nHeightOfFork);
};

#endif // STAKEREWARD_H
