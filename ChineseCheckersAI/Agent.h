//===------------------------------------------------------------*- C++ -*-===//
#ifndef AGENT_H_INCLUDED
#define AGENT_H_INCLUDED

#include <chrono>
#include <string>
#include <vector>

#include "ChineseCheckersState.h"

#define EXACT 0
#define ALPHA 1
#define BETA 2

struct TTEntry
{
	int value;
	uint16_t turn;
	uint8_t currentPlayer;
	uint8_t flag;
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
	void printAndRecvEcho(const std::string &msg) const;
	std::string readMsg() const;
	std::vector<std::string> tokenizeMsg(const std::string &msg) const;
	void waitForStart();
	void switchCurrentPlayer();

	bool isValidStartGameMessage(const std::vector<std::string> &tokens) const;
	bool isValidMoveMessage(const std::vector<std::string> &tokens) const;

	int getBestMove(ChineseCheckersState& state, unsigned depth, unsigned maxDepth, std::chrono::system_clock::time_point& endTime, int positionStrength, uint64_t hash, int alpha, int beta, Move& move); // move is an out parameter
	int getBestMoveDebug(ChineseCheckersState& state, unsigned depth, unsigned maxDepth, int positionStrength, std::vector<Move>& movesList); // move is an out paramter
	int evaluatePosition(ChineseCheckersState& state);
	int calculateDistanceToHome(ChineseCheckersState& state, unsigned piece, unsigned player);
	uint64_t hash(ChineseCheckersState& state);

	const int TIMEOUT = INT_MAX - 1;
	const int SECONDS_PER_TURN = 10;
	const int TTSIZE = 100000000;
	ChineseCheckersState state;
	enum Players { player1, player2 };
	Players current_player;
	Players my_player;
	std::string name;
	unsigned int maxDepth;
	bool debugging = false;
	std::string opp_name;
	unsigned operations;
	std::vector<std::vector<Move>*>* moveVectorCache;
	std::vector<std::vector<Move>*>* moveVectorCacheDebug;
	std::vector<std::vector<Move>*>* bestMoveVectorCache;
	std::vector<std::vector<Move>*>* bestMoveVectorCacheDebug;
	TTEntry* transpositionTable;
	int currentTurn;
	std::array<uint64_t, 162> zobristNumbers;
	int skipped;
	int alphaBetaSkipped;
};

#endif
