#include "autosave_daemon.h"

using namespace std::chrono;
using namespace std::chrono_literals;
using Clock = system_clock;
using Time_point = Clock::time_point;


namespace notes
{
namespace autosave_daemon
{

std::atomic<bool> exit_autosave{ false };
static Time_point last_time{ Clock::now() };

void
autosave_func
( std::mutex& mut,
  std::chrono::seconds interval,
  notes::Archive& ar,
  std::vector<Note>& buffer )
{
	duration<float> elapsed{};
	while(true) {
		auto now = Clock::now();
		elapsed += duration_cast<duration<float>>(now - last_time);

		if(elapsed > interval) {
			elapsed = decltype(elapsed){};
			if(!buffer.empty()) {
				std::lock_guard<std::mutex> lock(mut);

				for(auto it = buffer.begin(); it != buffer.end(); ++it) {
					ar.add(std::move(*it));
				}

				buffer.clear();
			}
		}
		last_time = now;

		if(exit_autosave.load()) break;
	}
}

}
}
