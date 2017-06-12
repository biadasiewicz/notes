#ifndef AUTOSAVE_DAEMON_H
#define AUTOSAVE_DAEMON_H

#include "archive.h"
#include "note.h"
#include <mutex>
#include <vector>
#include <chrono>
#include <atomic>

namespace notes
{
namespace autosave_daemon
{

extern std::atomic<bool> exit_autosave;

void
autosave_func
( std::mutex& mut,
  std::chrono::seconds interval,
  notes::Archive& ar,
  std::vector<Note>& buffer );

}
}

#endif
