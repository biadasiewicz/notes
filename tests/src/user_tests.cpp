#include <catch.hpp>
#include "user.h"

namespace fs = boost::filesystem;

TEST_CASE("user config file")
{
	{
		notes::User user;

		REQUIRE(fs::exists("temp"));
		REQUIRE(fs::exists(user.main_path()));

		REQUIRE(fs::exists("temp/archive"));
		REQUIRE(fs::exists(user.archive_path()));

		REQUIRE(user.user_name() == std::getenv("USER"));
	}

	std::string new_username = "john";
	fs::path new_ar_path = fs::current_path() / "temp" / "archive2";

	{
		notes::User user;
		user.change_user_name(new_username);
		user.set_archive_path(new_ar_path);

		REQUIRE(user.user_name() == new_username);

		REQUIRE(user.archive_path() == new_ar_path);
		REQUIRE(fs::exists(new_ar_path));
	}


	notes::User user;

	REQUIRE(user.user_name() == new_username);
	REQUIRE(user.archive_path() == new_ar_path);
	REQUIRE(fs::exists(new_ar_path));
}
