#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API  __declspec(dllimport)
#endif

#include "standard/AtlasCore.hpp"

namespace Atlas
{

namespace Time
{



//============================================================================
 enum class TimeUnit
{
	DAYS = 0,
	WEEKS = 1,
	MONTHS = 2,
};


//============================================================================
 struct TimeOffset
{
	TimeUnit type;
	size_t count;
};


 Int64 applyTimeOffset(Int64 t, TimeOffset o);
 int getMonthFromEpoch(Int64 epoch) noexcept;
 Result<Int64, AtlasException> strToEpoch(const String& str, const String& dt_format) noexcept;
 ATLAS_API String convertNanosecondsToTime(Int64 nanoseconds);

}

}