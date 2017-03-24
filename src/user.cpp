#include "user.h"

namespace fs = boost::filesystem;

namespace notes
{

User user_config;

static fs::path app_path()
{
#ifdef NOTES_TESTS
	return "temp";
#else
	fs::path p(std::getenv("HOME"));
	p /= ".notes";
	return p;
#endif
}

static fs::path config_file_path()
{
	return app_path() / "config";
}

static void init()
{
	if(!fs::exists(app_path())) {
		create_directories(app_path());
	}

	if(!fs::exists(config_file_path())) {

		std::ofstream file(config_file_path().string());

		//user name
		file << std::getenv("USER") << '\n';

		//archive path
		auto p = app_path();
		p /= "archive";
		file << p.string() << '\n';
	}
}

User::User()
{
	init();


	std::ifstream file(config_file_path().string());

	std::string temp;

	std::getline(file, temp);
	m_user_name = temp;

	std::getline(file, temp);
	m_archive_path = temp;


	if(!fs::exists(m_archive_path)) {
		fs::create_directories(m_archive_path);
	}
}

User::~User()
{
	try {
		std::ofstream file(config_file_path().string());
		if(!file) {
			throw std::exception{};
		}

		file << m_user_name << '\n';
		file << m_archive_path.string() << '\n';

	} catch(...) {
	}
}

void User::change_user_name(std::string name)
{
	m_user_name = std::move(name);
}

void User::set_archive_path(boost::filesystem::path path)
{
	if(!fs::exists(path)) {
		fs::create_directories(path);
	}

	m_archive_path = std::move(path);
}

fs::path User::main_path() const
{
	return app_path();
}

fs::path User::archive_path() const
{
	return m_archive_path;
}

std::string User::user_name() const
{
	return m_user_name;
}

}
