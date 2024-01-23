module;
#include <Eigen/Dense>
module RankNodeModule;

import ExchangeNodeModule;

namespace Atlas
{

namespace AST
{

//============================================================================
EVRankNode::EVRankNode(
	SharedPtr<ExchangeViewNode> ev,
	EVRankType type,
	size_t count
) noexcept :
	StrategyBufferOpNode(NodeType::RANK_NODE, ev->getExchange(), ev.get()),
	m_count(count),
	m_type(type),
	m_ev(ev)
{
	assert(ev.use_count() <= 2);
	size_t view_count = m_ev->getViewSize();
	for (size_t i = 0; i < view_count; ++i)
	{
		m_views.push_back(std::make_pair(i, 0.0));
	}
}


//============================================================================
EVRankNode::~EVRankNode() noexcept
{
}


//============================================================================
void
EVRankNode::evaluate(Eigen::VectorXd& target) noexcept
{
	m_ev->evaluate(target);
}

}

}