#ifndef USER_H
#define USER_H

#include <boost/filesystem.hpp>

namespace notes
{

class User
{
public:
	User();
	~User();

	void change_user_name(std::string name);
	void set_archive_path(boost::filesystem::path);

	boost::filesystem::path main_path() const;
	boost::filesystem::path archive_path() const;

	std::string user_name() const;

private:
	boost::filesystem::path m_archive_path;
	std::string m_user_name;
};

extern User user_config;

}

#endif
