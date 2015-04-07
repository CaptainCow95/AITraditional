#include <cstddef>

#include "Agent.h"

int main(int argc, char **argv) {
	Agent a;
	if (argc > 1)
	{
		a.setName(argv[1]);
	}

	a.playGame();

	return EXIT_SUCCESS;
}