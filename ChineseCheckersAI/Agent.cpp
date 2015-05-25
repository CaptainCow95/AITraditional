#include "Agent.h"

#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>
#include <stack>

Agent::Agent() : name("Agent_NJ")
{
    threadPool = new ThreadPool();
    moveVectorCache = new std::vector<std::vector<Move>*>();
    bestMoveVectorCache = new std::vector<std::vector<Move>*>();
    moveVectorCache->push_back(new std::vector<Move>());
    bestMoveVectorCache->push_back(new std::vector<Move>());
}

Agent::~Agent()
{
    delete threadPool;
    for (unsigned i = 0; i < moveVectorCache->size(); ++i)
    {
        delete moveVectorCache->at(i);
        delete bestMoveVectorCache->at(i);
    }

    delete moveVectorCache;
    delete bestMoveVectorCache;
}

void Agent::applyNodeToState(MoveTree::MoveTreeNode* node, ChineseCheckersState& stateCopy)
{
    if (!node->isRoot())
    {
        applyNodeToState(node->getParent(), stateCopy);
    }
    else
    {
        return;
    }

    stateCopy.applyMove(node->getMove());
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

float Agent::calculateUCBValue(int samples, int64_t payout)
{
    // Calculate an approximation of the natural log since it is a lot faster.
    // Bit twiddling log base 2 taken from https://graphics.stanford.edu/~seander/bithacks.html#IntegerLogIEEE64Float
    union { unsigned int u[2]; double d; } t; // temp
    t.u[0] = totalSamples;
    t.u[1] = 0x43300000;
    t.d -= 4503599627370496.0;
    // Multiply by the constant at the end to get from log base 2 to ln
    // Constant taken from http://www.flipcode.com/archives/Fast_log_Function.shtml
    float logResult = ((t.u[1] >> 20) - 0x3FF) * 0.69314718f;

    // Calculate the sqrt root using some floating point and integer bit twiddling
    // Bit twiddling sqrt taken from https://en.wikipedia.org/wiki/Methods_of_computing_square_roots#Approximations_that_depend_on_the_floating_point_representation
    float sqrtInputTemp = logResult / (float)samples;
    int sqrtTemp = *(int*)&sqrtInputTemp;
    sqrtTemp = (1 << 29) + (sqrtTemp >> 1) - (1 << 22) + -0x4C000;
    float sqrtResult = *(float*)&sqrtTemp;

    float fast = payout / (float)samples + explorationConstant * sqrtResult;
    return fast;
}

int Agent::evaluatePosition(ChineseCheckersState& state)
{
    int total = 0;
    for (int i = 0; i < 81; ++i)
    {
        if (state.board[i] != 0)
        {
            int player = state.board[i];
            int distance = calculateDistanceToHome(i, my_player + 1);
            if (player == my_player + 1)
            {
                switch (distance)
                {
                case 0:
                    total -= 0;
                    break;
                case 1:
                case 2:
                    total -= 1;
                    break;
                case 3:
                case 4:
                    total -= 2;
                    break;
                case 5:
                    total -= 3;
                    break;
                case 6:
                    total -= 4;
                    break;
                case 7:
                    total -= 5;
                    break;
                case 8:
                    total -= 7;
                    break;
                case 9:
                    total -= 9;
                    break;
                case 10:
                    total -= 11;
                    break;
                case 11:
                    total -= 14;
                    break;
                case 12:
                    total -= 17;
                    break;
                case 13:
                    total -= 21;
                    break;
                default:
                    total -= 25;
                    break;
                }
            }
            else
            {
                total -= distance;
            }
        }
    }

    return total;
}

void Agent::getStateCopy(MoveTree::MoveTreeNode* node, ChineseCheckersState& stateCopy)
{
    stateCopy.currentPlayer = state.currentPlayer;
    for (size_t i = 0; i < state.board.size(); ++i)
    {
        stateCopy.board[i] = state.board[i];
    }

    applyNodeToState(node, stateCopy);
}

bool Agent::isValidMoveMessage(const std::vector<std::string>& tokens) const
{
    return tokens.size() == 5 && tokens[0] == "MOVE" && tokens[1] == "FROM" &&
        tokens[3] == "TO";
}

bool Agent::isValidStartGameMessage(const std::vector<std::string>& tokens) const
{
    return tokens.size() == 4 && tokens[0] == "BEGIN" && tokens[1] == "CHINESECHECKERS";
}

Move Agent::nextMove()
{
    std::chrono::system_clock::time_point startTime = std::chrono::system_clock::now();
    endTime = startTime + std::chrono::milliseconds(secondsPerTurn * 1000 - 800);
    totalSamples = 0;
    deepestDepth = 0;

    bool doMiniMax = true;
    for (unsigned i = 0; i < 81; ++i)
    {
        if (my_player == player1)
        {
            if (state.board[i] == 1)
            {
                int row = i / 9;
                int col = i % 9;

                if (row + col < 9)
                {
                    doMiniMax = false;
                    break;
                }
            }
        }
        else
        {
            if (state.board[i] == 2)
            {
                int row = i / 9;
                int col = i % 9;

                if (row + col > 7)
                {
                    doMiniMax = false;
                    break;
                }
            }
        }
    }

    if (doMiniMax)
    {
        unsigned maxDepth = 1;
        Move move, bestMove;
        int moveValue = runMiniMax(0, maxDepth, endTime, move);
        bestMove = move;
        while (moveValue != TIMEOUT && moveValue != WIN)
        {
            bestMove = move;
            ++maxDepth;

            if (moveVectorCache->size() < maxDepth)
            {
                moveVectorCache->push_back(new std::vector<Move>());
                bestMoveVectorCache->push_back(new std::vector<Move>());
            }

            moveValue = runMiniMax(0, maxDepth, endTime, move);
        }

        if (verbose != -1)
        {
            std::cerr << "Reached a depth of " << maxDepth << std::endl;
        }

        return bestMove;
    }
    else
    {
        tree = new MoveTree();

        std::vector<Move> stateMoves;
        state.getMoves(stateMoves);
        std::vector<MoveTree::MoveTreeNode*>* nodeChildren = new std::vector<MoveTree::MoveTreeNode*>();

        for (auto& m : stateMoves)
        {
            ChineseCheckersState stateCopy;
            getStateCopy(tree->getRoot(), stateCopy);
            auto payout = playRandomDepth(stateCopy);
            nodeChildren->push_back(new MoveTree::MoveTreeNode(1, payout, calculateUCBValue(1, payout), m, tree->getRoot()));
            ++totalSamples;
        }

        tree->getRoot()->addChildren(nodeChildren);

        size_t numThreads = threadPool->getNumThreads();
        std::function<void(void*)> runMonteCarloFunction = bind(&Agent::runMonteCarlo, this, std::placeholders::_1);
        for (size_t i = 0; i < numThreads; ++i)
        {
            threadPool->queueJob(runMonteCarloFunction, nullptr);
        }

        while (threadPool->getQueuedJobs() > 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        auto node = tree->getRoot()->get(0);
        int64_t highestValue = node->payout / node->samples;
        int highestIndex = 0;

        if (verbose == 1)
        {
            std::cerr << node->getMove() << " " << node->samples << " samples; avg: " << highestValue << std::endl;
        }

        for (unsigned i = 1; i < tree->getRoot()->getSize(); ++i)
        {
            node = tree->getRoot()->get(i);
            int64_t value = node->payout / node->samples;
            if (value > highestValue)
            {
                highestValue = value;
                highestIndex = i;
            }

            if (verbose == 1)
            {
                std::cerr << node->getMove() << " " << node->samples << " samples; avg: " << value << std::endl;
            }
        }

        if (verbose != -1)
        {
            std::cerr << "Deepest depth: " << deepestDepth << " Total simulations: " << totalSamples << std::endl;
        }

        Move retMove = tree->getRoot()->get(highestIndex)->getMove();
        delete tree;
        return retMove;
    }
}

Players Agent::playGame(Agent& player1, Agent& player2)
{
    player1.my_player = Players::player1;
    player1.current_player = Players::player1;
    player2.my_player = Players::player2;
    player2.current_player = Players::player1;

    for (;;)
    {
        if (player1.state.gameOver())
        {
            return Players::player2;
        }

        Move m = player1.nextMove();
        player1.state.applyMove(m);
        player1.switchCurrentPlayer();
        player2.state.applyMove(m);
        player2.switchCurrentPlayer();

        if (player2.state.gameOver())
        {
            return Players::player1;
        }

        m = player2.nextMove();
        player1.state.applyMove(m);
        player1.switchCurrentPlayer();
        player2.state.applyMove(m);
        player2.switchCurrentPlayer();
    }
}

void Agent::playGame()
{
    // Identify myself
    std::cout << "#name " << name << std::endl;

    // Wait for start of game
    waitForStart();

    // Main game loop
    for (;;)
    {
        if (current_player == my_player)
        {
            // My turn

            // Check if game is over
            if (state.gameOver())
            {
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
        else
        {
            // Wait for move from other player
            // Get server's next instruction
            std::string server_msg = readMsg();
            const std::vector<std::string> tokens = tokenizeMsg(server_msg);

            if (tokens.size() == 5 && tokens[0] == "MOVE")
            {
                // Translate to local coordinates and update our local state
                const Move m = state.translateToLocal(tokens);
                state.applyMove(m);

                // It is now my turn
                switchCurrentPlayer();
            }
            else if (tokens.size() == 4 && tokens[0] == "FINAL" &&
                tokens[2] == "BEATS")
            {
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
            else
            {
                // Unknown command
                std::cerr << "Unknown command of '" << server_msg << "' from the server"
                    << std::endl;
            }
        }
    }
}

int Agent::playRandomDepth(ChineseCheckersState& state)
{
    int depth = 0;
    while (!state.gameOver() && depth < maxDepth)
    {
        ++depth;
        std::vector<Move> moves;
        moves.reserve(500);
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
            return WIN;
        }
        else
        {
            return LOSE;
        }
    }

    return evaluatePosition(state);
}

// Sends a msg to stdout and verifies that the next message to come in is it
// echoed back. This is how the server validates moves
void Agent::printAndRecvEcho(const std::string& msg) const
{
    // Note the endl flushes the stream, which is necessary
    std::cout << msg << std::endl;
    const std::string echo_recv = readMsg();
    if (msg != echo_recv)
        std::cerr << "Expected echo of '" << msg << "'. Received '" << echo_recv
        << "'" << std::endl;
}

// Reads a line, up to a newline from the server
std::string Agent::readMsg() const
{
    std::string msg;
    std::getline(std::cin, msg); // This is a blocking read

    // Trim white space from beginning of string
    const char* WhiteSpace = " \t\n\r\f\v";
    msg.erase(0, msg.find_first_not_of(WhiteSpace));
    // Trim white space from end of string
    msg.erase(msg.find_last_not_of(WhiteSpace) + 1);

    return msg;
}

int Agent::runMiniMax(unsigned depth, unsigned maxDepth, std::chrono::system_clock::time_point& endTime, Move& move)
{
    if (depth >= maxDepth)
    {
        return evaluatePosition(state);
    }

    if (std::chrono::system_clock::now() >= endTime)
    {
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

    std::vector<Move>* moves = moveVectorCache->at(depth);
    state.getMoves(*moves);
    std::vector<Move>* bestMoves = bestMoveVectorCache->at(depth);
    bestMoves->clear();
    int bestValue = LOSE;

    for (unsigned i = 0; i < moves->size(); ++i)
    {
        Move moveToMake = moves->at(i);
        int fromRow = moveToMake.from / 9;
        int fromCol = moveToMake.from % 9;
        int toRow = moveToMake.to / 9;
        int toCol = moveToMake.to % 9;

        if (my_player == player1)
        {
            if (fromRow + fromCol > toRow + toCol)
            {
                continue;
            }
        }
        else
        {
            if (toRow + toCol > fromRow + fromCol)
            {
                continue;
            }
        }

        state.applyMove(moveToMake);
        state.swapTurn();

        int value = runMiniMax(depth + 1, maxDepth, endTime, move);

        state.swapTurn();
        state.undoMove(moveToMake);

        if (value == TIMEOUT)
        {
            return TIMEOUT;
        }

        if (value > bestValue)
        {
            bestValue = value;
            bestMoves->clear();
        }

        if (value == bestValue)
        {
            bestMoves->push_back(moves->at(i));
        }
    }

    move = bestMoves->at(rand() % bestMoves->size());
    return bestValue;
}

void Agent::runMonteCarlo(void*)
{
    while (std::chrono::system_clock::now() < endTime)
    {
        auto node = tree->getRoot();
        int depth = 0;
        while (node->getSize() > 0)
        {
            // We have children
            auto child = node->get(0);
            float highestValue = child->ucbValue;
            int bestMove = 0;
            for (unsigned i = 1; i < node->getSize(); ++i)
            {
                child = node->get(i);
                float value = child->ucbValue;
                if (value > highestValue)
                {
                    highestValue = value;
                    bestMove = i;
                }
            }

            ++depth;
            node = node->get(bestMove);
        }

        deepestDepth = std::max(deepestDepth, depth);

        // We have no children
        if (node->samples > 10)
        {
            node->enterLock();

            if (node->addingChildren || node->getSize() > 0)
            {
                // Simulate the node
                node->exitLock();
                simulate(node);
            }
            else
            {
                // Expand the node
                node->addingChildren = true;
                node->exitLock();

                ChineseCheckersState stateCopy;
                getStateCopy(node, stateCopy);
                std::vector<Move> moves;
                stateCopy.getMoves(moves);
                std::vector<MoveTree::MoveTreeNode*>* children = new std::vector<MoveTree::MoveTreeNode*>();
                for (auto& m : moves)
                {
                    auto payout = simulate(node, m);
                    children->push_back(new MoveTree::MoveTreeNode(1, payout, calculateUCBValue(1, payout), m, node));
                }

                node->addChildren(children);
            }
        }
        else
        {
            // Simulate the node
            simulate(node);
        }
    }
}

void Agent::setDepth(int depth)
{
    maxDepth = depth;
}

void Agent::setExplorationConstant(int constant)
{
    explorationConstant = constant;
}

void Agent::setName(std::string newName)
{
    name = newName;
}

void Agent::setSecondsPerTurn(int value)
{
    secondsPerTurn = value;
}

void Agent::setVerbose(bool value)
{
    if (value)
    {
        verbose = 1;
    }
    else
    {
        verbose = -1;
    }
}

int Agent::simulate(MoveTree::MoveTreeNode* node)
{
    node->samples += 1;
    int payout = simulate(node->getParent(), node->getMove());
    node->payout += payout;
    node->ucbValue = calculateUCBValue(node->samples, node->payout);
    return payout;
}

int Agent::simulate(MoveTree::MoveTreeNode* node, Move m)
{
    node->samples += 1;
    ++totalSamples;

    auto parent = node->getParent();
    while (parent != nullptr && !parent->isRoot())
    {
        parent->samples += 1;
        parent = parent->getParent();
    }

    ChineseCheckersState stateCopy;
    getStateCopy(node, stateCopy);
    stateCopy.applyMove(m);

    int payout = playRandomDepth(stateCopy);
    node->payout += payout;
    node->ucbValue = calculateUCBValue(node->samples, node->payout);

    parent = node->getParent();
    while (parent != nullptr && !parent->isRoot())
    {
        parent->payout += payout;
        parent->ucbValue = calculateUCBValue(parent->samples, parent->payout);
        parent = parent->getParent();
    }

    return payout;
}

void Agent::switchCurrentPlayer()
{
    current_player = (current_player == player1) ? player2 : player1;
}
// Tokenizes a message based upon whitespace
std::vector<std::string> Agent::tokenizeMsg(const std::string& msg) const
{
    // Tokenize using whitespace as a delimiter
    std::stringstream ss(msg);
    std::istream_iterator<std::string> begin(ss);
    std::istream_iterator<std::string> end;
    std::vector<std::string> tokens(begin, end);

    return tokens;
}

void Agent::waitForStart()
{
    for (;;)
    {
        std::string response = readMsg();
        const std::vector<std::string> tokens = tokenizeMsg(response);

        if (tokens.size() == 4 && tokens[0] == "BEGIN" &&
            tokens[1] == "CHINESECHECKERS")
        {
            // Found BEGIN GAME message, determine if we play first
            if (tokens[2] == name)
            {
                // We go first!
                opp_name = tokens[3];
                my_player = player1;
                break;
            }
            else if (tokens[3] == name)
            {
                // They go first
                opp_name = tokens[2];
                my_player = player2;
                break;
            }
            else
            {
                std::cerr << "Did not find '" << name
                    << "', my name, in the BEGIN command.\n"
                    << "# Found '" << tokens[2] << "' and '" << tokens[3] << "'"
                    << " as player names. Received message '" << response << "'"
                    << std::endl;
                std::cout << "#quit" << std::endl;
                std::exit(EXIT_FAILURE);
            }
        }
        else if (response == "DUMPSTATE")
        {
            std::cout << state.dumpState() << std::endl;
        }
        else if (tokens[0] == "LOADSTATE")
        {
            std::string new_state = response.substr(10);
            if (!state.loadState(new_state))
                std::cerr << "Failed to load '" << new_state << "'\n";
        }
        else if (response == "LISTMOVES")
        {
            std::vector<Move> moves;
            state.getMoves(moves);
            for (const auto i : moves)
                std::cout << i.from << ", " << i.to << "; ";
            std::cout << std::endl;
        }
        else if (tokens[0] == "MOVE")
        {
            // Just apply the move
            const Move m = state.translateToLocal(tokens);
            if (!state.applyMove(m))
                std::cout << "Unable to apply move " << m << std::endl;
        }
        else if (tokens[0] == "PROFILE")
        {
            for (;;)
            {
                nextMove();
            }
        }
        else
        {
            std::cerr << "Unexpected message " << response << "\n";
        }
    }

    // Game is about to begin, restore to start state in case DUMPSTATE/LOADSTATE/LISTMOVES
    // were used
    state.reset();

    // Player 1 goes first
    current_player = player1;
}