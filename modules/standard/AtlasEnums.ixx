#pragma once

export module AtlasEnumsModule;

export namespace Atlas
{
	export enum class TracerType {
		NLV,
		WEIGHTS
	};

	//============================================================================
	export enum class TradeLimitType
	{
		STOP_LOSS = 0,
		TAKE_PROFIT = 1,
	};

}