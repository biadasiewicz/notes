#include <catch.hpp>
#include <fstream>

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
