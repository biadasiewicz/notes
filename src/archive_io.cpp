#include "archive_io.h"
#include "user.h"
#include "archive.h"
#include "errors.h"
#include <cereal/archives/xml.hpp>
#include <cerrno>
#include <cstring>
#include <fstream>
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

template<typename TCereal_archive, typename TStream, typename TError> static
void generic_serialize(Archive& ar, fs::path const& path)
{
	try {
		if(path.empty()) {
			throw Empty_path_error();
		}

		TStream stream(path.string());
		if(!stream) {
			throw std::runtime_error(strerror(errno) +
				std::string(": ") + path.string());
		}

		TCereal_archive archive(stream);

		archive(ar);

	} catch(std::exception& e) {
		throw TError{ e.what() };
	}
}

void load(Archive& ar, boost::filesystem::path const& path)
{
	generic_serialize<cereal::XMLInputArchive,
		std::ifstream, Archive_load_error>(ar, path);
}

void save(Archive& ar, boost::filesystem::path const& path)
{
	generic_serialize<cereal::XMLOutputArchive,
		std::ofstream, Archive_save_error>(ar, path);
}

}
