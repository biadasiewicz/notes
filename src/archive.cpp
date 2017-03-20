#include "archive.h"

namespace notes
{

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

}
