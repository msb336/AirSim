#pragma once
#include "common/Common.hpp"
#include "common/common_utils/FileSystem.hpp"
#include "common/StateReporterWrapper.hpp"
/* 
RaceLogger Tracks:
-Current leader
-Odometry
-Collisions with objects
-Near drone collisions
-Actual drone collisions
-Missed gates

*/
class RaceLogger
{
public:
	RaceLogger();
	~RaceLogger();
	void log(const std::string& update);
	void close();
private:
	void score();
private:
	static std::ofstream log_;
};
