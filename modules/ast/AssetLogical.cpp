module;
#include <Eigen/Dense>
#include "AtlasMacros.hpp"
module AssetLogicalModule;


namespace Atlas
{

namespace AST
{

//============================================================================
AssetIfNode::AssetIfNode(
	AssetCompType comp_type,
	SharedPtr<StrategyBufferOpNode> left_eval,
	SharedPtr<StrategyBufferOpNode> right_eval
) noexcept :
	StrategyBufferOpNode(NodeType::ASSET_IF, left_eval->getExchange(), left_eval.get()),
	m_left_eval(left_eval),
	m_right_eval(right_eval),
	m_comp_type(comp_type)
{
	m_warmup = std::max({left_eval->getWarmup(), right_eval->getWarmup()});
	m_buffer.resize(m_left_eval->getAssetCount());
	m_buffer.setZero();
}

//============================================================================
AssetIfNode::~AssetIfNode() noexcept
{

}


//============================================================================
void
AssetIfNode::evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept
{
    m_left_eval->evaluate(target);
    m_right_eval->evaluate(m_buffer);

    switch (m_comp_type) {
        case AssetCompType::EQUAL:
			target = (target.array() == m_buffer.array()).select(target, NAN_DOUBLE);
			break;
		case AssetCompType::NOT_EQUAL:
			target = (target.array() != m_buffer.array()).select(target, NAN_DOUBLE);
			break;
		case AssetCompType::GREATER:
			target = (target.array() > m_buffer.array()).select(target, NAN_DOUBLE);
			break;
		case AssetCompType::GREATER_EQUAL:
			target = (target.array() >= m_buffer.array()).select(target, NAN_DOUBLE);
			break;
		case AssetCompType::LESS:
			target = (target.array() < m_buffer.array()).select(target, NAN_DOUBLE);
			break;
		case AssetCompType::LESS_EQUAL:
			target = (target.array() <= m_buffer.array()).select(target, NAN_DOUBLE);
			break;
	}
}


//============================================================================
AssetCompNode::AssetCompNode(
	SharedPtr<StrategyBufferOpNode> left_eval,
	LogicalType logicial_type,
	SharedPtr<StrategyBufferOpNode> right_eval,
	SharedPtr<StrategyBufferOpNode> true_eval,
	SharedPtr<StrategyBufferOpNode> false_eval
) noexcept:
	StrategyBufferOpNode(NodeType::ASSET_COMP, left_eval->getExchange(), left_eval.get()),
	m_left_eval(left_eval),
	m_right_eval(right_eval),
	m_true_eval(true_eval),
	m_false_eval(false_eval),
	m_logical_type(logicial_type)
{
	m_warmup = std::max({left_eval->getWarmup(), right_eval->getWarmup(), true_eval->getWarmup(), false_eval->getWarmup()});
	m_buffer.resize(m_left_eval->getAssetCount(), 3);
	m_buffer.setZero();
}


//============================================================================
AssetCompNode::~AssetCompNode() noexcept
{
}


//============================================================================
void
AssetCompNode::evaluate(
	LinAlg::EigenRef<LinAlg::EigenVectorXd> target
) noexcept
{
	m_left_eval->evaluate(target);
	m_right_eval->evaluate(m_buffer.col(RIGHT_EVAL_IDX));
	m_true_eval->evaluate(m_buffer.col(TRUE_EVAL_IDX));
	m_false_eval->evaluate(m_buffer.col(FALSE_EVAL_IDX));
	switch (m_logical_type) {
		case LogicalType::AND:
			target = (!target.array().isNaN() && !m_buffer.col(RIGHT_EVAL_IDX).array().isNaN())
				.select(m_buffer.col(TRUE_EVAL_IDX), m_buffer.col(FALSE_EVAL_IDX));
		case LogicalType::OR:
			target = (!target.array().isNaN() || !m_buffer.col(RIGHT_EVAL_IDX).array().isNaN())
				.select(m_buffer.col(TRUE_EVAL_IDX), m_buffer.col(FALSE_EVAL_IDX));
			break;
	}
}


}


}