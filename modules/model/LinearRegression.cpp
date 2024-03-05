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
	else
	{
		m_X.resize(row_count, feature_count);
		m_X.setZero();
	};
	m_y.resize(row_count);
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
			m_theta = m_X.colPivHouseholderQr().solve(m_y);
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
	auto const& features = getFeatures();
	auto const& target = getTarget();
	auto x_block = m_X.block(
		m_buffer_idx,
		0,
		m_asset_count,
		features.size()
	);
	for (size_t i = 0; i < features.size(); ++i)
	{
		features[i]->evaluate(x_block.col(i));
	}
	m_buffer_idx += m_asset_count;
	size_t look_forward = target->getLookForward();
	if (getCurrentIdx() > look_forward)
	{
		assert(m_buffer_idx >= m_asset_count * (look_forward + 1));
		size_t y_block_row_start = m_buffer_idx - m_asset_count * (look_forward + 1);
		auto y_block = m_y.block(
			y_block_row_start,
			0,
			m_asset_count,
			1
		);
		target->evaluate(y_block);
	}
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

	auto x_block = m_X.block(
		m_buffer_idx,
		0,
		m_asset_count,
		m_theta.size()
	);
	m_signal = x_block * m_theta;
	if (loop_idx)
		m_buffer_idx = 0;
}


//============================================================================
bool
LinearRegressionModel::isSame(SharedPtr<StrategyBufferOpNode> other) const noexcept
{
	return false;
}

}

}