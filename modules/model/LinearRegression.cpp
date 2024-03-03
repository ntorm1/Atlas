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
	SharedPtr<AST::StrategyBufferOpNode> target,
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
	else
	{
		m_X.resize(row_count, feature_count);
		m_X.setZero();
	};
	m_y.resize(row_count, 1);
	m_y.setZero();
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
	switch (m_lr_config->m_solver)
	{
		case LinearRegressionSolver::LDLT:
			m_theta = (m_X.transpose() * m_X).ldlt().solve(m_X.transpose() * m_y);
			break;
		case LinearRegressionSolver::ColPivHouseholderQR:
			m_theta = m_X.householderQr().solve(m_y);
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
LinearRegressionModel::step() noexcept
{
	auto x_block = m_X(
		Eigen::seq(m_buffer_idx, m_buffer_idx + m_asset_count),
		Eigen::seq(0, getFeatures().size())
	);
	auto const& features = getFeatures();
	for (size_t i = 0; i < m_asset_count; ++i)
	{
		features[i]->evaluate(x_block.col(i));
	}
	m_buffer_idx += m_asset_count;
}


//============================================================================
void
LinearRegressionModel::predict() noexcept
{
	bool loop_idx = false;
	if (m_buffer_idx == static_cast<size_t>(m_X.rows()))
	{
		m_buffer_idx -= m_asset_count;
		loop_idx = true;
	}

	auto x_block = m_X(
		Eigen::seq(m_buffer_idx, m_buffer_idx + m_asset_count),
		Eigen::seq(0, getFeatures().size())
	);

	m_signal = x_block * m_theta;

	if (loop_idx)
		m_buffer_idx = 0;
}


}

}