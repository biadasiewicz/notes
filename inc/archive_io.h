#ifndef ARCHIVE_IO_H
#define ARCHIVE_IO_H

#include "errors.h"
#include <boost/filesystem.hpp>
#include <ctime>

namespace notes
{

class Archive;

class Archive_io_error : public Error
{
public:
	using Error::Error;
};

class Archive_load_error : public Archive_io_error
{
public:
	Archive_load_error(std::string const& msg = "unknown")
		: Archive_io_error(msg)
	{}
};

class Archive_save_error : public Archive_io_error
{
public:
	Archive_save_error(std::string const& msg = "unknown")
		: Archive_io_error(msg)
	{}
};

class Archive_decryption_error : public Archive_load_error
{
public:
	Archive_decryption_error(std::string const& msg = "unknown")
		: Archive_load_error(msg)
	{}
};

class Archive_encryption_error : public Archive_save_error
{
public:
	Archive_encryption_error(std::string const& msg = "unknown")
		: Archive_save_error(msg)
	{}
};

boost::filesystem::path
make_path_from_date
( std::string year,
  std::string month,
  std::string day );

boost::filesystem::path make_path_from_date(time_t t);

void load(Archive& ar, boost::filesystem::path const& path);
void save(Archive& ar, boost::filesystem::path const& path);

boost::filesystem::path parse_date_as_path(std::string const& date);

}

#endif
