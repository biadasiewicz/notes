#include <catch.hpp>
#include <fstream>
#include <iostream>

#include "archive.h"

bool find(notes::Archive const& ar, notes::Note const& n)
{
	return std::find(ar.begin(), ar.end(), n) != ar.end();
}

TEST_CASE("archive serialization", "[class_Archive][serialization]")
{
	std::string path("temp/archive_serialization");

	notes::Note n1("note 1", 1);
	notes::Note n2("note 2", 1);
	notes::Note n3("note 3", 2);

	notes::Archive ar;
	ar.add(n1);
	ar.add(n2);
	ar.add(n3);

	SECTION("serialization") {
		{
			std::ofstream file(path);
			cereal::XMLOutputArchive oa(file);
			oa(ar);
		}

		{
			notes::Archive loaded_ar;

			std::ifstream file(path);
			cereal::XMLInputArchive ia(file);
			ia(loaded_ar);

			REQUIRE(ar.size() == loaded_ar.size());
			REQUIRE(find(loaded_ar, n1));
			REQUIRE_FALSE(find(loaded_ar, n2));
			REQUIRE(find(loaded_ar, n3));
		}
	}

	SECTION("finding") {
		auto it = ar.find_if([](auto const& elem)
			{ return elem.time_stamp() == 1; });

		REQUIRE(it != ar.end());

		it = ar.find_if([](auto const& elem)
			{ return elem.time_stamp() == 3; });

		REQUIRE(it == ar.end());

		auto index = ar.index(0);
		REQUIRE(index != ar.end());

		auto last = ar.index();
		REQUIRE(index != ar.end());

		REQUIRE_THROWS(ar.index(123));
	}

	SECTION("removing") {
		auto size = ar.size();

		ar.remove_if([](auto const& x)
			{ return x.time_stamp() == 1; });

		REQUIRE(size != ar.size());

		--size;

		auto it = ar.begin();
		ar.remove(it);

		REQUIRE(size != ar.size());
		REQUIRE(ar.empty());
	}

	/*
	 *SECTION("printing") {
	 *        std::cout << ar << std::endl;
	 *}
	 */

}
