#include <cstddef>
#include <iostream>

#include "Agent.h"

int main(int argc, char **argv)
{
    std::cerr << "Additional arguments:" << std::endl;
    std::cerr << "--name n                Sets the agents name to what is supplied in 'n'" << std::endl;
    std::cerr << "-n n                    Sets the agents name to what is supplied in 'n'" << std::endl;
    std::cerr << "--depth d               Sets the depth to play randomly to" << std::endl;
    std::cerr << "-d d                    Sets the depth to play randomly to" << std::endl;
    std::cerr << "--verbose               Prints out tree information after each move" << std::endl;
    std::cerr << "-v                      Prints out tree information after each move" << std::endl;
    std::cerr << "--explorationconstant   Sets the exploration constant of the UCB calculation" << std::endl;
    std::cerr << "-e                      Sets the exploration constant of the UCB calculation" << std::endl;

    // Uncomment for an example of playing an AI against itself.
    //#define TRAIN
#ifdef TRAIN
    std::vector<int> depths, explorationConstants;
    depths.push_back(5);
    depths.push_back(10);
    depths.push_back(15);
    depths.push_back(20);
    depths.push_back(25);
    depths.push_back(50);
    depths.push_back(1000); // To the end

    explorationConstants.push_back(10);
    explorationConstants.push_back(15);
    explorationConstants.push_back(20);
    explorationConstants.push_back(25);
    explorationConstants.push_back(30);
    explorationConstants.push_back(35);
    explorationConstants.push_back(40);

    int bestDepth = depths[0];
    int bestExplorationConstant = explorationConstants[0];

    for (int d = 0; d < depths.size(); ++d)
    {
        if (d == 0) continue;
        int player1Wins = 0;
        int player2Wins = 0;
        for (;;)
        {
            if (player1Wins == 5 || player2Wins == 5) break;
            Agent player1, player2;
            player1.setName("Player1");
            player1.setDepth(bestDepth);
            player1.setExplorationConstant(bestExplorationConstant);
            player1.setVerbose(false);
            player1.setSecondsPerTurn(2);
            player2.setName("Player2");
            player2.setDepth(depths[d]);
            player2.setExplorationConstant(bestExplorationConstant);
            player2.setVerbose(false);
            player2.setSecondsPerTurn(2);
            if (Agent::playGame(player1, player2) == Players::player1)
            {
                std::cerr << "Player 1 wins!" << std::endl;
                ++player1Wins;
            }
            else
            {
                std::cerr << "Player 2 wins!" << std::endl;
                ++player2Wins;
            }
        }

        if (player2Wins == 5)
        {
            bestDepth = depths[d];
        }

        std::cerr << (player1Wins == 5 ? "Player 1 wins the set." : "Player 2 wins the set.") << " Winning values: depth " << bestDepth << " exploration constant " << bestExplorationConstant << std::endl;
    }

    for (int e = 0; e < explorationConstants.size(); ++e)
    {
        if (e == 0) continue;
        int player1Wins = 0;
        int player2Wins = 0;
        for (;;)
        {
            if (player1Wins == 5 || player2Wins == 5) break;
            Agent player1, player2;
            player1.setName("Player1");
            player1.setDepth(bestDepth);
            player1.setExplorationConstant(bestExplorationConstant);
            player1.setVerbose(false);
            player2.setName("Player2");
            player2.setDepth(bestDepth);
            player2.setExplorationConstant(explorationConstants[e]);
            player2.setVerbose(false);
            if (Agent::playGame(player1, player2) == Players::player1)
            {
                std::cerr << "Player 1 wins!" << std::endl;
                ++player1Wins;
            }
            else
            {
                std::cerr << "Player 2 wins!" << std::endl;
                ++player2Wins;
            }
        }

        if (player2Wins == 5)
        {
            bestExplorationConstant = explorationConstants[e];
        }

        std::cerr << (player1Wins == 5 ? "Player 1 wins the set." : "Player 2 wins the set.") << " Winning values: depth " << bestDepth << " exploration constant " << bestExplorationConstant << std::endl;
    }

    std::cerr << "Best values found: depth " << bestDepth << " exploration constant " << bestExplorationConstant << std::endl;
    system("pause");
#else
    Agent a;

    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "--name") == 0 || strcmp(argv[i], "-n") == 0)
        {
            ++i;
            if (i < argc)
            {
                a.setName(argv[i]);
                std::cerr << "Setting agent name to " << argv[i] << std::endl;
            }
            else
            {
                std::cerr << "Found argument to set the name, but no name followed." << std::endl;
            }
        }
        else if (strcmp(argv[i], "--depth") == 0 || strcmp(argv[i], "-d") == 0)
        {
            ++i;
            if (i < argc)
            {
                a.setDepth(atoi(argv[i]));
                std::cerr << "Setting depth to " << argv[i] << std::endl;
            }
            else
            {
                std::cerr << "Found argument to set the depth, but no depth followed." << std::endl;
            }
        }
        else if (strcmp(argv[i], "--verbose") == 0 || strcmp(argv[i], "-v") == 0)
        {
            a.setVerbose(true);
            std::cerr << "Setting to verbose mode" << std::endl;
        }
        else if (strcmp(argv[i], "--explorationconstant") == 0 || strcmp(argv[i], "-e") == 0)
        {
            ++i;
            if (i < argc)
            {
                a.setExplorationConstant(atoi(argv[i]));
                std::cerr << "Setting exploration constant to " << argv[i] << std::endl;
            }
            else
            {
                std::cerr << "Found argument to set the exploration constant, but no constant followed." << std::endl;
            }
        }
    }

    a.playGame();
#endif

    return EXIT_SUCCESS;
}