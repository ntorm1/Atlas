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
Vector<SharedPtr<AST::StrategyBufferOpNode>> const&
ModelBase::getFeatures() const noexcept
{
	return m_impl->m_features;
}


//============================================================================
ModelBase::ModelBase(
	String id,
	Vector<SharedPtr<AST::StrategyBufferOpNode>> features,
	SharedPtr<AST::StrategyBufferOpNode> target,
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


}


}