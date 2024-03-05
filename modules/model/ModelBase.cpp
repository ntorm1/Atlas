module;
#include <Eigen/Dense>
module ModelBaseModule;

import ExchangeModule;

namespace Atlas
{


namespace Model
{

struct ModelBaseImpl
{
	String m_id;
	Vector<SharedPtr<AST::StrategyBufferOpNode>> m_features;
	SharedPtr<AST::StrategyBufferOpNode> m_target;

	ModelBaseImpl(
		String model_id,
		Vector<SharedPtr<AST::StrategyBufferOpNode>> features,
		SharedPtr<AST::StrategyBufferOpNode> target
	) noexcept
	{
		m_id = model_id;
		m_features = std::move(features);
		m_target = std::move(target);
	}

};


//============================================================================
ModelConfig::ModelConfig(
	size_t training_window,
	size_t walk_forward_window,
	ModelType type,
	SharedPtr<Exchange> exchange
) noexcept:
	training_window(training_window),
	walk_forward_window(walk_forward_window),
	type(type),
	exchange(exchange)
{
}


//============================================================================
void
ModelBase::stepBase() noexcept
{
	size_t const current_idx = m_exchange->currentIdx();
	if (current_idx < m_feature_warmup)
	{
		return;
	}

	step();
	if (current_idx > 0 && current_idx % m_config->training_window == 0)
	{
		train();
	}
}


//============================================================================
Vector<SharedPtr<AST::StrategyBufferOpNode>> const&
ModelBase::getFeatures() const noexcept
{
	return m_impl->m_features;
}


//============================================================================
ModelBase::ModelBase(
	String id,
	Vector<SharedPtr<AST::StrategyBufferOpNode>> features,
	SharedPtr<ModelTarget> target,
	SharedPtr<ModelConfig> config
) noexcept:
	AST::StrategyBufferOpNode(AST::NodeType::MODEL, *config->exchange, std::nullopt),
	m_config(config),
	m_exchange(config->exchange)
{
	m_asset_count = m_exchange->getAssetCount();
	m_signal.resize(m_asset_count);
	m_signal.setZero();
	m_impl= new ModelBaseImpl(
		std::move(id),
		std::move(features),
		std::move(target)
	);

	for (auto const& feature : m_impl->m_features)
	{
		m_feature_warmup = std::max(m_feature_warmup, feature->getWarmup());
	}
	m_warmup = m_feature_warmup + m_config->training_window;
}


//============================================================================
ModelBase::~ModelBase() noexcept
{
	delete m_impl;
}


//============================================================================
String const&
ModelBase::getId() const noexcept
{
	return m_impl->m_id;
}

//============================================================================
size_t
ModelBase::getWarmup() const noexcept
{
	return m_warmup;
}


//============================================================================
void
ModelBase::evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept
{
	target = m_signal;
}



//============================================================================
ModelTarget::ModelTarget(
	SharedPtr<AST::StrategyBufferOpNode> target,
	ModelTargetType type,
	size_t lookforward
) noexcept:
	AST::StrategyBufferOpNode(AST::NodeType::TARGET, target->getExchange(), std::nullopt),
	m_target(target),
	m_type(type),
	m_lookforward(lookforward)
{
	m_target_buffer.resize(m_target->getAssetCount(), m_lookforward);
}


//============================================================================
ModelTarget::~ModelTarget() noexcept
{
}


//============================================================================
void
ModelTarget::cacheTarget() noexcept
{
	auto const col_slice = m_target_buffer.col(m_buffer_idx);
	m_target->evaluate(col_slice);
	m_buffer_idx = (m_buffer_idx + 1) % m_lookforward;
}


//============================================================================
bool
ModelTarget::isSame(SharedPtr<StrategyBufferOpNode> other) const noexcept
{
	if (other->getType() != AST::NodeType::TARGET)
	{
		return false;
	}
	auto const other_target = std::static_pointer_cast<ModelTarget>(other);
	return m_target == other_target->m_target&&
		m_type == other_target->m_type &&
		m_lookforward == other_target->m_lookforward;
}


//============================================================================
void
ModelTarget::evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept
{
	switch (m_type)
	{
	case ModelTargetType::ABSOLUTE:
		break;
	case ModelTargetType::DELTA:
		break;
		case ModelTargetType::PERCENTAGE_CHANGE:
		break;
	}
}


//============================================================================
void
ModelTarget::reset() noexcept
{
	m_buffer_idx = 0;
	m_target_buffer.setZero();
	m_target->reset();
}


}


}