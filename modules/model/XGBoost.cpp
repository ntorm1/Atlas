module;
#include "AtlasFeature.hpp"
#include <cassert>

#ifdef ATLAS_XGBOOST
#include <xgboost/c_api.h>
#endif

module XGBoostModule;

namespace Atlas
{

namespace Model
{

#ifdef ATLAS_XGBOOST


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
	BoosterHandle booster;

	XGBoostModelImpl() noexcept;
	~XGBoostModelImpl() noexcept;
};


//============================================================================
XGBoostModelImpl::XGBoostModelImpl()
noexcept:
	dmatrix(nullptr),
	booster(nullptr)
{
}


//============================================================================
XGBoostModelImpl::~XGBoostModelImpl() noexcept
{
	if (dmatrix != nullptr)
		XGDMatrixFree(dmatrix);
	if (booster != nullptr)
		XGBoosterFree(booster);
}


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
	XGBoosterSetParam(m_impl->booster, "seed", "0");
}


//============================================================================
XGBoostModel::~XGBoostModel() noexcept
{
	delete m_impl;
}


//============================================================================
void
XGBoostModel::train() noexcept
{
	size_t look_forward = getTarget()->getLookForward();
	size_t training_window = m_config->training_window;
	LinAlg::EigenMatrixXf m_X_train(training_window - look_forward, getFeatures().size());
	LinAlg::EigenVectorXf m_y_train(training_window - look_forward);
	copyBlocks<float, LinAlg::EigenMatrixXf, LinAlg::EigenVectorXf>(m_X_train, m_y_train);

	auto res = XGDMatrixCreateFromMat(
		m_X_train.data(),
		m_X_train.rows(),
		m_X_train.cols(),
		0,
		&m_impl->dmatrix
	);
	assert(res == 0);

	res = XGDMatrixSetFloatInfo(
		m_impl->dmatrix,
		"label",
		m_y_train.data(),
		m_y_train.size()
	);
	assert(res == 0);

	assert(!(XGBoosterSetParam(m_impl->booster, "booster", "gblinear")));
	assert(!(XGBoosterSetParam(m_impl->booster, "max_depth", "3")));
	assert(!(XGBoosterSetParam(m_impl->booster, "eta", "0.1")));

	int num_of_iterations = 20;
	for (int i = 0; i < num_of_iterations; ++i) {
		assert(!(XGBoosterUpdateOneIter(m_impl->booster, i, m_impl->dmatrix)));
	}
}


//============================================================================
void
XGBoostModel::reset() noexcept
{
}


//============================================================================
void
XGBoostModel::predict() noexcept
{
}


//============================================================================
bool
XGBoostModel::isSame(SharedPtr<StrategyBufferOpNode> other) const noexcept
{
	return false;
}
}

#endif


}