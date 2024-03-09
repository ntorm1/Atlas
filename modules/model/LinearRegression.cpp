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

	// if the buffer has looped around then we have to copy the features and target into the training
	// blocks in two steps. First we copy the features and target from 0 to the buffer index for the newest features,
	// then copy from the back starting at the end of the buffer offset by the number of rows remaining in the training window.
	if (m_buffer_looped && m_buffer_idx > m_asset_count * (look_forward + 1))
	{
		m_X_train.block(
			0,
			0,
			m_buffer_idx,
			m_theta.size()
		) = m_X.block(
			0,
			0,
			m_buffer_idx,
			m_theta.size()
		);
		m_y_train.block(
			0,
			0,
			m_buffer_idx,
			1
		) = m_y.block(
			0,
			0,
			m_buffer_idx,
			1
		);
		size_t remaining_rows = look_forward - m_buffer_idx / m_asset_count;
		size_t remaining_start = m_X.rows() - m_asset_count * remaining_rows;
		m_X_train.block(
			m_buffer_idx,
			0,
			remaining_rows,
			m_theta.size()
		) = m_X.block(
			remaining_start,
			0,
			remaining_rows * m_asset_count,
			m_theta.size()
		);
		m_y_train.block(
			m_buffer_idx,
			0,
			remaining_rows,
			1
		) = m_y.block(
			remaining_start,
			0,
			remaining_rows * m_asset_count,
			1
		);
	}
	else
	{
		// if buffer has wrapped around but it hasn't passed the look forward window then we can just copy the features
		// from the back of the buffer offest by the difference between the look forward window and the buffer index
		size_t train_end_idx;
		if (m_buffer_looped)
		{
			train_end_idx = m_X.rows() - m_asset_count * (look_forward - m_buffer_idx / m_asset_count);
		}
		else
		{
			train_end_idx = m_buffer_idx - m_asset_count * look_forward;
		}
		size_t train_start_idx = train_end_idx - m_asset_count * (training_window - look_forward);
		size_t train_block_size = train_end_idx - train_start_idx;
		m_X_train = m_X.block(
			train_start_idx,
			0,
			train_block_size,
			m_theta.size()
		);
		m_y_train = m_y.block(
			train_start_idx,
			0,
			train_block_size,
			1
		);
	}

	// copy into std vectors
	Vector<Vector<double>> X_train_vec;
	Vector<double> y_train_vec;
	X_train_vec.reserve(m_X_train.rows());
	y_train_vec.reserve(m_y_train.rows());
	for (size_t i = 0; i < m_X_train.rows(); ++i)
	{
		Vector<double> row;
		row.reserve(m_X_train.cols());
		for (size_t j = 0; j < m_X_train.cols(); ++j)
		{
			row.push_back(m_X_train(i, j));
		}
		X_train_vec.push_back(row);
	}
	for (size_t i = 0; i < m_y_train.rows(); ++i)
	{
		y_train_vec.push_back(m_y_train(i));
	}

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
LinearRegressionModel::step() noexcept
{
	auto const& features = getFeatures();
	auto const& target = getTarget();

	// check for buffer index loop around and reset to 0
	if (m_buffer_idx == static_cast<size_t>(m_X.rows()))
	{
		m_buffer_idx = 0;
		m_buffer_looped = true;
	}

	// get the features x block starting at buffer index and number of rows
	// equal to the number of assets. Evaluate the features directly into the block view,
	// with constant being at the end of the block past features 
	assert(m_buffer_idx + m_asset_count <= static_cast<size_t>(m_X.rows()));
	assert(features.size() + m_lr_config->m_fit_intercept == static_cast<size_t>(m_X.cols()));
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

	// if past look forward copy the features target into the y block scaled back to where the 
	// corresponding features were evaluated
	size_t look_forward = target->getLookForward();
	if (getCurrentIdx() > look_forward)
	{
		// if we haven't reached the end of the buffer size check to make sure the buffer idx
		// is past the look forward window.
		if (!m_buffer_looped)
			assert(m_buffer_idx >= m_asset_count * (look_forward + 1));

		// look to find the start of the y block for the corresponding featues. If the buffer
		// recently looped around then we have to look at the end of the buffer and find the relative
		// depth to the back of the buffer. Otherwise we can just look back from the current buffer index
		size_t y_block_row_start;
		if (m_asset_count * (look_forward + 1) > m_buffer_idx)
		{
			size_t buffer_relative_index = m_buffer_idx / m_asset_count;
			y_block_row_start = m_X.rows() - m_asset_count * (look_forward + 1 - buffer_relative_index);
		}
		else
		{
			y_block_row_start = m_buffer_idx - m_asset_count * (look_forward + 1);
		}
		
		assert(y_block_row_start + m_asset_count <= static_cast<size_t>(m_y.rows()));
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