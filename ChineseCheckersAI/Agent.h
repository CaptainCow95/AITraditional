//===------------------------------------------------------------*- C++ -*-===//
#ifndef AGENT_H_INCLUDED
#define AGENT_H_INCLUDED

#include <atomic>
#include <chrono>
#include <vector>
#include "ChineseCheckersState.h"
#include "ThreadPool.h"
#include "Tree.h"

enum Players
{
    player1,
    player2
};

class Agent
{
public:
    Agent();
    ~Agent();
    static Players playGame(Agent& player1, Agent& player2);
    void playGame();
    void setDepth(int depth);
    void setExplorationConstant(int constant);
    void setName(std::string newName);
    void setSecondsPerTurn(int value);
    void setVerbose(bool value);

private:
    void applyNodeToState(MoveTree::MoveTreeNode* node, ChineseCheckersState& stateCopy);
    int calculateDistanceToHome(unsigned piece, unsigned player);
    int calculateMoveDistance(Move m, int player);
    float calculateUCBValue(int samples, int64_t payout);
    int evaluatePosition(ChineseCheckersState& state);
    void getStateCopy(MoveTree::MoveTreeNode* node, ChineseCheckersState& stateCopy);
    bool isValidMoveMessage(const std::vector<std::string>& tokens) const;
    bool isValidStartGameMessage(const std::vector<std::string>& tokens) const;
    Move nextMove();
    int playRandomDepth(ChineseCheckersState& state);
    void printAndRecvEcho(const std::string& msg) const;
    std::string readMsg() const;
    void runMonteCarlo(void*);
    int simulate(MoveTree::MoveTreeNode* node);
    int simulate(MoveTree::MoveTreeNode* node, Move m);
    void switchCurrentPlayer();
    std::vector<std::string> tokenizeMsg(const std::string& msg) const;
    void waitForStart();

    const int WIN = 200;
    const int LOSE = -200;

    Players current_player;
    Players my_player;
    std::string name;
    std::string opp_name;

    ThreadPool* threadPool;
    int deepestDepth;
    int maxDepth = 10;
    int explorationConstant = 35;
    int secondsPerTurn = 10;
    ChineseCheckersState state;
    std::atomic<uint32_t> totalSamples;
    int verbose = 0;
    MoveTree* tree;
    std::chrono::system_clock::time_point endTime;
};

#endif
