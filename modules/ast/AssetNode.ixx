module;
#pragma once
#define NOMINMAX
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API  __declspec(dllimport)
#endif
#include <cassert>
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
	[[nodiscard]] size_t getColumn() const noexcept { return m_column; }
	[[nodiscard]] size_t getWarmup() const noexcept override { return m_warmup; }
	ATLAS_API void evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept override;
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
	auto& getLeft() noexcept { return m_asset_op_left; }
	auto& getRight() noexcept { return m_asset_op_right; }

	virtual ~AssetOpNode() noexcept = default;

	//============================================================================
	AssetOpNode(
		SharedPtr<StrategyBufferOpNode> asset_op_left,
		SharedPtr<StrategyBufferOpNode> asset_op_right,
		AssetOpType op_type
	) noexcept;

	[[nodiscard]] size_t refreshWarmup() noexcept override;
	[[nodiscard]] size_t getWarmup() const noexcept override { return warmup; }

	//============================================================================
	ATLAS_API static Result<SharedPtr<AssetOpNode>, AtlasException>
	make(
			SharedPtr<StrategyBufferOpNode> asset_op_left,
			SharedPtr<StrategyBufferOpNode> asset_op_right,
			AssetOpType op_type
		) noexcept
	{
		assert(asset_op_left);
		assert(asset_op_right);
		return std::make_shared<AssetOpNode>(
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
	
	ATLAS_API static void swapLeft(SharedPtr<ASTNode> asset_op, SharedPtr<StrategyBufferOpNode>& left) noexcept;
	ATLAS_API static void swapRight(SharedPtr<ASTNode> asset_op, SharedPtr<StrategyBufferOpNode>& right) noexcept;
	ATLAS_API uintptr_t getSwapLeft() const noexcept { return reinterpret_cast<uintptr_t>(&AssetOpNode::swapLeft);}
	ATLAS_API uintptr_t getSwapRight() const noexcept { return reinterpret_cast<uintptr_t>(&AssetOpNode::swapRight);}
	void evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept override;
};


//============================================================================
export class AssetMedianNode
	: public StrategyBufferOpNode
{
private:
	size_t m_col_1;
	size_t m_col_2;

	AssetMedianNode(SharedPtr<Exchange> exchange, size_t col_1, size_t col_2) noexcept;

public:
	template<typename ...Arg> SharedPtr<AssetMedianNode>
	static make(Arg&&...arg) {
		struct EnableMakeShared : public AssetMedianNode {
			EnableMakeShared(Arg&&...arg) :AssetMedianNode(std::forward<Arg>(arg)...) {}
		};
		return std::make_shared<EnableMakeShared>(std::forward<Arg>(arg)...);
	}

	ATLAS_API static SharedPtr<AssetMedianNode> pyMake(SharedPtr<Exchange> exchange, String const& col_1, String const& col_2);
	ATLAS_API ~AssetMedianNode() noexcept;

	[[nodiscard]] size_t getWarmup() const noexcept override { return 0; }
	void evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept override;

};


//============================================================================
export class ATRNode
	: public StrategyBufferOpNode
{
private:
	size_t m_window = 0;
	size_t m_high = 0;
	size_t m_low = 0;
	size_t m_close = 0;
	LinAlg::EigenMatrixXd m_atr;

	ATRNode(
		Exchange& exchange,
		size_t high,
		size_t low,
		size_t window
	) noexcept;
	void build() noexcept;

public:
	template<typename ...Arg> SharedPtr<ATRNode>
	static make(Arg&&...arg) {
		struct EnableMakeShared : public ATRNode {
			EnableMakeShared(Arg&&...arg) :ATRNode(std::forward<Arg>(arg)...) {}
		};
		return std::make_shared<EnableMakeShared>(std::forward<Arg>(arg)...);
	}
	ATLAS_API static SharedPtr<ATRNode> pyMake(
		SharedPtr<Exchange> exchange,
		String const& high,
		String const& low,
		size_t window
	);

	size_t getHigh() const noexcept { return m_high; }
	size_t getLow() const noexcept { return m_low; }
	size_t getWindow() const noexcept { return m_window; }
	[[nodiscard]] bool isSame(SharedPtr<StrategyBufferOpNode> other) const noexcept override;
	[[nodiscard]] size_t getWarmup() const noexcept override { return m_window; }

	ATLAS_API ~ATRNode() noexcept;
	ATLAS_API auto const& getATR() const noexcept { return m_atr; }
	void evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept override;
};


}


}