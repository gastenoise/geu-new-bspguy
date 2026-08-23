#pragma once
#include <mutex>

namespace Sync
{
	extern std::mutex LogConsole;
	extern std::mutex BspOps;
	extern std::mutex Clipnodes;
	extern std::mutex TextureUpload;
	extern std::mutex TexturesList;
	extern std::mutex LogQueue;
	extern std::mutex LogFlush;
	extern std::mutex Settings;
}
