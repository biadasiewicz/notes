#include "archive.h"
#include <iostream>

namespace notes
{

void Archive::remove(Iterator it)
{
	m_cont.erase(it);
}

void Archive::serialize(cereal::XMLOutputArchive& ar, std::uint32_t )
{
	ar(m_cont);
}

void Archive::serialize(cereal::XMLInputArchive& ar, std::uint32_t ver)
{
	if(ver <= 1) {
		std::multiset<Note> cont;
		ar(cont);

		m_cont.clear();
		m_cont.insert(cont.begin(), cont.end());
	} else if( ver == 2) {
		ar(m_cont);
	}
}

Archive::Iterator Archive::index(int index) const
{
	if(index < 0) {
		index = size() == 0 ? 0 : size() - 1;
	}

	if(index >= static_cast<int>(size())) {
		throw Error("invalid note index in archive: " +
			std::to_string(index));
	}

	auto it = begin();
	while(index--) {
		++it;
	}
	return it;
}

bool Archive::operator==(Archive const& rhs) const
{
	return m_cont == rhs.m_cont;
}

bool Archive::operator!=(Archive const& rhs) const
{
	return m_cont != rhs.m_cont;
}

std::ostream& operator<< (std::ostream& os, Archive const& ar)
{
	auto it = ar.begin();
	if(it != ar.end()) {
		os << *it;
		++it;
	}

	for(; it != ar.end(); ++it) {
		os << '\n' << *it;
	}

	return os;
}


}
