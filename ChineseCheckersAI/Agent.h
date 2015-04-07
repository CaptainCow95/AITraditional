//===------------------------------------------------------------*- C++ -*-===//
#ifndef AGENT_H_INCLUDED
#define AGENT_H_INCLUDED

#include <chrono>
#include <string>
#include <vector>

#include "ChineseCheckersState.h"

class Agent {
public:
	Agent();
	~Agent();
	void playGame();
	void setName(std::string newName);

private:
	Move nextMove();
	void printAndRecvEcho(const std::string &msg) const;
	std::string readMsg() const;
	std::vector<std::string> tokenizeMsg(const std::string &msg) const;
	void waitForStart();
	void switchCurrentPlayer();

	bool isValidStartGameMessage(const std::vector<std::string> &tokens) const;
	bool isValidMoveMessage(const std::vector<std::string> &tokens) const;

	int getBestMove(ChineseCheckersState& state, unsigned depth, unsigned maxDepth, std::chrono::system_clock::time_point& endTime, int positionStrength, Move& move); // move is an out parameter
	int evaluatePosition(ChineseCheckersState& state);
	int calculateDistanceToHome(ChineseCheckersState& state, unsigned piece, unsigned player);

	const int TIMEOUT = INT_MAX - 1;
#ifdef DEBUG
	const int SECONDS_PER_TURN = 10000;
#else
	const int SECONDS_PER_TURN = 10;
#endif
	ChineseCheckersState state;
	enum Players { player1, player2 };
	Players current_player;
	Players my_player;
	std::string name;
	std::string opp_name;
	unsigned operations;
	std::vector<std::vector<Move>*>* moveVectorCache;
	std::vector<std::vector<Move>*>* bestMoveVectorCache;
};

#endif
