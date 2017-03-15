#define CATCH_CONFIG_COLOUR_NONE
#define CATCH_CONFIG_RUNNER
#include <catch.hpp>

int main(int ac, char** av)
{
	//setup...

	//catch run
	int result = Catch::Session().run(ac, av);

	//cleanup...

	return result;
}
