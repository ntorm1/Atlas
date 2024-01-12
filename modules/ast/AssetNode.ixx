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
import AtlasLinAlg;

namespace Atlas
{

namespace AST
{

//============================================================================
export class AssetReadNode final
	: public ExpressionNode<LinAlg::EigenConstColView<double>>
{
private:
	size_t m_column;
	int m_row_offset;
	size_t m_warmup;
	Exchange const& m_exchange;

public:
	AssetReadNode(size_t column, int row_offset, const Exchange& exchange) noexcept
		: ExpressionNode<LinAlg::EigenConstColView<double>>(NodeType::ASSET_READ),
		m_column(column),
		m_row_offset(row_offset),
		m_exchange(exchange),
		m_warmup(static_cast<size_t>(std::abs(m_row_offset))) {}

	ATLAS_API static Result<UniquePtr<AssetReadNode>, AtlasException>
	make(
		String const& column,
		int row_offset,
		Exchange const& m_exchange
	) noexcept;

	[[nodiscard]] size_t size() const noexcept;
	[[nodiscard]] size_t getWarmup() const noexcept override { return m_warmup; }
	ATLAS_API [[nodiscard]] LinAlg::EigenConstColView<double> evaluate() noexcept override;
};


//============================================================================
export template <typename NodeCwiseBinOp>
class AssetOpNode
	:public ExpressionNode<LinAlg::EigenCwiseBinOp<NodeCwiseBinOp>>
{
private:
	UniquePtr<AssetReadNode> m_asset_read_left;
	UniquePtr<AssetReadNode> m_asset_read_right;
	AssetOpType m_op_type;
	size_t warmup;

public:
	virtual ~AssetOpNode() noexcept = default;
	AssetOpNode(
		UniquePtr<AssetReadNode> asset_read_left,
		UniquePtr<AssetReadNode> asset_read_right,
		AssetOpType op_type
	) noexcept: 
		ExpressionNode<LinAlg::EigenCwiseBinOp<NodeCwiseBinOp>>(NodeType::ASSET_OP),
		m_asset_read_left(std::move(asset_read_left)),
		m_asset_read_right(std::move(asset_read_right)),
		m_op_type(op_type)
	{
		warmup = std::max(m_asset_read_left->getWarmup(), m_asset_read_right->getWarmup());
	}

	[[nodiscard]] size_t getWarmup() const noexcept override
	{
		return warmup;
	}

	//============================================================================
	[[nodiscard]] LinAlg::EigenCwiseBinOp<NodeCwiseBinOp>
	evaluate() noexcept override
	{
		auto left_view = m_asset_read_left->evaluate();
		auto right_view = m_asset_read_right->evaluate();

		// if template type is EigenCwiseProductOp, return left_view.cwiseProduct(right_view);
		if constexpr (std::is_same_v<NodeCwiseBinOp,LinAlg::EigenCwiseProductOp>)
		{
			return left_view.cwiseProduct(right_view);
		}
		else if constexpr (std::is_same_v<NodeCwiseBinOp, LinAlg::EigenCwiseQuotientOp>)
		{
			return left_view.cwiseQuotient(right_view);
		}
		else if constexpr (std::is_same_v<NodeCwiseBinOp, LinAlg::EigenCwiseSumOp>)
		{
			return left_view + right_view;
		}
		else if constexpr (std::is_same_v<NodeCwiseBinOp, LinAlg::EigenCwiseDifferenceOp>)
		{
			return left_view - right_view;
		}
	}
};


//============================================================================
#define ASSET_OPERATION_NODE(NAME, OP_TYPE)                            \
export class Asset##NAME##Node : public AssetOpNode<LinAlg::OP_TYPE>          \
{                                                                      \
public:                                                                \
    Asset##NAME##Node(                                                \
        UniquePtr<AssetReadNode> asset_read_left,                     \
        UniquePtr<AssetReadNode> asset_read_right,                    \
        AssetOpType op_type) noexcept :                               \
        AssetOpNode<LinAlg::OP_TYPE>(                                 \
            std::move(asset_read_left),                               \
            std::move(asset_read_right),                              \
            op_type)                                                   \
    {}                                                                 \
};

ASSET_OPERATION_NODE(Product, EigenCwiseProductOp)
ASSET_OPERATION_NODE(Quotient, EigenCwiseQuotientOp)
ASSET_OPERATION_NODE(Sum, EigenCwiseSumOp)
ASSET_OPERATION_NODE(Difference, EigenCwiseDifferenceOp)



};


}