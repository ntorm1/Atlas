module;
#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API  __declspec(dllimport)
#endif
export module AtlasTimeModule;


import AtlasCore;

namespace Atlas
{

namespace Time
{



//============================================================================
export enum class TimeUnit
{
	DAYS = 0,
	WEEKS = 1,
	MONTHS = 2,
};


//============================================================================
export struct TimeOffset
{
	TimeUnit type;
	size_t count;
};


export Int64 applyTimeOffset(Int64 t, TimeOffset o);
export int getMonthFromEpoch(Int64 epoch) noexcept;
export Result<Int64, AtlasException> strToEpoch(const String& str, const String& dt_format) noexcept;
export ATLAS_API String convertNanosecondsToTime(Int64 nanoseconds);

}

}