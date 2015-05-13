#include <cstddef>
#include <iostream>

#include "Agent.h"

int main(int argc, char **argv)
{
    std::cerr << "Additional arguments:" << std::endl;
    std::cerr << "--name n    Sets the agents name to what is supplied in 'n'" << std::endl;
    std::cerr << "--depth d   Sets the depth to play randomly to" << std::endl;
    std::cerr << "--verbose   Prints out tree information after each move" << std::endl;
    std::cerr << "--profile   Forces the program to think a lot longer about each move, useful for profiling" << std::endl;

    Agent a;

    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "--name") == 0)
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
        else if (strcmp(argv[i], "--depth") == 0)
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
        else if (strcmp(argv[i], "--verbose") == 0)
        {
            a.setVerbose();
            std::cerr << "Setting to verbose mode" << std::endl;
        }
        else if (strcmp(argv[i], "--profile") == 0)
        {
            a.setProfile();
            std::cerr << "Setting to profile mode" << std::endl;
        }
    }

    a.playGame();

    return EXIT_SUCCESS;
}