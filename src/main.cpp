#include "user.h"
#include "archive.h"
#include "archive_io.h"
#include "sorted_directory_iterator.hpp"
#include <boost/program_options.hpp>
#include <iostream>
#include <regex>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <atomic>
#include <condition_variable>

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

void edit_func(string const& edit, string const& write)
{
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
}

void remove_func(string const& remove)
{
	notes::Archive ar;

	string date;
	int index;
	parse_date_and_index(remove, date, index);

	auto path = notes::parse_date_as_path(date);

	notes::load(ar, path);

	auto it = ar.index(index);

	ar.remove(it);

	notes::save(ar, path);
}

void read_func(string const& read)
{
	notes::Archive ar;

	fs::path path;
	if(read[0] == '#') {
		fs::sorted_directory_iterator iter(
			notes::user_config.archive_path(),
				notes::sort_archive_by_date);

		notes::Archive ar;
		bool found = false;
		for(auto const& p : iter) {
			notes::load(ar, p.path());

			for(auto const& note : ar) {
				if(note.is_tagged(read)) {
					found = true;
					std::cout << note << std::endl;
				}
			}
		}

		if(!found) {
			std::cout << "tag not found: " << read << std::endl;
		}


	} else {
		if(read == "all") {
			fs::sorted_directory_iterator iter(
				notes::user_config.archive_path(),
				notes::sort_archive_by_date);

			for(auto const& p : iter) {
				notes::Archive ar;
				notes::load(ar, p.path());
				std::cout << ar << std::endl;
			}
		} else {
			auto path = notes::parse_date_as_path(read);
			notes::load(ar, path);

			std::cout << ar << std::endl;
		}
	}
}

void backup_func(fs::path const& dest)
{
	if(!fs::exists(dest)) {
		fs::create_directories(dest);
	}

	auto source = notes::user_config.archive_path();
	fs::sorted_directory_iterator iter(source);

	for(auto const& p : iter) {
		fs::copy(p.path(), dest / p.path().filename());
	}
}

namespace autosave
{

using Clock = std::chrono::system_clock;
using Time_point = Clock::time_point;

std::mutex mut;
std::condition_variable var;
std::atomic<bool> exit{ false };

void autosave_func(notes::Archive& ar, std::vector<notes::Note>& buffer)
{
	while(exit.load() == false) {
		std::unique_lock<std::mutex> lock{ mut };
		var.wait(lock);
		for(auto& note : buffer) {
			ar.add(std::move(note));
		}
		buffer.clear();
		std::cout << "buffer..." << std::endl;
	}
	std::cout << "end buffer..." << std::endl;
}

}

void run(int argc, char** argv)
{
	string write;
	string date_to_write;
	string read;
	string edit;
	string remove;
	string backup;

	po::options_description desc("notes application usage");
	desc.add_options()
		("help,h", "print this message")
		("write,w", po::value<string>(&write), "note to write to archive")
		("date,d", po::value<string>(&date_to_write), "combined with --write; at this date note will be written")
		("read,r", po::value<string>(&read), "date that will be readed")
		("edit", po::value<string>(&edit), "date and index in archive that will be edited [date:index]")
		("remove", po::value<string>(&remove), "date and index in archive that will be removed [date:index]")
		("backup,b", po::value<string>(&backup), "backup archive at specified path")
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

		edit_func(edit, write);
	} else if(vm.count("remove")) {
		remove_func(remove);
	} else if(vm.count("read")) {
		read_func(read);
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
	} else if(vm.count("backup")) {
		backup_func(backup);
	} else if(vm.count("interactive")) {
		fs::path path = notes::make_path_from_date(time(0));
		notes::Archive ar;

		if(fs::exists(path)) {
			notes::load(ar, path);
		}

		std::vector<notes::Note> buffer;

		std::thread autosave_thread{ autosave::autosave_func,
			std::ref(ar), std::ref(buffer) };

		using namespace std::chrono;

		seconds interval{ 60 };
		autosave::Time_point last_time{ autosave::Clock::now() };
		duration<float> elapsed{};

		string line;
		while(getline(std::cin, line)) {
			{
				std::lock_guard<std::mutex> lock{ autosave::mut };
				buffer.emplace_back(std::move(line), time(0));
			}


			auto now = autosave::Clock::now();
			elapsed += duration_cast<duration<float>>(now - last_time);
			if(elapsed > interval) {
				elapsed = decltype(elapsed){};
				autosave::var.notify_all();
				std::cout << "elapsed..." << std::endl;
			}
			last_time = now;
		}

		autosave::exit.store(true);
		autosave::var.notify_all();
		autosave_thread.join();

		notes::save(ar, path);
		fs::remove(path);

		std::cout << ar << std::endl;
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
