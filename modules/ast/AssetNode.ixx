module;
#pragma once
#define NOMINMAX
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API  __declspec(dllimport)
#endif
#include <Eigen/Dense>
export module AssetNodeModule;

import AtlasCore;
import BaseNodeModule;
import StrategyBufferModule;
import AtlasLinAlg;

namespace Atlas
{

namespace AST
{

//============================================================================
export class AssetReadNode final
	: public StrategyBufferOpNode
{
private:
	size_t m_column;
	int m_row_offset;
	size_t m_warmup;
	size_t m_null_count = 0;
	Exchange const& m_exchange;

public:
	AssetReadNode(size_t column, int row_offset, Exchange& exchange) noexcept;

	ATLAS_API AssetReadNode(size_t column, int row_offset, SharedPtr<Exchange> exchange) noexcept :
		AssetReadNode(column, row_offset, *exchange) {}


	ATLAS_API static Result<SharedPtr<AssetReadNode>, AtlasException>
		make(
			String const& column,
			int row_offset,
			Exchange& m_exchange
		) noexcept;

	ATLAS_API static SharedPtr<AssetReadNode>
	pyMake(
			String const& column,
			int row_offset,
			Exchange& m_exchange
	);
	
	[[nodiscard]] int getRowOffset() const noexcept { return m_row_offset; }
	[[nodiscard]] size_t getNullCount() const noexcept { return m_null_count; }
	[[nodiscard]] size_t size() const noexcept;
	[[nodiscard]] size_t getWarmup() const noexcept override { return m_warmup; }
	ATLAS_API void evaluate(LinAlg::EigenVectorXd& target) noexcept override;
};


//============================================================================
export class AssetOpNode final
	: public StrategyBufferOpNode
{
private:
	SharedPtr<StrategyBufferOpNode> m_asset_op_left;
	SharedPtr<StrategyBufferOpNode> m_asset_op_right;
	LinAlg::EigenVectorXd m_right_buffer;
	AssetOpType m_op_type;
	size_t warmup;

public:
	virtual ~AssetOpNode() noexcept = default;


	//============================================================================
	AssetOpNode(
		SharedPtr<StrategyBufferOpNode> asset_op_left,
		SharedPtr<StrategyBufferOpNode> asset_op_right,
		AssetOpType op_type
	) noexcept :
		StrategyBufferOpNode(NodeType::ASSET_OP, asset_op_left->getExchange(), std::nullopt),
		m_asset_op_left(std::move(asset_op_left)),
		m_asset_op_right(std::move(asset_op_right)),
		m_op_type(op_type)
	{
		warmup = std::max(m_asset_op_left->getWarmup(), m_asset_op_right->getWarmup());
	}


	//============================================================================
	ATLAS_API static Result<SharedPtr<AssetOpNode>, AtlasException>
	make(
			SharedPtr<StrategyBufferOpNode> asset_op_left,
			SharedPtr<StrategyBufferOpNode> asset_op_right,
			AssetOpType op_type
		) noexcept
	{
		return std::make_unique<AssetOpNode>(
			std::move(asset_op_left),
			std::move(asset_op_right),
			op_type
		);
	}

	//============================================================================
	ATLAS_API static SharedPtr<AssetOpNode>
	pyMake(
		SharedPtr<StrategyBufferOpNode> asset_op_left,
		SharedPtr<StrategyBufferOpNode> asset_op_right,
		AssetOpType op_type
	);


	//============================================================================
	[[nodiscard]] size_t getWarmup() const noexcept override
	{
		return warmup;
	}


	//============================================================================
	ATLAS_API void evaluate(LinAlg::EigenVectorXd& target) noexcept override;
};

};


}