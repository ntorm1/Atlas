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
	SharedPtr<ModelTarget> m_target;

	ModelBaseImpl(
		String model_id,
		Vector<SharedPtr<AST::StrategyBufferOpNode>> features,
		SharedPtr<ModelTarget> target
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
	if (current_idx >= m_config->training_window)
	{
		if (
			current_idx == m_config->training_window ||
			current_idx % m_config->walk_forward_window == 0
			)
		{
			train();
		}
		predict();
	}
}


//============================================================================
void
ModelBase::copyBlocks(
	LinAlg::EigenRef<LinAlg::EigenMatrixXd> x_train,
	LinAlg::EigenRef<LinAlg::EigenVectorXd> y_train
) const noexcept
{
	size_t look_forward = getTarget()->getLookForward();
	size_t training_window = m_config->training_window;
	size_t buffer_col_size = static_cast<size_t>(m_X.cols());
	// if the buffer has looped around then we have to copy the features and target into the training
	// blocks in two steps. First we copy the features and target from 0 to the buffer index for the newest features,
	// then copy from the back starting at the end of the buffer offset by the number of rows remaining in the training window.
	if (m_buffer_looped && m_buffer_idx >= m_asset_count * (look_forward + 1))
	{
		// get the end of front portion of buffer offset by the look forward window
		size_t front_end_idx = m_buffer_idx - m_asset_count * (look_forward);
		x_train.block(
			0,
			0,
			front_end_idx,
			buffer_col_size
		) = m_X.block(
			0,
			0,
			front_end_idx,
			buffer_col_size
		);
		y_train.block(
			0,
			0,
			front_end_idx,
			1
		) = m_y.block(
			0,
			0,
			front_end_idx,
			1
		);
		size_t remaining_rows = training_window - (front_end_idx / m_asset_count) - look_forward;
		size_t remaining_start = m_X.rows() - m_asset_count * remaining_rows;
		x_train.block(
			front_end_idx,
			0,
			remaining_rows,
			buffer_col_size
		) = m_X.block(
			remaining_start,
			0,
			remaining_rows,
			buffer_col_size
		);
		y_train.block(
			front_end_idx,
			0,
			remaining_rows,
			1
		) = m_y.block(
			remaining_start,
			0,
			remaining_rows,
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
		x_train = m_X.block(
			train_start_idx,
			0,
			train_block_size,
			buffer_col_size
		);
		y_train = m_y.block(
			train_start_idx,
			0,
			train_block_size,
			1
		);
	}

	Vector<Vector<double>> x_train_vec;
	for (size_t i = 0; i < x_train.rows(); ++i)
	{
		Vector<double> row;
		row.resize(x_train.cols());
		for (size_t j = 0; j < x_train.cols(); ++j)
		{
			row[j] = x_train(i, j);
		}
		x_train_vec.push_back(row);
	}
	auto x = 2;
}


//============================================================================
SharedPtr<ModelTarget> const &
ModelBase::getTarget() const noexcept
{
	return m_impl->m_target;
}


//============================================================================
Vector<SharedPtr<AST::StrategyBufferOpNode>> const&
ModelBase::getFeatures() const noexcept
{
	return m_impl->m_features;
}


//============================================================================
size_t
ModelBase::getCurrentIdx() const noexcept
{
	return m_exchange->currentIdx();
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
void
ModelBase::step() noexcept
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
	assert(target.rows() == m_target_buffer.rows());
	auto const col_slice = m_target_buffer.col(m_buffer_idx);
	m_target->evaluate(col_slice);

	switch (m_type)
	{
	case ModelTargetType::ABSOLUTE:
		target = col_slice;
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
	m_in_lookforward = false;
}


}


}