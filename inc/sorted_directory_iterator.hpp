#ifndef SORTED_DIRECTORY_ITERATOR_H
#define SORTED_DIRECTORY_ITERATOR_H

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <cerrno>
#include <iterator>
#include <memory>
#include <stack>
#include <fstream>

namespace boost
{
namespace filesystem
{

namespace detail
{

struct compare_paths
{

bool operator()(directory_entry const& a,
	directory_entry const& b) const noexcept
{
	return a.path().filename().string() < b.path().filename().string();
}

};

}

/*
*
* class sorted_directory_iterator
*
*/

class sorted_directory_iterator
{
public:
	using value_type = directory_entry;
	using difference_type = std::ptrdiff_t;
	using pointer = value_type const*;
	using reference = value_type const&;
	using iterator_category = std::input_iterator_tag;
	using container = std::vector<value_type>;

	sorted_directory_iterator() = default;

	template<typename TCompare = detail::compare_paths>
	sorted_directory_iterator(path const& p,
		TCompare comp = TCompare{})
		: m_impl(new impl)
	{
		directory_iterator iter(p);

		for(auto const& entry : iter) {
			m_impl->paths.emplace_back(entry.path());
		}

		std::sort(m_impl->paths.begin(), m_impl->paths.end(), comp);

		if(m_impl->paths.empty()) {
			m_impl.reset();
		} else {
			m_impl->current = m_impl->paths.cbegin();
		}
	}

	sorted_directory_iterator& increment() noexcept
	{
		++m_impl->current;

		if(m_impl->current == m_impl->paths.cend()) {
			m_impl.reset();
		}

		return *this;
	}

	sorted_directory_iterator& operator++() noexcept
	{
		return increment();
	}

	reference operator* () const noexcept
	{
		return *m_impl->current;
	}

	pointer operator-> () const noexcept
	{
		return &(*m_impl->current);
	}

private:
	struct impl
	{
		container paths;
		container::const_iterator current;
	};

	std::shared_ptr<impl> m_impl;

friend bool operator== (sorted_directory_iterator const& a,
	sorted_directory_iterator const& b) noexcept;  
friend bool operator!= (sorted_directory_iterator const& a,
	sorted_directory_iterator const& b) noexcept;  
};

inline bool operator== (sorted_directory_iterator const& a,
	sorted_directory_iterator const& b) noexcept
{
	return a.m_impl == b.m_impl;
}

inline bool operator!= (sorted_directory_iterator const& a,
	sorted_directory_iterator const& b) noexcept
{
	return !(operator==(a, b));
}

sorted_directory_iterator const& begin(sorted_directory_iterator const& it) noexcept
{
	return it;
}

sorted_directory_iterator end(sorted_directory_iterator const& ) noexcept
{
	return sorted_directory_iterator{};
}

/*
*
* class basic_recursive_sorted_directory_iterator
*
*/

namespace detail
{

inline bool is_access_allowed(path const& p) noexcept
{
	try {
		directory_iterator iter(p);
	} catch(filesystem_error const& e) {
		if(e.code().value() == EACCES) {
			return false;
		}
	} catch(...) {
	}
	return true;
}

}

enum class directory_options
{
	none = 0,
	follow_directory_symlink = 1,
	skip_permission_denied = 2,

	_disable_recursion_pending = 4
};

BOOST_BITMASK(directory_options);

template<typename TCompare = detail::compare_paths>
class basic_recursive_sorted_directory_iterator
{
public:
	using value_type = sorted_directory_iterator::value_type;
	using difference_type = sorted_directory_iterator::difference_type;
	using pointer = sorted_directory_iterator::pointer;
	using reference = sorted_directory_iterator::reference;
	using iterator_category = std::input_iterator_tag;
	using compare = TCompare;

	basic_recursive_sorted_directory_iterator() = default;
	
	basic_recursive_sorted_directory_iterator(
		path const& p,
		directory_options options = directory_options::none,
		compare comp = compare{})
		: m_impl(new impl{p, options, std::move(comp)})
	{
	}

	basic_recursive_sorted_directory_iterator& increment()
	{
		if(m_impl->iter != end(m_impl->iter)) {
			if(recursion_pending()) {
				decltype(auto) path = *m_impl->iter;
				m_impl->hier.emplace(std::move(m_impl->iter));
				m_impl->iter = sorted_directory_iterator(path,
					m_impl->comp);
			} else {
				++m_impl->iter;
			}
		} 

		m_impl->remove(directory_options::_disable_recursion_pending);

		pop_until_valid_or_end();

		return *this;
	}

	basic_recursive_sorted_directory_iterator& operator++ ()
	{
		return increment();
	}

	value_type const& operator* () const noexcept
	{
		return *m_impl->iter;
	}

	value_type const* operator-> () const noexcept
	{
		return m_impl->iter.operator->();
	}

	bool recursion_pending() const
	{
		decltype(auto) path = *m_impl->iter;

		if(m_impl->is_set(
			directory_options::_disable_recursion_pending)) {
			return false;
		} 

		if(!is_directory(path)) {
			return false;
		}

		if(is_symlink(path)) {
			if(m_impl->is_set(
				directory_options::follow_directory_symlink)) {
				return true;
			} else {
				return false;
			}
		}

		if(m_impl->is_set(directory_options::skip_permission_denied) &&
			!detail::is_access_allowed(path)) {
			return false;
		}

		return true;
	}

	int depth() const noexcept
	{
		return m_impl->hier.size();
	}

	void pop() noexcept
	{
		if(depth()) {
			m_impl->iter = sorted_directory_iterator{};

			pop_until_valid_or_end();
		}
	}

	void disable_recursion_pending() noexcept
	{
		m_impl->set(directory_options::_disable_recursion_pending);
	}

	directory_options options() const noexcept
	{
		return m_impl->options;
	}

private:
	void pop_until_valid_or_end() noexcept
	{
		while(m_impl->iter == sorted_directory_iterator{} && depth()) {
			m_impl->iter = std::move(m_impl->hier.top());
			++m_impl->iter;

			m_impl->hier.pop();
		}

		if(m_impl->iter == sorted_directory_iterator{}) {
			m_impl.reset();
		}
	}

	struct impl
	{
		impl(path const& p, directory_options options, compare comp)
			: iter(p, comp), comp(comp), options(options)
		{

		}

		bool is_set(directory_options opt) const noexcept
		{
			return (options & opt) == opt;
		}

		void set(directory_options opt) noexcept
		{
			options |= opt;
		}

		void remove(directory_options opt) noexcept
		{
			options &= ~opt;
		}

		std::stack<sorted_directory_iterator> hier;
		sorted_directory_iterator iter;
		compare comp;
		directory_options options;
	};

	std::shared_ptr<impl> m_impl;

template<typename Compare>
friend bool operator== (basic_recursive_sorted_directory_iterator<Compare> const&,
	basic_recursive_sorted_directory_iterator<Compare> const&) noexcept;
template<typename Compare>
friend bool operator!= (basic_recursive_sorted_directory_iterator<Compare> const&,
	basic_recursive_sorted_directory_iterator<Compare> const&) noexcept;
};

template<typename Compare>
inline bool operator== (basic_recursive_sorted_directory_iterator<Compare> const& a,
	basic_recursive_sorted_directory_iterator<Compare> const& b) noexcept
{
	return a.m_impl == b.m_impl;
}

template<typename Compare>
inline bool operator!= (basic_recursive_sorted_directory_iterator<Compare> const& a,
	basic_recursive_sorted_directory_iterator<Compare> const& b) noexcept
{
	return !(operator==(a, b));
}

template<typename Compare>
inline basic_recursive_sorted_directory_iterator<Compare> const&
	begin(basic_recursive_sorted_directory_iterator<Compare> const& it) noexcept
{
	return it;
}

template<typename Compare>
inline basic_recursive_sorted_directory_iterator<Compare>
	end(basic_recursive_sorted_directory_iterator<Compare> const& ) noexcept
{
	return basic_recursive_sorted_directory_iterator<Compare>{};
}

using recursive_sorted_directory_iterator = basic_recursive_sorted_directory_iterator<>;

}
}

#endif
