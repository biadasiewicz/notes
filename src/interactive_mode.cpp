#include "interactive_mode.hpp"
#include "archive.h"
#include "archive_io.h"
#include <boost/filesystem.hpp>
#include <queue>
#include <string>
#include <atomic>
#include <future>
#include <condition_variable>
#include <fstream>
#include <chrono>
#include <iostream>
#include <ctime>

using namespace std;

namespace notes
{

Archive archive;
mutex mut;
atomic<bool> exit_daemon(false);
condition_variable save_request;
boost::filesystem::path path;

void write_daemon()
{
	while(exit_daemon.load() == false) {
		unique_lock<mutex> lock(mut);
		save_request.wait(lock);
		save(archive, path);
	}
}

decltype(auto) prompt(std::string& line)
{
	time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	std::tm* time = localtime(&t);
	cout << time->tm_hour << ":" << time->tm_min << " >>> ";
	return std::getline(cin, line);
}

void interactive_mode()
{
	path = make_path_from_date(time(0));
	if(boost::filesystem::exists(path)) {
		load(archive, path);
	}

	auto task = async(launch::async, write_daemon);

	using Clock = chrono::steady_clock;
	Clock::time_point last_time(Clock::now());
	auto elapsed = Clock::duration();
	auto interval = chrono::seconds(10);

	string line;
	while(prompt(line)) {
		{
			lock_guard<mutex> lock(mut);
			archive.add(std::move(line));
		}

		auto now = Clock::now();
		elapsed += now - last_time;
		last_time = now;
		if(elapsed > interval) {
			elapsed = Clock::duration();
			save_request.notify_one();
		}
	}

	exit_daemon.store(true);
	save_request.notify_one();
	task.get();
}

}
