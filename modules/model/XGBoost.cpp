module;

#include <xgboost/c_api.h>

module XGBoostModule;

namespace Atlas
{

namespace Model
{

//============================================================================
XGBoostModelConfig::XGBoostModelConfig(
	SharedPtr<ModelConfig> base_config
) noexcept:
	m_base_config(std::move(base_config))
{
}


//============================================================================
struct XGBoostModelImpl
{
	DMatrixHandle dmatrix;
};


//============================================================================
XGBoostModel::XGBoostModel(
	String id,
	Vector<SharedPtr<AST::StrategyBufferOpNode>> features,
	SharedPtr<ModelTarget> target,
	SharedPtr<const XGBoostModelConfig> config
) noexcept:
	ModelBase(std::move(id), std::move(features), std::move(target), config->m_base_config),
	m_xgb_config(config)
{
	m_impl = new XGBoostModelImpl();
}


//============================================================================
XGBoostModel::~XGBoostModel() noexcept
{
	delete m_impl;
}
}


}