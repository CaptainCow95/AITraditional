#include "Agent.h"

#include <cstdlib>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <time.h>
#include <vector>

Agent::Agent() : name("Agent_NJ")
{
	transpositionTable = new TTEntry[TTSIZE];
	memset(transpositionTable, 0, TTSIZE * sizeof(TTEntry));

	srand(9167318);
	for (int i = 0; i < 162; ++i)
	{
		uint64_t x = rand() & 0xFF;
		x |= (uint64_t)(rand() & 0xFF) << 8;
		x |= (uint64_t)(rand() & 0xFF) << 16;
		x |= (uint64_t)(rand() & 0xFF) << 24;
		x |= (uint64_t)(rand() & 0xFF) << 32;
		x |= (uint64_t)(rand() & 0xFF) << 40;
		x |= (uint64_t)(rand() & 0xFF) << 48;
		x |= (uint64_t)(rand() & 0xFF) << 56;
		zobristNumbers[i] = x;
	}
}

Agent::~Agent()
{
	for (unsigned i = 0; i < moveVectorCache->size(); ++i)
	{
		delete moveVectorCache->at(i);
		delete bestMoveVectorCache->at(i);
		delete moveVectorCacheDebug->at(i);
		delete bestMoveVectorCacheDebug->at(i);
	}

	delete moveVectorCache;
	delete bestMoveVectorCache;
	delete moveVectorCacheDebug;
	delete bestMoveVectorCacheDebug;
}

Move Agent::nextMove() {
	if (moveVectorCache == NULL)
	{
		moveVectorCache = new std::vector<std::vector<Move>*>();
		bestMoveVectorCache = new std::vector<std::vector<Move>*>();
		moveVectorCacheDebug = new std::vector<std::vector<Move>*>();
		bestMoveVectorCacheDebug = new std::vector<std::vector<Move>*>();

		moveVectorCache->push_back(new std::vector<Move>());
		bestMoveVectorCache->push_back(new std::vector<Move>());
		moveVectorCacheDebug->push_back(new std::vector<Move>());
		bestMoveVectorCacheDebug->push_back(new std::vector<Move>());
	}

	++currentTurn;

	if (my_player == 0){
		if (currentTurn == 1)
		{
			return{ 3, 12 };
		}
		else if (currentTurn == 2)
		{
			return{ 1, 21 };
		}
		else if (currentTurn == 3)
		{
			return{ 2, 22 };
		}
		else if (currentTurn == 4)
		{
			return{ 22, 31 };
		}
	}
	else{
		if (currentTurn == 1)
		{
			return{ 77, 68 };
		}
		else if (currentTurn == 2)
		{
			return{ 79, 59 };
		}
		else if (currentTurn == 3)
		{
			return{ 78, 58 };
		}
		else if (currentTurn == 4)
		{
			return{ 58, 49 };
		}
	}

	std::chrono::system_clock::time_point startTime = std::chrono::system_clock::now();

	Move bestMove;
	Move move;
	unsigned maxDepth = 1;
	operations = 0;

	std::chrono::system_clock::time_point endTime;
	if (fixedDepth > 0)
	{
		endTime = startTime + std::chrono::hours(5);
	}
	else
	{
		endTime = startTime + std::chrono::milliseconds(SECONDS_PER_TURN * 1000 - 500);
	}

	bool cutoff;
	int moveValue = getBestMove(state, 0, maxDepth, endTime, hash(state), LOSE, WIN, cutoff, move);

	// Keep trying to go deeper and deeper in the search for the best move until we run out of time
	while (moveValue != TIMEOUT && moveValue != WIN)
	{
		bestMove = move;

		if (fixedDepth > 0 && maxDepth >= fixedDepth)
		{
			break;
		}

		// Store the best move and try to go deeper the next time
		++maxDepth;
		++currentTurn;

		if (moveVectorCache->size() < maxDepth)
		{
			// Allocate a new vector if necessary
			moveVectorCache->push_back(new std::vector<Move>());
			bestMoveVectorCache->push_back(new std::vector<Move>());
			moveVectorCacheDebug->push_back(new std::vector<Move>());
			bestMoveVectorCacheDebug->push_back(new std::vector<Move>());
		}

		moveValue = getBestMove(state, 0, maxDepth, endTime, hash(state), LOSE, WIN, cutoff, move);
	}

	std::cerr << "Reached depth of " << maxDepth << std::endl;

	return bestMove;
}

int Agent::getBestMove(ChineseCheckersState& state, unsigned depth, unsigned maxDepth, std::chrono::system_clock::time_point& endTime, uint64_t hash, int alpha, int beta, bool& cutoff, Move& move) // cutoff and move are out parameters
{
	int alphaOriginal = alpha;
	cutoff = false;

	if (depth >= maxDepth)
	{
		// We are at the max depth, return the current node's value
		return evaluatePosition(state);
	}

	if (std::chrono::system_clock::now() >= endTime)
	{
		// We ran out of time, return a timeout
		return TIMEOUT;
	}

	if (state.gameOver())
	{
		if (state.winner() == state.currentPlayer)
		{
			return WIN;
		}
		else
		{
			return LOSE;
		}
	}

	TTEntry entry = transpositionTable[hash % TTSIZE];
	if (entry.turn == currentTurn && entry.currentPlayer == state.currentPlayer)
	{
		if (entry.flag == EXACT)
		{
			return entry.value;
		}
		else if (entry.flag == LOWERBOUND)
		{
			alpha = std::max(alpha, entry.value);
		}
		else if (entry.flag == UPPERBOUND)
		{
			beta = std::min(beta, entry.value);
		}

		if (alpha >= beta)
		{
			cutoff = true;
			return entry.value;
		}
	}

	std::vector<Move>* moves = moveVectorCache->at(depth);
	state.getMoves(*moves);

	if (state.currentPlayer == 1)
	{
		std::sort(moves->begin(), moves->end(), MoveSortP1);
	}
	else
	{
		std::sort(moves->begin(), moves->end(), MoveSortP2);
	}

	std::vector<Move>* bestMoves = bestMoveVectorCache->at(depth);
	bestMoves->clear();
	int bestValue = LOSE;
	bool first = true;

	for (unsigned i = 0; i < moves->size(); ++i)
	{
		uint64_t newHash = hash;
		newHash ^= zobristNumbers[moves->at(i).from + 81 * (state.currentPlayer - 1)];
		newHash ^= zobristNumbers[moves->at(i).to + 81 * (state.currentPlayer - 1)];

		state.applyMove(moves->at(i));

		bool childCutoff;
		int value;
		if (!first)
		{
			value = getBestMove(state, depth + 1, maxDepth, endTime, newHash, -alpha - 1, -alpha, childCutoff, move);
			if (value > alpha && value < beta)
			{
				value = getBestMove(state, depth + 1, maxDepth, endTime, newHash, -beta, -value, childCutoff, move);
			}
		}
		else
		{
			value = getBestMove(state, depth + 1, maxDepth, endTime, newHash, -beta, -alpha, childCutoff, move);
		}

		state.undoMove(moves->at(i));

		if (value == TIMEOUT)
		{
			return TIMEOUT;
		}

		if ((first || !childCutoff) || (depth == 0 && bestMoves->size() == 0))
		{
			// Negate for negamax
			value = -value;

			if (value > bestValue)
			{
				bestValue = value;
				bestMoves->clear();
			}

			if (value == bestValue)
			{
				bestMoves->push_back(moves->at(i));
			}

			alpha = std::max(alpha, value);

			if (alpha >= beta)
			{
				cutoff = true;
				break;
			}
		}

		first = false;
	}

	TTEntry newEntry;
	newEntry.currentPlayer = state.currentPlayer;
	newEntry.turn = currentTurn;
	newEntry.value = bestValue;
	if (bestValue <= alphaOriginal)
	{
		newEntry.flag = UPPERBOUND;
	}
	else if (bestValue >= beta)
	{
		newEntry.flag = LOWERBOUND;
	}
	else
	{
		newEntry.flag = EXACT;
	}

	transpositionTable[hash % TTSIZE] = newEntry;
	if (bestMoves->size() > 0)
	{
		move = bestMoves->at(rand() % bestMoves->size());
	}

	if (debugging && alpha < beta)
	{
		std::vector<Move> debugMoves;
		int debugValue = getBestMoveDebug(state, depth, maxDepth, debugMoves);

		// Only check for our moves not being among the best moves found by the debug version since because of alpha-beta, we may not find every best node.
		for (Move m : *bestMoves)
		{
			if (std::find(debugMoves.begin(), debugMoves.end(), m) == debugMoves.end())
			{
				std::cerr << "Extra move found!" << std::endl;
			}
		}
	}

	return bestValue;
}

int Agent::getBestMoveDebug(ChineseCheckersState& state, unsigned depth, unsigned maxDepth, std::vector<Move>& movesList)
{
	if (depth >= maxDepth)
	{
		// We are at the max depth, return the current node's value
		return evaluatePositionDebug(state);
	}

	if (state.gameOver())
	{
		if (state.winner() == my_player + 1)
		{
			// The player won, so return the max value
			return WIN;
		}
		else
		{
			// The player lost, so return the min value
			return LOSE;
		}
	}

	std::vector<Move>* moves = moveVectorCacheDebug->at(depth);
	state.getMoves(*moves);

	std::vector<Move>* bestMoves = bestMoveVectorCacheDebug->at(depth);

	// This will be set on the first iteration
	int total;

	for (unsigned i = 0; i < moves->size(); ++i)
	{
		// Undo where the move is coming from
		int player = state.currentPlayer;

		state.applyMove(moves->at(i));

		// Redo where the move is going to
		std::vector<Move> newMoves;
		int value = getBestMoveDebug(state, depth + 1, maxDepth, newMoves);
		state.undoMove(moves->at(i));

		if (i == 0)
		{
			total = value;
			bestMoves->clear();
		}

		if (state.currentPlayer == my_player + 1)
		{
			// Take the max as this is our move
			if (value > total)
			{
				total = value;
				bestMoves->clear();
			}
		}
		else
		{
			// Take the min as this is our opponents move
			if (value < total)
			{
				total = value;
				bestMoves->clear();
			}
		}

		if (value == total)
		{
			bestMoves->push_back(moves->at(i));
		}
	}

	for (Move m : *bestMoves)
	{
		movesList.push_back(m);
	}

	return total;
}

uint64_t Agent::hash(ChineseCheckersState& state)
{
	unsigned int hash = 0;
	for (int i = 0; i < 81; ++i)
	{
		if (state.board[i] != 0)
		{
			hash ^= zobristNumbers[i + 81 * (state.board[i] - 1)];
		}
	}

	return hash;
}

// Evaluates the strength of the state's position for the state's current player
int Agent::evaluatePosition(ChineseCheckersState& state)
{
	int total = 0;
	for (int i = 0; i < 81; ++i)
	{
		if (state.board[i] != 0)
		{
			int player = state.board[i];
			int distance = calculateDistanceToHome(i, player);
			if (player == state.currentPlayer)
			{
				total -= distance;
			}
			else
			{
				total += distance;
			}
		}
	}

	return total;
}

int Agent::evaluatePositionDebug(ChineseCheckersState& state)
{
	int total = 0;
	for (int i = 0; i < 81; ++i)
	{
		if (state.board[i] != 0)
		{
			int player = state.board[i];
			int distance = calculateDistanceToHome(i, player);
			if (player == my_player + 1)
			{
				total -= distance;
			}
			else
			{
				total += distance;
			}
		}
	}

	return total;
}

int Agent::calculateDistanceToHome(unsigned piece, unsigned player)
{
	int row = piece / 9;
	int col = piece % 9;

	if (player == 1)
	{
		// if the row + col is at least 13, then the piece is in the home position
		return std::max(13 - (row + col), 0);
	}
	else
	{
		// if the row + col is 3 or less, then the piece is in the home position
		return std::max((row + col) - 3, 0);
	}
}

bool Agent::MoveSortP1(Move a, Move b)
{
	int fromARow = a.from / 9;
	int fromACol = a.from % 9;
	int toARow = a.to / 9;
	int toACol = a.to % 9;
	int distanceA = (fromARow + fromACol) - (toARow + toACol);

	int fromBRow = b.from / 9;
	int fromBCol = b.from % 9;
	int toBRow = b.to / 9;
	int toBCol = b.to % 9;
	int distanceB = (fromBRow + fromBCol) - (toBRow + toBCol);

	return distanceA < distanceB;
}

bool Agent::MoveSortP2(Move a, Move b)
{
	int fromARow = a.from / 9;
	int fromACol = a.from % 9;
	int toARow = a.to / 9;
	int toACol = a.to % 9;
	int distanceA = (toARow + toACol) - (fromARow + fromACol);

	int fromBRow = b.from / 9;
	int fromBCol = b.from % 9;
	int toBRow = b.to / 9;
	int toBCol = b.to % 9;
	int distanceB = (toBRow + toBCol) - (fromBRow + fromBCol);

	return distanceA < distanceB;
}

void Agent::playGame() {
	// Identify myself
	std::cout << "#name " << name << std::endl;

	// Wait for start of game
	waitForStart();

	// Main game loop
	for (;;) {
		if (current_player == my_player) {
			// My turn

			// Check if game is over
			if (state.gameOver()) {
				std::cerr << "I, " << name << ", have lost" << std::endl;
				switchCurrentPlayer();
				continue;
			}

			// Determine next move
			const Move m = nextMove();

			// Apply it locally
			state.applyMove(m);

			// Tell the world
			printAndRecvEcho(m);

			// It is the opponents turn
			switchCurrentPlayer();
		}
		else {
			// Wait for move from other player
			// Get server's next instruction
			std::string server_msg = readMsg();
			const std::vector<std::string> tokens = tokenizeMsg(server_msg);

			if (tokens.size() == 5 && tokens[0] == "MOVE") {
				// Translate to local coordinates and update our local state
				const Move m = state.translateToLocal(tokens);
				state.applyMove(m);

				// It is now my turn
				switchCurrentPlayer();
			}
			else if (tokens.size() == 4 && tokens[0] == "FINAL" &&
				tokens[2] == "BEATS") {
				// Game over
				if (tokens[1] == name && tokens[3] == opp_name)
					std::cerr << "I, " << name << ", have won!" << std::endl;
				else if (tokens[3] == name && tokens[1] == opp_name)
					std::cerr << "I, " << name << ", have lost." << std::endl;
				else
					std::cerr << "Did not find expected players in FINAL command.\n"
					<< "Found '" << tokens[1] << "' and '" << tokens[3] << "'. "
					<< "Expected '" << name << "' and '" << opp_name << "'.\n"
					<< "Received message '" << server_msg << "'" << std::endl;
				break;
			}
			else {
				// Unknown command
				std::cerr << "Unknown command of '" << server_msg << "' from the server"
					<< std::endl;
			}
		}
	}
}

void Agent::setName(std::string newName)
{
	name = newName;
}

void Agent::setDepth(int depth)
{
	fixedDepth = depth;
}

void Agent::setDebug()
{
	debugging = true;
}

// Sends a msg to stdout and verifies that the next message to come in is it
// echoed back. This is how the server validates moves
void Agent::printAndRecvEcho(const std::string &msg) const {
	// Note the endl flushes the stream, which is necessary
	std::cout << msg << std::endl;
	const std::string echo_recv = readMsg();
	if (msg != echo_recv)
		std::cerr << "Expected echo of '" << msg << "'. Received '" << echo_recv
		<< "'" << std::endl;
}

// Reads a line, up to a newline from the server
std::string Agent::readMsg() const {
	std::string msg;
	std::getline(std::cin, msg); // This is a blocking read

	// Trim white space from beginning of string
	const char *WhiteSpace = " \t\n\r\f\v";
	msg.erase(0, msg.find_first_not_of(WhiteSpace));
	// Trim white space from end of string
	msg.erase(msg.find_last_not_of(WhiteSpace) + 1);

	return msg;
}

// Tokenizes a message based upon whitespace
std::vector<std::string> Agent::tokenizeMsg(const std::string &msg) const {
	// Tokenize using whitespace as a delimiter
	std::stringstream ss(msg);
	std::istream_iterator<std::string> begin(ss);
	std::istream_iterator<std::string> end;
	std::vector<std::string> tokens(begin, end);

	return tokens;
}

void Agent::waitForStart() {
	for (;;) {
		std::string response = readMsg();
		const std::vector<std::string> tokens = tokenizeMsg(response);

		if (tokens.size() == 4 && tokens[0] == "BEGIN" &&
			tokens[1] == "CHINESECHECKERS") {
			// Found BEGIN GAME message, determine if we play first
			if (tokens[2] == name) {
				// We go first!
				opp_name = tokens[3];
				my_player = player1;
				break;
			}
			else if (tokens[3] == name) {
				// They go first
				opp_name = tokens[2];
				my_player = player2;
				break;
			}
			else {
				std::cerr << "Did not find '" << name
					<< "', my name, in the BEGIN command.\n"
					<< "# Found '" << tokens[2] << "' and '" << tokens[3] << "'"
					<< " as player names. Received message '" << response << "'"
					<< std::endl;
				std::cout << "#quit" << std::endl;
				std::exit(EXIT_FAILURE);
			}
		}
		else if (response == "DUMPSTATE") {
			std::cout << state.dumpState() << std::endl;
		}
		else if (tokens[0] == "LOADSTATE") {
			std::string new_state = response.substr(10);
			if (!state.loadState(new_state))
				std::cerr << "Failed to load '" << new_state << "'\n";
		}
		else if (response == "LISTMOVES") {
			std::vector<Move> moves;
			state.getMoves(moves);
			for (const auto i : moves)
				std::cout << i.from << ", " << i.to << "; ";
			std::cout << std::endl;
		}
		else if (tokens[0] == "MOVE") {
			// Just apply the move
			const Move m = state.translateToLocal(tokens);
			if (!state.applyMove(m))
				std::cout << "Unable to apply move " << m << std::endl;
		}
		else if (tokens[0] == "PROFILE") {
			for (int i = 0; i < 6; ++i)
			{
				nextMove();
			}
		}
		else {
			std::cerr << "Unexpected message " << response << "\n";
		}
	}

	// Game is about to begin, restore to start state in case DUMPSTATE/LOADSTATE/LISTMOVES
	// were used
	state.reset();

	// Player 1 goes first
	current_player = player1;
}

void Agent::switchCurrentPlayer() {
	current_player = (current_player == player1) ? player2 : player1;
}

bool Agent::isValidStartGameMessage(const std::vector<std::string> &tokens) const {
	return tokens.size() == 4 && tokens[0] == "BEGIN" && tokens[1] == "CHINESECHECKERS";
}

bool Agent::isValidMoveMessage(const std::vector<std::string> &tokens) const {
	return tokens.size() == 5 && tokens[0] == "MOVE" && tokens[1] == "FROM" &&
		tokens[3] == "TO";
}