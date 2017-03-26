#include "user.h"
#include "archive.h"
#include "archive_io.h"
#include "sorted_directory_iterator.hpp"
#include <boost/program_options.hpp>
#include <iostream>
#include <regex>
#include <string>

namespace fs = boost::filesystem;
namespace po = boost::program_options;
using std::string;

void parse_date_and_index(string const& arg, string& date, int& index)
{
	std::regex reg("(.+):(.+)");
	std::smatch match;
	if(!std::regex_match(arg, match, reg)) {
		throw po::error("invalid date and index: " + arg);
	}

	date = match[1];
	index = std::stoi(match[2]);
}

static std::tuple<int,int,int> parse_archive_filename(fs::path const& p)
{
	static std::regex reg("([0-9]+)_([0-9]+)_([0-9]+)");
	std::smatch m;
	decltype(auto) filename = p.filename().string();

	if(!std::regex_match(filename, m, reg)) {
		throw std::logic_error("invalid archive name: " + filename);
	}

	using std::stoi;
	return std::make_tuple(stoi(m[1].str()), stoi(m[2].str()), stoi(m[3].str()));
}

static bool sort_by_date(fs::directory_entry const& p1, fs::directory_entry const& p2)
{
	auto m1 = parse_archive_filename(p1.path());
	auto m2 = parse_archive_filename(p2.path());

	return m1 < m2;
}

void run(int argc, char** argv)
{
	string write;
	string date_to_write;
	string read;
	string edit;
	string remove;

	po::options_description desc("notes application usage");
	desc.add_options()
		("help,h", "print this message")
		("write,w", po::value<string>(&write), "note to write to archive")
		("date,d", po::value<string>(&date_to_write), "combined with --write; at this date note will be written")
		("read,r", po::value<string>(&read), "date that will be readed")
		("edit", po::value<string>(&edit), "date and index in archive that will be edited [date:index]")
		("remove", po::value<string>(&remove), "date and index in archive that will be removed [date:index]")
		;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if(vm.empty() || vm.count("help")) {
		std::cout << desc << std::endl;
	}

	if(vm.count("edit")) {
		if(!vm.count("write")) {
			throw po::required_option("write");
		}

		string date;
		int index;

		parse_date_and_index(edit, date, index);

		fs::path path = notes::parse_date_as_path(date);

		notes::Archive ar;
		notes::load(ar, path);

		auto it = ar.index(index);

		notes::Note temp(*it);

		ar.remove(it);

		temp = write;

		ar.add(std::move(temp));

		notes::save(ar, path);

	} else if(vm.count("remove")) {
		notes::Archive ar;

		string date;
		int index;
		parse_date_and_index(remove, date, index);

		auto path = notes::parse_date_as_path(date);

		notes::load(ar, path);

		auto it = ar.index(index);

		ar.remove(it);

		notes::save(ar, path);

	} else if(vm.count("read")) {
		notes::Archive ar;

		fs::path path;
		if(read[0] == '#') {
			//read.erase(read.begin());

			fs::sorted_directory_iterator iter(
				notes::user_config.archive_path(), sort_by_date);

			notes::Archive ar;
			for(auto const& p : iter) {
				notes::load(ar, p.path());

				for(auto const& note : ar) {
					if(note.is_tagged(read)) {
						std::cout << note << std::endl;
					}
				}
			}


		} else {
			auto path = notes::parse_date_as_path(read);
			notes::load(ar, path);

			std::cout << ar << std::endl;
		}
	} else if(vm.count("write")) {
		notes::Archive ar;
		fs::path path;
		time_t t = time(0);

		if(vm.count("date")) {
			path = notes::parse_date_as_path(date_to_write);
		} else {
			path = notes::make_path_from_date(t);
		}

		if(fs::exists(path)) {
			notes::load(ar, path);
		}

		ar.add(write, t);

		notes::save(ar, path);
	}
}

int main(int argc, char** argv)
{
	try {
		run(argc, argv);
	} catch(po::error const& e) {
		std::cerr << "invalid options: " << e.what() << std::endl;
	} catch(notes::Error const& e) {
		std::cerr << "application error: " << e.what() << std::endl;
	} catch(std::exception const& e) {
		std::cerr << "system error: " << e.what() << std::endl;
	} catch(...) {
		std::cerr << "notes: undefined error" << std::endl;
	}
}
