module;
#include "AtlasMacros.hpp"
module ExchangeModule;

import ExchangePrivateModule;

namespace Atlas
{

//==============================================================================
struct PyColDef
{
	String asset_id;
	String column_name;
	size_t column_index;

	PyColDef(String const& asset_id, String const& column_name, size_t column_index)
		: asset_id(asset_id)
		, column_name(column_name)
		, column_index(column_index)
	{
	}
};


//==============================================================================
Exchange::Exchange(
	Vector<String> const& columns,
	Vector<Int64> const& py_timestamps,
	Vector<Vector<double>> const& data,
	size_t id)
{
	// check that each column has exactly one space in the name
	Set<String> py_asset_names;
	Vector<PyColDef> py_columns;
	for (auto const& column : columns)
	{
		if (std::count(column.begin(), column.end(), ' ') != 1)
		{
			throw std::runtime_error("Column names must have exactly one space");
		}
		
		// get the asset name as the first word
		py_asset_names.push_back(column.substr(0, column.find(' ')));

		// the column name is the second word
		String column_name = column.substr(column.find(' ') + 1);

		// add the column to py_columns
		PyColDef py_col_def(py_asset_names.back(), column_name, py_columns.size());
		py_columns.push_back(py_col_def);
	}

	// get the column count, ensure that the division has no remainder
	if (py_columns[0].size() % py_asset_names.size() != 0) 
	{
		throw runtime_error("Column count not a multiple of asset count");
	}
	size_t column_count = py_columns.size() / py_asset_names.size();

	// sort py_columns by name
	std::sort(
		py_columns.begin(),
		py_columns.end(),
		[](auto const& a, auto const& b) { return a.asset_id < b.asset_id; }
	);

	// check that each asset has the same columns 
	size_t i = 0;
	for (auto const& asset_id : py_asset_names)
	{
		// find the first column in py_columns that matches the asset name
		auto it = std::find_if(
			py_columns.begin(),
			py_columns.end(),
			[&asset_id](auto const& py_col) { return py_col.asset_id == asset_id; }
		);
		assert(it)

		// get distance from the first column to the first column of the next asset
		size_t distance = std::distance(it, py_columns.end());
		for (size_t j = distance; j < column_count; ++j)
		{
			auto const& column = py_columns[distance + j];
			if (column.column_name != py_columns[j].column_name)
			{
				throw runtime_error("Asset " + asset_id + " does not have the same columns as the other assets");
			}

			if (i == 0)
			{
				m_impl->headers[column.column_name] = j;
			}
		}
		++i;
	}
	
	m_impl->col_count = column_count;
	
	Option<size_t> close_index = getCloseIndex();
	if (!close_index.has_value())
	{
		throw runtime_error("No close column found");
	}
	m_impl->close_index = close_index.value();

	// build data matrix
	m_impl->data.resize(
		py_asset_names.size(),
		py_timestamps.size() * m_impl->headers.size()
	);
	// store the percentage change in price for each asset at each timestamp
	m_impl->returns.resize(
		m_impl->assets.size(),
		py_timestamps.size()
	);

	// copy data from python to c++

}