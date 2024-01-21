module;
#include <chrono>
#include <cassert>
module AtlasTimeModule;


namespace Atlas
{

namespace Time
{



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
int getMonthFromEpoch(int64_t ns_epoch) noexcept
{
	std::tm time_info = nsEpochToTm(ns_epoch);
	int month_number = time_info.tm_mon + 1; // tm_mon is zero-based
	return month_number;
}

}


}