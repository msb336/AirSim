#include "logging/RaceLogger.hpp"
#include "common/ClockFactory.hpp"

std::ofstream RaceLogger::log_;

RaceLogger::RaceLogger()
{
	//log_path_ = common_utils::FileSystem::createLogFile("odometry", log_);
	common_utils::FileSystem::createLogFile("odometry", log_);
	//common_utils::FileSystem::createTextFile("odometry.temp", log_);
}
RaceLogger::~RaceLogger()
{
	close();
}
void RaceLogger::log(const std::string& update)
{
	log_ << update;
}
void RaceLogger::score()
{
}
void RaceLogger::close()
{
	score();
	log_.close();
}