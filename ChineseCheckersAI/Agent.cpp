#include "Agent.h"

#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

Agent::Agent() : name("Agent_NJ")
{
    _tree = new Tree<MoveEntry>();
}

Agent::~Agent()
{
}

Move Agent::nextMove()
{
    std::chrono::system_clock::time_point startTime = std::chrono::system_clock::now();
    std::chrono::system_clock::time_point endTime = startTime + std::chrono::milliseconds(SECONDS_PER_TURN * 1000 - 500);
    totalSamples = 0;

    std::vector<Move> stateMoves;
    state.getMoves(stateMoves);

    for (auto& m : stateMoves)
    {
        ChineseCheckersState stateCopy;
        getStateCopy(_tree->getRoot(), stateCopy);
        _tree->getRoot().addChild({ 1, playRandomDepth(stateCopy, m), m });
        ++totalSamples;
    }

    runMonteCarlo(endTime);

    int64_t highestValue = _tree->getRoot()[0].getValue().payout / _tree->getRoot()[0].getValue().samples;
    int highestIndex = 0;
    for (size_t i = 1; i < _tree->getRoot().size(); ++i)
    {
        int64_t value = _tree->getRoot()[i].getValue().payout / _tree->getRoot()[i].getValue().samples;
        std::cerr << _tree->getRoot()[i].getValue().move << " " << _tree->getRoot()[i].getValue().samples << " samples; avg: " << value << std::endl;
        if (value > highestValue)
        {
            highestValue = value;
            highestIndex = i;
        }
    }

    return _tree->getRoot().getValue().move;
}

void Agent::runMonteCarlo(std::chrono::system_clock::time_point endTime)
{
    while (std::chrono::system_clock::now() < endTime)
    {
        runSampling(_tree->getRoot());
    }
}

void Agent::runSampling(Tree<MoveEntry>::TreeNode& node)
{
    if (node.size() > 0)
    {
        // We have children
        float highestValue = calculateUCBValue(node[0].getValue());
        int bestMove = 0;
        for (size_t i = 1; i < node.size(); ++i)
        {
            float value = calculateUCBValue(node[i].getValue());
            if (value > highestValue)
            {
                highestValue = value;
                bestMove = i;
            }
        }

        runSampling(node[bestMove]);
    }
    else
    {
        // We have no children
        if (node.getValue().samples > 5)
        {
            // Expand the node
            ChineseCheckersState stateCopy;
            getStateCopy(node, stateCopy);
            std::vector<Move> moves;
            stateCopy.getMoves(moves);
            for (auto& m : moves)
            {
                node.addChild({ 0, 0, m });
                auto childNode = node[node.size() - 1];
                getStateCopy(childNode, stateCopy);
                simulate(stateCopy, childNode);
            }
        }
        else
        {
            // Simulate the node
            ChineseCheckersState stateCopy;
            getStateCopy(node, stateCopy);
            simulate(stateCopy, node);
        }
    }
}

void Agent::getStateCopy(Tree<MoveEntry>::TreeNode& node, ChineseCheckersState& stateCopy)
{
    stateCopy.currentPlayer = state.currentPlayer;
    for (size_t i = 0; i < state.board.size(); ++i)
    {
        stateCopy.board[i] = state.board[i];
    }

    std::stack<Move> moveStack;
    Tree<MoveEntry>::TreeNode parentNode = node;
    while (!parentNode.isRoot())
    {
        moveStack.push(parentNode.getValue().move);
        parentNode = parentNode.getParent();
    }

    while (!moveStack.empty())
    {
        stateCopy.applyMove(moveStack.top());
        moveStack.pop();
    }
}

void Agent::simulate(ChineseCheckersState& state, Tree<MoveEntry>::TreeNode& node)
{
    int payout = playRandomDepth(state, node.getValue().move);
    node.getValue().payout += payout;
    node.getValue().samples += 1;
    ++totalSamples;

    auto parent = node.getParent();
    while (!parent.isRoot())
    {
        parent.getValue().payout += payout;
        parent.getValue().samples += 1;
        parent = parent.getParent();
    }
}

float Agent::calculateUCBValue(MoveEntry me)
{
    return (me.payout / me.samples) + (float)sqrt((2 * log(totalSamples)) / me.samples);
}

int Agent::playRandomDepth(ChineseCheckersState& state, Move m)
{
    int depth = 0;
    while (!state.gameOver() && depth < maxDepth)
    {
        ++depth;
        std::vector<Move> moves;
        state.getMoves(moves);
        if (rand() % 10 == 0)
        {
            // Play random
            state.applyMove(moves[rand() % moves.size()]);
        }
        else
        {
            // Play best
            Move best = moves[0];
            int bestDistance = calculateMoveDistance(best, state.currentPlayer);

            for (size_t i = 1; i < moves.size(); ++i)
            {
                int distance = calculateMoveDistance(moves[i], state.currentPlayer);
                if (distance > bestDistance)
                {
                    best = moves[i];
                    bestDistance = distance;
                }
            }

            state.applyMove(best);
        }
    }

    if (state.gameOver())
    {
        if (state.winner() - 1 == current_player)
        {
            return INT_MAX;
        }
        else
        {
            return INT_MIN;
        }
    }

    return evaluatePosition(state);
}

int Agent::calculateMoveDistance(Move m, int player)
{
    int fromRow = m.from / 9;
    int fromCol = m.from % 9;
    int toRow = m.to / 9;
    int toCol = m.to % 9;

    if (player == 1)
    {
        return (toRow + toCol) - (fromRow + fromCol);
    }
    else
    {
        return (fromRow + fromCol) - (toRow + toCol);
    }
}

int Agent::evaluatePosition(ChineseCheckersState& state)
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
    maxDepth = depth;
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