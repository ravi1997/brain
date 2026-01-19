#pragma once

// Common STL
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <mutex>
#include <thread>
#include <chrono>
#include <deque>
#include <map>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <regex>
#include <atomic>
#include <unordered_set>

// Vendors
#include <sqlite3.h>

#ifdef USE_REDIS
#include <hiredis/hiredis.h>
#endif
