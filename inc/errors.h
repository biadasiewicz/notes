#ifndef ERRORS_H
#define ERRORS_H

#include <stdexcept>
#include <boost/filesystem/path.hpp>

namespace notes
{

class Error : public std::runtime_error
{
public:
	using std::runtime_error::runtime_error;
};

class Filesystem_error : public Error
{
public:
	Filesystem_error(std::string msg = "filesystem error",
			boost::filesystem::path const& p1 = "",
			boost::filesystem::path const& p2 = "")
		: Error(msg + (p1.empty() ? "" : (": " + p1.string())) +
			(p2.empty() ? "" : (": " + p2.string())))
	{

	}
};

class Empty_path_error : public Filesystem_error
{
public:
	Empty_path_error(std::string msg = "empty path")
		: Filesystem_error(msg)
	{

	}
};

class File_not_exists_error : public Filesystem_error
{
public:
	File_not_exists_error(boost::filesystem::path const& p,
			std::string msg = "file not exists")
		: Filesystem_error(msg, p)
	{

	}
};

}

#endif
