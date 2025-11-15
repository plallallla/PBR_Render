#pragma once
#include <ctime>
#include <sstream>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/base_sink.h>
#include <string_view>
#include "utility.hpp"

struct sPdlog
{
	std::shared_ptr<spdlog::logger> logger;
	sPdlog(std::string name)
	{
		time_t tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		std::tm time_tm{};
#ifdef _WIN32
    localtime_s(&time_tm, &tt);
#else
    localtime_r(&tt, &time_tm);
#endif
    	std::stringstream ss;
    	ss << "log/" << name << "_"
        	<< (time_tm.tm_year + 1900) << "_"
        	<< (time_tm.tm_mon + 1) << "_"
        	<< time_tm.tm_mday << ".log";
		logger = spdlog::rotating_logger_mt(name, ss.str(), 1024 * 10, 100);
		logger->flush_on(spdlog::level::trace);
	}
};

#define LOG LogHelper::getInstance()
class LogHelper
{
    SINGLETON(LogHelper);
	sPdlog sPdlogger{ "render" };
public:
	void Init()
	{
		sPdlogger.logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] %v ");
	}
	void info(std::string_view content)
	{
		sPdlogger.logger->info(content.data());
	}
};