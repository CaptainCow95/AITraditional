#include "Agent.h"

#include <cstdlib>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <time.h>
#include <vector>

Agent::Agent() : name("MyName") {}

Agent::~Agent()
{
	for (unsigned i = 0; i < moveVectorCache->size(); ++i)
	{
		delete moveVectorCache->at(i);
		delete bestMoveVectorCache->at(i);
	}

	delete moveVectorCache;
	delete bestMoveVectorCache;
}

Move Agent::nextMove() {
	if (moveVectorCache == NULL)
	{
		moveVectorCache = new std::vector<std::vector<Move>*>();
		bestMoveVectorCache = new std::vector<std::vector<Move>*>();

		moveVectorCache->push_back(new std::vector<Move>());
		bestMoveVectorCache->push_back(new std::vector<Move>());
	}

	time_t startTime;
	time(&startTime);

	Move bestMove;
	Move move;
	unsigned maxDepth = 1;
	operations = 0;

	// Keep trying to go deeper and deeper in the search for the best move until we run out of time
	while (getBestMove(state, 0, maxDepth, startTime + SECONDS_PER_TURN, evaluatePosition(state), move) != TIMEOUT)
	{
		// Store the best move and try to go deeper the next time
		bestMove = move;
		++maxDepth;

		if (moveVectorCache->size() < maxDepth)
		{
			// Allocate a new vector if necessary
			moveVectorCache->push_back(new std::vector<Move>());
			bestMoveVectorCache->push_back(new std::vector<Move>());
		}
	}

	std::cerr << "Reached depth of " << (maxDepth - 1) << std::endl;
	std::cerr << "Processed " << (operations / SECONDS_PER_TURN) << " operations per second" << std::endl;

	return bestMove;
}

int Agent::getBestMove(ChineseCheckersState& state, unsigned depth, unsigned maxDepth, time_t endTime, int positionStrength, Move& move) // move is an out parameter
{
	++operations;
	time_t currentTime;
	time(&currentTime);
	if (currentTime >= endTime)
	{
		// We ran out of time, return a timeout
		return TIMEOUT;
	}

	if (depth >= maxDepth)
	{
		// We are at the max depth, return the current node's value
		//return evaluatePosition(state);
		return positionStrength;
	}

	if (state.gameOver())
	{
		if (state.winner() == my_player + 1)
		{
			// The player won, so return the max value
			return INT_MAX;
		}
		else
		{
			// The player lost, so return the min value
			return INT_MIN;
		}
	}

	std::vector<Move>* moves = moveVectorCache->at(depth);
	state.getMoves(*moves);

	std::vector<Move>* bestMoves = bestMoveVectorCache->at(depth);

	// This will be set on the first iteration
	int total;

	for (unsigned i = 0; i < moves->size(); ++i)
	{
		// Undo where the move is coming from
		int newStrength = positionStrength;
		int moveValue = calculateDistanceToHome(state, moves->at(i).from, state.currentPlayer);
		(state.currentPlayer == my_player + 1) ? newStrength -= moveValue : newStrength += moveValue;

		state.applyMove(moves->at(i));

		// Redo where the move is going to
		moveValue = calculateDistanceToHome(state, moves->at(i).to, state.currentPlayer);
		(state.currentPlayer == my_player + 1) ? newStrength += moveValue : newStrength -= moveValue;
		int value = getBestMove(state, depth + 1, maxDepth, endTime, newStrength, move);
		state.undoMove(moves->at(i));

		if (value == TIMEOUT)
		{
			return TIMEOUT;
		}

		if (i == 0)
		{
			total = value;
			bestMoves->clear();
		}
		else if (state.currentPlayer == my_player + 1)
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

	move = bestMoves->at(rand() % bestMoves->size());
	return total;
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
			int distance = calculateDistanceToHome(state, i, player);
			if (player == my_player + 1)
			{
				total += distance;
			}
			else
			{
				total -= distance;
			}
		}
	}

	return total;
}

int Agent::calculateDistanceToHome(ChineseCheckersState& state, unsigned piece, unsigned player)
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