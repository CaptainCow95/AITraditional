#include <cstddef>
#include <iostream>

#include "Agent.h"

int main(int argc, char **argv)
{
	std::cerr << "Additional arguments:" << std::endl;
	std::cerr << "--name n    Sets the agents name to what is supplied in 'n'" << std::endl;
	std::cerr << "--debug     Turns on debug mode for the agent" << std::endl;
	std::cerr << "--depth d   Sets the depth that the agent will search to to 'd', ignoring all time limits" << std::endl;

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
		else if (strcmp(argv[i], "--debug") == 0)
		{
			a.setDebug();
			std::cerr << "Turning on debug mode" << std::endl;
		}
		else if (strcmp(argv[i], "--depth") == 0)
		{
			++i;
			if (i < argc)
			{
				int depth = atoi(argv[i]);
				if (depth > 0)
				{
					a.setDepth(depth);
					std::cerr << "Setting depth to " << depth << std::endl;
				}
				else
				{
					std::cerr << "--depth was given an invalid number. Only numbers greater than 0 are accepted." << std::endl;
				}
			}
			else
			{
				std::cerr << "Found argument to set the depth, but no depth followed." << std::endl;
			}
		}
	}

	a.playGame();

	return EXIT_SUCCESS;
}