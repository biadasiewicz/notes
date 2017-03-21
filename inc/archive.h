#ifndef ARCHIVE_H
#define ARCHIVE_H

#include "note.h"
#include "errors.h"
#include <cereal/cereal.hpp>
#include <cereal/types/set.hpp>
#include <cereal/archives/xml.hpp>
#include <set>
#include <algorithm>

namespace notes
{

class Archive
{
public:
	using Container = std::set<Note>;
	using Iterator = Container::const_iterator;

	template<typename... Args>
	void add(Args&&... args);

	template<typename TPredicate>
	Iterator find_if(TPredicate pred) const;

	template<typename TPredicate>
	void remove_if(TPredicate pred);

	void serialize(cereal::XMLOutputArchive& ar, std::uint32_t ver);
	void serialize(cereal::XMLInputArchive& ar, std::uint32_t ver);

	Iterator begin() const;
	Iterator end() const;

	Container::size_type size() const noexcept;

private:
	Container m_cont;
};

template<typename... Args>
void Archive::add(Args&&... args)
{
	try {
		m_cont.emplace(std::forward<Args>(args)...);
	} catch(...) {
		throw Error("failed to add new note to archive");
	}
}

template<typename TPredicate>
Archive::Iterator Archive::find_if(TPredicate pred) const
{
	return std::find_if(begin(), end(), pred);
}

template<typename TPredicate>
void Archive::remove_if(TPredicate pred)
{
	for(auto it = begin(); it != end(); ++it) {
		if(pred(*it)) {
			m_cont.erase(it);
		}
	}
}

inline Archive::Iterator Archive::begin() const
{
	return m_cont.cbegin();
}

inline Archive::Iterator Archive::end() const
{
	return m_cont.cend();
}

inline Archive::Container::size_type Archive::size() const noexcept
{
	return m_cont.size();
}

}

CEREAL_CLASS_VERSION( notes::Archive, 2 );

#endif
