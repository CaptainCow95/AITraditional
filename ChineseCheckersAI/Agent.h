//===------------------------------------------------------------*- C++ -*-===//
#ifndef AGENT_H_INCLUDED
#define AGENT_H_INCLUDED

#include <chrono>
#include <string>
#include <vector>

#include "ChineseCheckersState.h"

#define EXACT 0
#define LOWERBOUND 1
#define UPPERBOUND 2

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
	void setDebug();

private:
	Move nextMove();
	void printAndRecvEcho(const std::string &msg) const;
	std::string readMsg() const;
	std::vector<std::string> tokenizeMsg(const std::string &msg) const;
	void waitForStart();
	void switchCurrentPlayer();

	bool isValidStartGameMessage(const std::vector<std::string> &tokens) const;
	bool isValidMoveMessage(const std::vector<std::string> &tokens) const;

	int getBestMove(ChineseCheckersState& state, unsigned depth, unsigned maxDepth, std::chrono::system_clock::time_point& endTime, uint64_t hash, int alpha, int beta, Move& move); // move is an out parameter
	int getBestMoveDebug(ChineseCheckersState& state, unsigned depth, unsigned maxDepth, std::vector<Move>& movesList); // move is an out paramter
	int evaluatePosition(ChineseCheckersState& state);
	int evaluatePositionDebug(ChineseCheckersState& state);
	static int calculateDistanceToHome(unsigned piece, unsigned player);
	uint64_t hash(ChineseCheckersState& state);
	static bool MoveSortP1(Move a, Move b);
	static bool MoveSortP2(Move a, Move b);

	const int TIMEOUT = INT_MAX - 1;
	const int SECONDS_PER_TURN = 10;
	const int TTSIZE = 100000000;
	const int WIN = INT_MAX;
	const int LOSE = INT_MIN + 1;
	ChineseCheckersState state;
	enum Players { player1, player2 };
	Players current_player;
	Players my_player;
	std::string name;
	unsigned int fixedDepth = 0;
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
};

#endif
