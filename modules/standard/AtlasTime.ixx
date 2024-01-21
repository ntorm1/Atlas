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
export int getMonthFromEpoch(int64_t epoch) noexcept;

}

}