#pragma once

export module AtlasEnumsModule;

export namespace Atlas
{
	//============================================================================
	export enum class TracerType {
		NLV,
		VOLATILITY,
		WEIGHTS,
		ORDERS_EAGER,
		ORDERS_LAZY
	};

	//============================================================================
	export enum TradeLimitType : unsigned int
	{
		NONE = 0,
		STOP_LOSS = 1 << 0,
		TAKE_PROFIT = 1 << 1,
		BOTH = STOP_LOSS | TAKE_PROFIT
	};

	//============================================================================
	export enum class CovarianceType
	{
		FULL,
		INCREMENTAL
	};
}