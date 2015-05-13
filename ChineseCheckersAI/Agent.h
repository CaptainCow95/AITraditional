//===------------------------------------------------------------*- C++ -*-===//
#ifndef AGENT_H_INCLUDED
#define AGENT_H_INCLUDED

#include <atomic>
#include <chrono>
#include <vector>
#include "ChineseCheckersState.h"
#include "ThreadPool.h"
#include "Tree.h"

struct MoveEntry
{
    int samples;
    int64_t payout;
    Move move;
};

class Agent
{
public:
    Agent();
    ~Agent();
    void playGame();
    void setDepth(int depth);
    void setName(std::string newName);
    void setProfile();
    void setVerbose();

private:
    int calculateDistanceToHome(unsigned piece, unsigned player);
    int calculateMoveDistance(Move m, int player);
    float calculateUCBValue(MoveEntry me);
    int evaluatePosition(ChineseCheckersState& state);
    void getStateCopy(Tree<MoveEntry>::TreeNode& node, ChineseCheckersState& stateCopy);
    bool isValidMoveMessage(const std::vector<std::string>& tokens) const;
    bool isValidStartGameMessage(const std::vector<std::string>& tokens) const;
    Move nextMove();
    int playRandomDepth(ChineseCheckersState& state);
    void printAndRecvEcho(const std::string& msg) const;
    std::string readMsg() const;
    void runMonteCarlo(void*);
    void simulate(ChineseCheckersState& state, Tree<MoveEntry>::TreeNode& node);
    void switchCurrentPlayer();
    std::vector<std::string> tokenizeMsg(const std::string& msg) const;
    void waitForStart();

    enum Players
    {
        player1,
        player2
    };

    const int TIMEOUT = INT_MAX - 1;
    const int WIN = INT_MAX;
    const int LOSE = INT_MIN + 1;

    Players current_player;
    Players my_player;
    std::string name;
    std::string opp_name;

    ThreadPool* threadPool;
    int deepestDepth;
    int maxDepth = 5;
    int secondsPerTurn = 10;
    ChineseCheckersState state;
    std::atomic<int> totalSamples;
    bool verbose = false;
    Tree<MoveEntry>* tree;
    std::chrono::system_clock::time_point endTime;
};

#endif
