module;
#include <boost/math/distributions/students_t.hpp>
#include <Eigen/Dense>
#include <Eigen/IterativeLinearSolvers>
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
static double
calculateRSquared(
	const LinAlg::EigenVectorXd& y,       
	const LinAlg::EigenVectorXd& y_hat,   
	bool use_adjusted = false             
) noexcept {
	// Calculate total sum of squares (TSS)
	double y_mean = y.mean();
	double tss = (y.array() - y_mean).square().sum();

	// Calculate residual sum of squares (RSS)
	double rss = (y - y_hat).array().square().sum();

	// Calculate number of observations and number of predictors
	int n = static_cast<int>(y.size());
	int p = 1; // assuming only one predictor for simplicity

	// Calculate R-squared
	double r_squared = 1.0 - (rss / tss);

	// If adjusted R-squared is requested and there are predictors
	if (use_adjusted && p > 0) {
		// Calculate adjusted R-squared
		double adj_r_squared = 1.0 - ((1.0 - r_squared) * (n - 1) / (n - p - 1));
		return adj_r_squared;
	}
	else {
		return r_squared;
	}
}


//============================================================================
static LinAlg::EigenMatrixXd
gramSchmidtOrthogonalization(
	LinAlg::EigenMatrixXd const& A
) {
	// Number of rows and columns in the matrix
	int numRows = static_cast<int>(A.rows());
	int numCols = static_cast<int>(A.cols());

	// Create a matrix to store the orthogonalized columns
	LinAlg::EigenMatrixXd orthoMatrix(numRows, numCols);

	// Iterate through each column of the input matrix
	for (int j = 0; j < numCols; ++j) {
		// Get the j-th column of the input matrix
		LinAlg::EigenVectorXd v = A.col(j);
		// Normalize the current vector
		double norm = v.norm();
		if (norm != 0) {
			v /= norm;
		}

		// Subtract the projection of v onto the previous orthogonalized columns
		for (int k = j + 1; k < numCols; ++k) {
			LinAlg::EigenVectorXd vk = A.col(k);
			double dotProduct = v.dot(vk);
			vk -= dotProduct * v;
			A.col(k) = vk;
		}

		// Store the normalized and orthogonalized vector in the result matrix
		orthoMatrix.col(j) = v;
	}
	return orthoMatrix;
}


//============================================================================
static LinAlg::EigenVectorXd
lassoRegression(
	LinAlg::EigenMatrixXd const& X,
	LinAlg::EigenVectorXd const& y,
	double lambda,
	double epsilon,
	size_t max_iter
) noexcept 
{
	// Initialize coefficients vector to zeros
	LinAlg::EigenVectorXd coef_ = LinAlg::EigenVectorXd::Zero(X.cols());

	// Iterate until convergence or maximum iterations
	for (size_t iter = 0; iter < max_iter; ++iter) {
		// Calculate squared column sums of X
		auto z = (X.array() * X.array()).colwise().sum();

		// Temporary vector to store updated coefficients
		LinAlg::EigenVectorXd tmp = LinAlg::EigenVectorXd::Zero(X.cols());

		// Iterate over each feature
		for (int k = 0; k < X.cols(); ++k) {
			// Save the current coefficient value
			double wk = coef_(k);

			// Set coefficient of feature k to zero
			coef_(k) = 0;

			// Compute the dot product of X(:,k) with the residual
			double p_k = X.col(k).transpose() * (y - X * coef_);

			// Compute the soft-thresholded value
			double w_k = 0.0;
			if (p_k < -lambda / 2.0)
				w_k = (p_k + lambda / 2.0) / z(k);
			else if (p_k > lambda / 2.0)
				w_k = (p_k - lambda / 2.0) / z(k);
			else
				w_k = 0.0;

			// Store the updated coefficient
			tmp(k) = w_k;

			// Restore the original value of the coefficient
			coef_(k) = wk;
		}

		// Check for convergence
		if ((coef_ - tmp).norm() < epsilon)
			break;

		// Update coefficients
		coef_ = tmp;
	}

	// Return the final coefficient vector
	return coef_;
}


//============================================================================
static LinAlg::EigenVectorXd
ridgeRegression(
	LinAlg::EigenMatrixXd const& X,
	LinAlg::EigenVectorXd const& y,
	double alpha
) noexcept
{
	// Compute coefficients using closed-form solution
	int n_features = static_cast<int>(X.cols());
	LinAlg::EigenMatrixXd I = LinAlg::EigenMatrixXd::Identity(n_features, n_features);
	LinAlg::EigenVectorXd coef = (X.transpose() * X + alpha * I).ldlt().solve(X.transpose() * y);
	return coef;
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