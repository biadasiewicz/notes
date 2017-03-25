#include <catch.hpp>
#include "archive_io.h"
#include "archive.h"
#include "user.h"

TEST_CASE("archive filename")
{
	constexpr auto end = std::string::npos;
	auto archive_path = notes::user_config.archive_path();


	std::string year = "2017";
	std::string month = "3";
	std::string day = "24";
	auto p1 = notes::make_path_from_date(year, month, day);

	REQUIRE(p1.parent_path() == archive_path);
	REQUIRE(p1.string().find(year) != end);
	REQUIRE(p1.string().find(month) != end);
	REQUIRE(p1.string().find(day) != end);


	auto t = time(0);
	auto p2 = notes::make_path_from_date(t);

	REQUIRE(!p2.empty());
	REQUIRE(p2.parent_path() == archive_path);
}

TEST_CASE("archive save/load")
{
	auto path = notes::make_path_from_date(time(0));

	notes::Archive ar;
	ar.add("note 1", 100000);
	ar.add("note 2", 900000);
	ar.add("note 3", 9900000);

	SECTION("load and save") {
		notes::save(ar, path);

		notes::Archive loaded;
		REQUIRE(loaded != ar);

		notes::load(loaded, path);
		REQUIRE(ar == loaded);
	}
}
