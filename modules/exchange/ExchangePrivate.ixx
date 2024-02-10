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
	String name;
	size_t id;
	HashMap<String, size_t> headers;

	void resize(size_t rows, size_t cols) noexcept
	{
		timestamps.resize(rows);
		data.resize(rows * cols);
	}

	Asset(String n, size_t i) noexcept 
		: name(std::move(n)), id(i)
	{}
};


//============================================================================
export struct ExchangeImpl
{
	HashMap<String, size_t> asset_id_map;
	HashMap<String, size_t> headers;
	Vector<Asset> assets;
	Vector<Int64> timestamps;
	Vector<SharedPtr<AST::TriggerNode>> registered_triggers;
	FastMap<String,SharedPtr<AST::CovarianceNode>> covariance_nodes;
	Vector<Strategy*> registered_strategies;
	Int64 current_timestamp = 0;
	Eigen::MatrixXd data;
	Eigen::MatrixXd returns;
	Eigen::VectorXi null_count;
	size_t col_count = 0;
	size_t close_index = 0;
	size_t current_index = 0;

	ExchangeImpl() noexcept
	{
		data = Eigen::MatrixXd::Zero(0, 0);
	}
};

}