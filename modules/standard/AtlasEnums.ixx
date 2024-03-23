#pragma once

export module AtlasEnumsModule;

export namespace Atlas
{
	//============================================================================
	export enum class TracerType {
		NLV,
		VOLATILITY,
		WEIGHTS,
		ORDERS_EAGER
	};

	//============================================================================
	export enum TradeLimitType : unsigned int
	{
		NONE = 0,
		STOP_LOSS = 1 << 0,
		TAKE_PROFIT = 1 << 1
	};

	//============================================================================
	export enum class CovarianceType
	{
		FULL,
		INCREMENTAL
	};

	//============================================================================
	export enum class GridType
	{
		FULL = 0,
		UPPER_TRIANGULAR = 1,
		LOWER_TRIANGULAR = 2
	};

	//============================================================================
	export enum class LogicalType : unsigned int
	{
		AND = 0,
		OR	= 1
	};

}