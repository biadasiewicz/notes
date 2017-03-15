#include "note.h"
#include <regex>

namespace notes
{

Note::Note(std::string const& note, time_t time_stamp)
	: m_time_stamp(time_stamp)
{
	set_note(note);
}

void Note::set_note(std::string note)
{
	std::regex reg("#[^\\s]+");
	auto beg = std::sregex_iterator(note.begin(), note.end(), reg);
	auto end = std::sregex_iterator();

	Tags temp;
	temp.reserve(std::distance(beg, end));

	for(auto i = beg; i != end; ++i) {
		temp.emplace_back(i->str());
	}

	m_tags = std::move(temp);
	m_note = std::move(note);
}

Note& Note::operator=(std::string note)
{
	set_note(std::move(note));
	return *this;
}

Note& Note::operator+=(std::string note)
{
	std::string temp;
	temp.reserve(this->note().size() + 2 + note.size());
	
	temp = this->note();
	temp += "; ";
	temp += std::move(note);

	set_note(std::move(temp));

	return *this;
}

Note::Tags const& Note::tags() const
{
	return m_tags;
}

std::string const& Note::note() const
{
	return m_note;
}

time_t const& Note::time_stamp() const
{
	return m_time_stamp;
}

Note& Note::operator=(time_t time_stamp)
{
	m_time_stamp = time_stamp;
	return *this;
}

bool Note::operator==(Note const& rhs) const
{
	return m_tags == rhs.m_tags && m_note == rhs.m_note &&
		m_time_stamp == rhs.m_time_stamp;
}

bool Note::operator!=(Note const& rhs) const
{
	return !(this->operator==(rhs));
}

bool Note::operator< (Note const& rhs) const
{
	return m_time_stamp < rhs.m_time_stamp;
}

bool Note::is_tagged(std::string const& tag) const
{
	return std::find(tags().begin(), tags().end(), tag) != tags().end();
}

std::ostream& operator<<(std::ostream& os, Note const& note)
{
	struct tm* date = localtime(&note.time_stamp());
	char buffer[32];

	strftime(buffer, 31, "%Y/%m/%d %H:%M", date);

	os << buffer << "\n\t";

	os << note.note();

	return os;
}

}
