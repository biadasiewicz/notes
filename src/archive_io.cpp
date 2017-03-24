#include "archive_io.h"
#include "user.h"
namespace fs = boost::filesystem;

namespace notes
{

fs::path
make_path_from_date
( std::string year,
  std::string month,
  std::string day )
{
	std::string filename;
	filename.reserve(year.size() + month.size() + day.size() + 2);
	filename += std::move(year) + "_";
	filename += std::move(month) + "_";
	filename += std::move(day);

	fs::path path = user_config.archive_path();
	path /= filename;

	return path;
}

fs::path make_path_from_date(time_t t)
{
	struct tm* date = localtime(&t);

	using std::to_string;
	return make_path_from_date(to_string(date->tm_year + 1900),
		to_string(date->tm_mon + 1), to_string(date->tm_mday));
}

}
