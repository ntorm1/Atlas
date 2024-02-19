#pragma once


namespace Atlas
{

//============================================================================
struct Order
{
	size_t asset_id;
	size_t strategy_id;
	long long fill_time;
	double quantity;
	double fill_price;

	Order(
		size_t asset_id,
		size_t strategy_id,
		long long fill_time,
		double quantity,
		double fill_price
	) noexcept : asset_id(asset_id), strategy_id(strategy_id), fill_time(fill_time), quantity(quantity), fill_price(fill_price)
	{}
};

//============================================================================
struct Trade
{
	size_t asset_id;
	size_t strategy_id;
	long long open_time;
	long long close_time;
	double quantity;
	double open_price;
	double close_price;

	Trade(size_t asset_id, size_t strategy_id, long long open_time, long long close_time, double quantity, double open_price, double close_price)
		: asset_id(asset_id), strategy_id(strategy_id), open_time(open_time), close_time(close_time), quantity(quantity), open_price(open_price), close_price(close_price)
	{
	}
};

}