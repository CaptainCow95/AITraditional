#include <cstddef>
#include <iostream>

#include "Agent.h"

int main(int argc, char **argv)
{
    std::cerr << "Additional arguments:" << std::endl;
    std::cerr << "--name n    Sets the agents name to what is supplied in 'n'" << std::endl;

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
    }

    a.playGame();

    return EXIT_SUCCESS;
}