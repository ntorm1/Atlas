module;
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
size_t
ModelBase::getAssetCount() const noexcept
{
	return m_exchange->getAssetCount();
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
	SharedPtr<AST::StrategyBufferOpNode> target,
	SharedPtr<ModelConfig> config
) noexcept:
	m_config(config),
	m_exchange(config->exchange)
{
	m_impl= new ModelBaseImpl(
		std::move(id),
		std::move(features),
		std::move(target)
	);
}


//============================================================================
ModelBase::~ModelBase() noexcept
{
	delete m_impl;
}


}


}