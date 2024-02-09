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
static bool
comparePairsReverse(const std::pair<size_t, double>& a, const std::pair<size_t, double>& b)
{
    if (std::isnan(a.second))
    {
        return true;
    }
    if (std::isnan(b.second))
    {
        return false;
    }
    return a.second > b.second;
}


//============================================================================
void
EVRankNode::sort() noexcept
{
    switch (m_type) {
    case EVRankType::NSMALLEST:
        // do a partial sort to find the smallest N elements
        std::partial_sort(
            m_view.begin(),
            m_view.begin() + m_N,
            m_view.end(),
            &comparePairs
        );
        break;
    case EVRankType::NLARGEST:
        // do a partial sort to find the largest N elements
        std::partial_sort(
            m_view.begin(),
            m_view.begin() + m_N,
            m_view.end(),
            &comparePairsReverse
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
    case EVRankType::FULL:
		std::sort(
			m_view.begin(),
			m_view.end(),
			&comparePairs
		);
		break;
    }
}


//============================================================================
size_t
EVRankNode::getWarmup() const noexcept
{
    return m_ev->getWarmup();
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
	// [Nan, largest_element(1), Nan, second_largest_element (-1), Nan, Nan]
	sort();
    switch (m_type) {
        case EVRankType::NSMALLEST:
        case EVRankType::NLARGEST:
            for (size_t i = m_N; i < m_view.size(); ++i)
            {
				target[m_view[i].first] = std::numeric_limits<double>::quiet_NaN();
			}
			break;
        case EVRankType::NEXTREME:
            // the first N elements are the smallest, the last N are the largest. But 
            // we have to allow the allocation node to distinguish between the two so we eat the signal
            for (size_t i = 0; i < m_N; ++i)
            {
                target[m_view[i].first] = -1.0f;
            }
            for (size_t i = m_view.size() - m_N; i < m_view.size(); ++i)
            {
				target[m_view[i].first] = 1.0f;
			}
            for (size_t i = m_N; i < m_view.size() - m_N; ++i)
            {
                target[m_view[i].first] = std::numeric_limits<double>::quiet_NaN();
            }
            break;
        case EVRankType::FULL:
            for (size_t i = 0; i < m_view.size(); ++i)
            {
                target[m_view[i].first] = static_cast<double>(i);
            }
    }
}

}

}