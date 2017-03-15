#include <catch.hpp>
#include "note.h"
#include <fstream>

TEST_CASE("note", "[class_Note][tags][time]")
{
	std::vector<std::string> tags{ "#tag", "#tag_2", "#tag3" };

	std::string note_str;
	note_str += "Hello! This ";
	note_str += tags[0];
	note_str += " is test";
	note_str += tags[1];
	note_str += " note.";
	note_str += tags[2];

	auto t = time(0);
	notes::Note note(note_str, t);

	REQUIRE(note.tags() == tags);
	REQUIRE(note.note() == note_str);
	REQUIRE(note.time_stamp() == t);

	char const* path = "temp/note_class_serialization";

	{
		std::ofstream file(path);
		cereal::XMLOutputArchive ar(file);
		ar(note);
	}

	{
		std::ifstream file(path);
		cereal::XMLInputArchive ar(file);
		notes::Note loaded;
		ar(loaded);

		REQUIRE(note != notes::Note{});
		REQUIRE(loaded == note);
	}

	std::string tag_4 = "#tag4";
	note += tag_4;

	auto const& ref_tags = note.tags();

	REQUIRE(find(ref_tags.begin(), ref_tags.end(), tag_4) != ref_tags.end());
	REQUIRE(note.note().find(tag_4) != std::string::npos);

	REQUIRE(note.is_tagged(tag_4));
	REQUIRE_FALSE(note.is_tagged(tag_4 + "xxx"));
}

TEST_CASE("note <", "[class_Note]")
{
	notes::Note a("msg", 100), b("msg", 101);

	REQUIRE(a != b);
	REQUIRE(a < b);
}
