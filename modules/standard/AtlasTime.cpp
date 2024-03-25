#include <chrono>
#include <cassert>
#include "AtlasMacros.hpp"
#include "AtlasTime.hpp"

namespace Atlas
{

namespace Time
{



//============================================================================
static std::tm nsEpochToTm(int64_t ns_epoch) noexcept
{
	std::chrono::seconds epoch_time(ns_epoch / 1000000000);

	// Convert nanoseconds to system_clock::time_point
	std::chrono::system_clock::time_point time_point = std::chrono::time_point<std::chrono::system_clock>(epoch_time);

	// Convert system_clock::time_point to std::time_t
	std::time_t time_t_value = std::chrono::system_clock::to_time_t(time_point);

	// Convert std::time_t to std::tm using gmtime_s for GMT time
	std::tm time_info;
	gmtime_s(&time_info, &time_t_value);

	return time_info;
}


//============================================================================
Int64
applyTimeOffset(Int64 timestamp, TimeOffset offset)
{
	std::tm time_info = nsEpochToTm(timestamp);

	switch (offset.type)
	{
		case TimeUnit::DAYS: {
			time_info.tm_mday += static_cast<int>(offset.count);
			return std::mktime(&time_info) * 1000000000;
			break;
		}
		case TimeUnit::WEEKS: {
			time_info.tm_mday += static_cast<int>(offset.count * 7);
			return std::mktime(&time_info) * 1000000000;
			break;
		}
		case TimeUnit::MONTHS: {
			time_info.tm_mon += static_cast<int>(offset.count);
			return std::mktime(&time_info) * 1000000000;
			break;
		}
		default: {
			assert(false);
			return 0;
		}
	}
}


//============================================================================
Result<Int64, AtlasException>
strToEpoch(
	const String& date_string,
	const String& dt_format
) noexcept
{
	try {
		std::tm timeStruct = {};
		std::istringstream iss(date_string);
		iss >> std::get_time(&timeStruct, dt_format.c_str());

		std::time_t utcTime = std::mktime(&timeStruct);

		// Convert to std::chrono::time_point
		std::chrono::system_clock::time_point timePoint = std::chrono::system_clock::from_time_t(utcTime);

		// Get the epoch time in nanoseconds
		return std::chrono::duration_cast<std::chrono::nanoseconds>(timePoint.time_since_epoch()).count();
	}
	catch (const std::exception& e) {
		return Err(e.what());
	}
}


//============================================================================
String
convertNanosecondsToTime(Int64 nanoseconds)
{
	// Convert nanoseconds to seconds
	auto seconds = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::nanoseconds(nanoseconds));

	// Create a time_point using the epoch time
	std::chrono::system_clock::time_point timePoint(std::chrono::seconds(seconds.count()));

	// Convert time_point to time_t
	std::time_t time = std::chrono::system_clock::to_time_t(timePoint);

	// Convert time_t to std::tm using localtime_s
	std::tm tmStruct;
	localtime_s(&tmStruct, &time);

	// Format the time as a string
	std::ostringstream oss;
	oss << std::put_time(&tmStruct, "%Y-%m-%d %H:%M:%S");

	return oss.str();
}


//============================================================================
int
getMonthFromEpoch(int64_t ns_epoch) noexcept
{
	std::tm time_info = nsEpochToTm(ns_epoch);
	int month_number = time_info.tm_mon + 1; // tm_mon is zero-based
	return month_number;
}

}


}