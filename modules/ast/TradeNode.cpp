module;
#include <Eigen/Dense>
module TradeNodeModule;

import AllocationNodeModule;

namespace Atlas
{

namespace AST
{


//============================================================================
struct TradeLimitNodeImpl
{
	Eigen::VectorXd m_pnl;
	TradeLimitType m_trade_type;
	double m_limit;

	TradeLimitNodeImpl(TradeLimitType trade_type, double limit) noexcept:
		m_trade_type(trade_type),
		m_limit(limit)
	{
	}
};

//============================================================================
TradeLimitNode::~TradeLimitNode() noexcept
{
}


//============================================================================
TradeLimitNode::TradeLimitNode(
	AllocationBaseNode* parent,
	TradeLimitType trade_type,
	double limit
) noexcept:
	OpperationNode<void, LinAlg::EigenVectorXd const&>(NodeType::TRADE_LIMIT, parent)
{
	m_impl = std::make_unique<TradeLimitNodeImpl>(trade_type, limit);
	size_t asset_count = parent->getAssetCount();
	m_impl->m_pnl.resize(asset_count);
	m_impl->m_pnl.setZero();
}


//============================================================================
void TradeLimitNode::evaluate(LinAlg::EigenVectorXd const& x) noexcept
{
}


}

}