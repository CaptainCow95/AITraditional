//===------------------------------------------------------------*- C++ -*-===//
#ifndef AGENT_H_INCLUDED
#define AGENT_H_INCLUDED

#include <chrono>
#include <string>
#include <vector>

#include "ChineseCheckersState.h"
#include "Tree.h"

struct MoveEntry
{
    int samples;
    int64_t payout;
    Move move;
};

class Agent {
public:
    Agent();
    ~Agent();
    void playGame();
    void setName(std::string newName);
    void setDepth(int depth);

private:
    Move nextMove();
    void runMonteCarlo(std::chrono::system_clock::time_point endTime);
    void runSampling(Tree<MoveEntry>::TreeNode& node);
    float calculateUCBValue(MoveEntry me);
    int calculateMoveDistance(Move m, int player);
    int playRandom(Move m);
    int playRandomDepth(Move m);
    int evaluatePosition(ChineseCheckersState& state);
    int calculateDistanceToHome(unsigned piece, unsigned player);
    void printAndRecvEcho(const std::string &msg) const;
    std::string readMsg() const;
    std::vector<std::string> tokenizeMsg(const std::string &msg) const;
    void waitForStart();
    void switchCurrentPlayer();

    bool isValidStartGameMessage(const std::vector<std::string> &tokens) const;
    bool isValidMoveMessage(const std::vector<std::string> &tokens) const;

    const int TIMEOUT = INT_MAX - 1;
    const int SECONDS_PER_TURN = 10;
    const int WIN = INT_MAX;
    const int LOSE = INT_MIN + 1;
    ChineseCheckersState state;
    enum Players { player1, player2 };
    Players current_player;
    Players my_player;
    std::string name;
    std::string opp_name;
    int totalSamples;
    int maxDepth = -1;
    Tree<MoveEntry>* _tree;
};

#endif
