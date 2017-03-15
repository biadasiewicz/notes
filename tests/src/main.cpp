#define CATCH_CONFIG_COLOUR_NONE
#define CATCH_CONFIG_RUNNER
#include <catch.hpp>
#include <boost/filesystem.hpp>

int main(int ac, char** av)
{
	//setup...
	boost::filesystem::create_directories("temp");

	//catch run
	int result = Catch::Session().run(ac, av);

	//cleanup...
	boost::filesystem::remove_all("temp");

	return result;
}
