module;
#pragma once
#include <Eigen/Dense>
#include "unordered_dense.h"
export module ExchangePrivateModule;

import AtlasCore;

template<typename K, typename V>
using FastMap = ankerl::unordered_dense::map<K, V>;

namespace Atlas
{

//============================================================================
export struct Asset {
	Vector<Int64> timestamps;
	Vector<double> data;
	Option<String> source;
	String name;
	size_t id;
	size_t rows = 0;
	size_t cols = 0;
	HashMap<String, size_t> headers;

	Asset(String _name, size_t _id) noexcept
		: name(std::move(_name)), id(_id)
	{}

	void resize(size_t _rows, size_t _cols) noexcept
	{
		timestamps.resize(_rows);
		data.resize(_rows * _cols);
		rows = rows;
		cols = _cols;
	}

	void setSource(String _source) noexcept
	{
		source = std::move(_source);
	}

	bool isAscending() noexcept
	{
		for (size_t i = 1; i < timestamps.size(); ++i)
		{
			if (timestamps[i] < timestamps[i - 1])
			{
				return false;
			}
		}
		return true;
	}
	Result<bool, AtlasException> loadCSV();

};


//============================================================================
export struct ExchangeImpl
{
	friend class Exchange;

public:
	HashMap<String, size_t> asset_id_map;
	HashMap<String, size_t> headers;
	Vector<Asset> assets;
	Vector<Int64> timestamps;
	Vector<SharedPtr<AST::TriggerNode>> registered_triggers;
	FastMap<String,SharedPtr<AST::CovarianceNodeBase>> covariance_nodes;
	Vector<Strategy*> registered_strategies;
	Int64 current_timestamp = 0;
	Eigen::MatrixXd data;
	Eigen::MatrixXd returns;
	Eigen::VectorXd returns_scalar;
	size_t exchange_offset = 0;
	size_t col_count = 0;
	size_t close_index = 0;
	size_t current_index = 0;

	ExchangeImpl() noexcept
	{
		data = Eigen::MatrixXd::Zero(0, 0);
	}

private:
	void setExchangeOffset(size_t _offset) noexcept
	{
		exchange_offset = _offset;
	}

};

}