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
    float average;
    Move move;
};

class Agent {
public:
    Agent();
    ~Agent();
    void playGame();
    void setName(std::string newName);

private:
    Move nextMove();
    Move runMonteCarlo();
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
    Tree<MoveEntry>* moves;
};

#endif
