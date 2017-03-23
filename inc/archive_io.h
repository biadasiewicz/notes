#ifndef ARCHIVE_IO_H
#define ARCHIVE_IO_H

#include "errors.h"
#include "archive.h"

namespace notes
{

class Archive_io_error : public Error
{
public:
	using Error::Error;
};

class Archive_load_error : public Archive_io_error
{
public:
	Archive_load_error(std::string const& msg)
		: Archive_io_error("failed to laod archive: " + msg)
	{}
};

class Archive_save_error : public Archive_io_error
{
public:
	Archive_save_error(std::string const& msg)
		: Archive_io_error("failed to save archive: " + msg)
	{}
};

class Archive_decryption_error : public Archive_load_error
{
public:
	Archive_decryption_error(std::string const& msg)
		: Archive_load_error("error while decrypting: " + msg)
	{}
};

class Archive_encryption_error : public Archive_save_error
{
public:
	Archive_encryption_error(std::string const& msg)
		: Archive_save_error("error while encrypting: " + msg)
	{}
};

}

#endif
