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
            a.setVerbose();
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

    return EXIT_SUCCESS;
}