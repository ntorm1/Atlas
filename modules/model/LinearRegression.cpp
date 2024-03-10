module;
#include <Eigen/Dense>
module LinearRegressionModule;


namespace Atlas
{

namespace Model
{


//============================================================================
LinearRegressionModelConfig::LinearRegressionModelConfig(
	SharedPtr<ModelConfig> base_config,
	LinearRegressionSolver solver,
	bool fit_intercept 
) noexcept:
	m_solver(solver),
	m_base_config(std::move(base_config)),
	m_fit_intercept(fit_intercept)
{
}


//============================================================================
LinearRegressionModel::LinearRegressionModel(
	String id,
	Vector<SharedPtr<AST::StrategyBufferOpNode>> features,
	SharedPtr<ModelTarget> target,
	SharedPtr<const LinearRegressionModelConfig> config
) noexcept:
	ModelBase(std::move(id), std::move(features), std::move(target), config->m_base_config),
	m_lr_config(config)
{
	size_t row_count = m_config->training_window * m_asset_count;
	size_t feature_count = getFeatures().size();
	if (m_lr_config->m_fit_intercept)
	{
		m_X.resize(row_count, feature_count + 1);
		m_X.setZero();
		m_X.col(feature_count).setOnes();
	}
	m_y.setZero();
	m_theta.resize(feature_count + m_lr_config->m_fit_intercept);
	m_theta.setZero();
}


//============================================================================
LinearRegressionModel::~LinearRegressionModel() noexcept
{
}


//============================================================================
void 
LinearRegressionModel::train() noexcept
{
	size_t look_forward = getTarget()->getLookForward();
	size_t training_window = m_config->training_window;
	LinAlg::EigenMatrixXd m_X_train(training_window - look_forward, m_theta.size());
	LinAlg::EigenVectorXd m_y_train(training_window - look_forward);
	copyBlocks(m_X_train, m_y_train);

	switch (m_lr_config->m_solver)
	{
		case LinearRegressionSolver::LDLT:
			m_theta = (m_X_train.transpose() * m_X_train).ldlt().solve(m_X_train.transpose() * m_y_train);
			break;
		case LinearRegressionSolver::ColPivHouseholderQR:
			m_theta = m_X_train.colPivHouseholderQr().solve(m_y_train);
			break;
	}
}

//============================================================================
void
LinearRegressionModel::reset() noexcept
{
	m_X.setZero();
	m_y.setZero();
	m_theta.setZero();
}


//============================================================================
void
LinearRegressionModel::predict() noexcept
{
	size_t buffer_idx = getBufferIdx();
	if (buffer_idx == static_cast<size_t>(m_X.rows()))
	{
		buffer_idx -= m_asset_count;
	}

	auto x_block = m_X.block(
		buffer_idx,
		0,
		m_asset_count,
		m_theta.size()
	);
	m_signal = x_block * m_theta;
}


//============================================================================
bool
LinearRegressionModel::isSame(SharedPtr<StrategyBufferOpNode> other) const noexcept
{
	return false;
}

}

}