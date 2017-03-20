#ifndef NOTE_H
#define NOTE_H

#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/archives/xml.hpp>
#include <ctime>
#include <vector>
#include <string>
#include <iosfwd>

namespace notes
{

class Note
{
public:
	using Tags = std::vector<std::string>;

	Note() = default;
	Note(std::string const& note, time_t time_stamp = time(0));

	void set_note(std::string note);
	Note& operator=(std::string note);
	Note& operator+=(std::string note);

	Tags const& tags() const;

	std::string const& note() const;

	time_t const& time_stamp() const;
	Note& operator=(time_t time_stamp);

	bool operator==(Note const& rhs) const;
	bool operator!=(Note const& rhs) const;

	bool operator< (Note const& rhs) const;

	template<typename T>
	void serialize(T& archive, std::uint32_t version);

	bool is_tagged(std::string const& tag) const;

private:
	Tags m_tags;
	std::string m_note;
	time_t m_time_stamp = 0;

friend std::ostream& operator<<(std::ostream& os, Note const& note);
};

template<typename T>
void Note::serialize(T& archive, std::uint32_t version)
{
	if(version <= 1) {
		archive(m_tags, m_note, m_time_stamp);
	}
}

std::ostream& operator<<(std::ostream& os, Note const& note);

}

CEREAL_CLASS_VERSION(notes::Note, 1);

#endif
