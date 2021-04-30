#pragma once
#include <sqlite3.hpp>

namespace Arcollect {
	namespace db {
		std::unique_ptr<SQLite3::sqlite3> open(int flags = SQLite3::OPEN_READWRITE|SQLite3::OPEN_CREATE);
	}
}

