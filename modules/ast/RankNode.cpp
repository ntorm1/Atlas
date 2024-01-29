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
	UniquePtr<ExchangeViewNode> ev,
	EVRankType type,
	size_t count
) noexcept :
	StrategyBufferOpNode(NodeType::RANK_NODE, ev->getExchange(), ev.get()),
	m_N(count),
	m_type(type),
	m_ev(std::move(ev))
{
	size_t view_count = m_ev->getViewSize();
	m_view.reserve(view_count);
	for (size_t i = 0; i < view_count; ++i)
	{
		m_view.push_back(std::make_pair(i, 0.0));
	}
}


//============================================================================
EVRankNode::~EVRankNode() noexcept
{
}


//============================================================================
static bool
comparePairs(const std::pair<size_t, double>& a, const std::pair<size_t, double>& b)
{
    if (std::isnan(a.second))
    {
        return false;
    }
    if (std::isnan(b.second))
    {
        return true;
    }
    return a.second < b.second;
}


//============================================================================
void
EVRankNode::sort() noexcept
{
    switch (m_type) {
    case EVRankType::NSmallest:
        // do a partial sort to find the smallest N elements
        std::partial_sort(
            m_view.begin(),
            m_view.begin() + m_N,
            m_view.end(),
            &comparePairs
        );
        break;
    case EVRankType::NLargest:
        // do a partial sort to find the largest N elements
        std::partial_sort(
            m_view.begin(),
            m_view.begin() + m_N,
            m_view.end(),
            &comparePairs
        );
        break;
    case EVRankType::NEXTREME:
        // do a partial sort, first N elements are the smallest
        std::partial_sort(
            m_view.begin(),
            m_view.begin() + m_N,
            m_view.end(),
            &comparePairs
        );

        // Custom comparator for the last N elements (largest)
        std::partial_sort(
            m_view.begin() + m_N,
            m_view.end(),
            m_view.end(),
            &comparePairs
        );
        break;
    }
}

//============================================================================
void
EVRankNode::evaluate(Eigen::VectorXd& target) noexcept
{
	// before executing cross sectional rank, execute the parent exchange 
	// view operation to populate target vector with feature values
	m_ev->evaluate(target);

	// then we copy target over to the view pair vector to keep 
	// track of the index locations of each value when sorting
	assert(static_cast<size_t>(target.size()) == m_view.size());
	for (size_t i = 0; i < m_view.size(); ++i)
	{
		m_view[i].second = target[i];
	}

	// sort the view pair vector, the target elements will be in
	// the first N locations, we can then set the rest to Nan to
	// prevent them from being allocated. At which point we have a 
	// target vector that looks like: 
	// [Nan, largest_element, Nan, second_largest_element, Nan, Nan]
	sort();
	for (size_t i = m_N; i < m_view.size(); ++i)
	{
		target[m_view[i].first] = std::numeric_limits<double>::quiet_NaN();
	}
}

}

}