module;
#include <boost/math/distributions/students_t.hpp>
#include <Eigen/Dense>
module LinearRegressionModule;


namespace Atlas
{

namespace Model
{


//============================================================================
static void
calculatePValues(
		LinAlg::EigenMatrixXd const& X,
		LinAlg::EigenVectorXd const& y,
		LinAlg::EigenVectorXd const& theta,
		LinAlg::EigenVectorXd& p_values
	) noexcept
{
	// Calculate residuals
	auto residuals = y - X * theta;

	// Calculate residual sum of squares (RSS)
	auto rss = residuals.squaredNorm();

	// Calculate degrees of freedom
	auto df = static_cast<int>(X.rows() - X.cols());

	// Calculate estimate of error variance (sigma^2)
	auto sigma = rss / (df);

	// Calculate squared sigma
	auto sigma2 = sigma * sigma;

	// Calculate (X^T * X)^{-1}
	auto XTX = (X.transpose() * X).inverse();

	// Calculate standard errors of coefficients
	auto se = sigma2 * XTX.diagonal().cwiseSqrt();

	// Calculate t-values
	auto t_values = theta.cwiseQuotient(se);

	// Initialize Student's t-distribution with degrees of freedom
	boost::math::students_t dist(df);

	// Ensure the size of p_values matches the size of theta
	assert(p_values.size() == theta.size());

	// Calculate p-values for each coefficient
	for (int i = 0; i < theta.size(); i++)
	{
		// Calculate p-value using cumulative distribution function of Student's t-distribution
		p_values[i] = 2 * (1 - boost::math::cdf(dist, std::abs(t_values[i])));
	}
}


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
	copyBlocks<double, LinAlg::EigenMatrixXd, LinAlg::EigenVectorXd>(m_X_train, m_y_train);

	switch (m_lr_config->m_solver)
	{
		case LinearRegressionSolver::LDLT:
			m_theta = (m_X_train.transpose() * m_X_train).ldlt().solve(m_X_train.transpose() * m_y_train);
			break;
		case LinearRegressionSolver::ColPivHouseholderQR:
			m_theta = m_X_train.colPivHouseholderQr().solve(m_y_train);
			break;
	}
	if (m_pvalues.size())
	{
		calculatePValues(m_X_train, m_y_train, m_theta, m_pvalues);
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
	m_signal = getXPredictionBlock() * m_theta;
}


//============================================================================
bool
LinearRegressionModel::isSame(SharedPtr<StrategyBufferOpNode> other) const noexcept
{
	return false;
}

}

}