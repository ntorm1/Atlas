module;
#include <Eigen/Dense>
module PCAModule;

import RiskNodeModule;

namespace Atlas {

namespace AST {

//============================================================================
PCAModel::PCAModel(String id,
                   Vector<SharedPtr<AST::StrategyBufferOpNode>> features,
                   SharedPtr<CovarianceNodeBase> cov,
                   size_t components) noexcept
    : StrategyBufferOpNode(NodeType::ASSET_PCA, features[0]->getExchange(),
                           features[0].get()), m_cov(cov), m_components(components) {
  m_id = std::move(id);
  m_features = std::move(features);
  m_data.resize(getAssetCount(), features.size());
  m_components_data.resize(features.size(), components);
  m_components_data.setZero();
  m_data.setZero();
}

//============================================================================
PCAModel::~PCAModel() noexcept {}

//============================================================================
void PCAModel::evaluate(
    LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept {
  // evaluate features into m_data
  for (size_t i = 0; i < m_features.size(); ++i) {
    m_features[i]->evaluate(m_data.col(i));
  }

  // standardize the data using standard scaler over columns
  double M = static_cast<double>(m_data.rows());
  auto mean = m_data.colwise().mean();
  auto std_dev =
      ((m_data.rowwise() - mean).array().square().colwise().sum() / (M - 1))
          .array()
          .sqrt();

  // get the covariance matrix from covariance base node
  auto const &covaraiance_matrix = m_cov->getCovariance();

  // eigen decomposition of the covariance matrix
  Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> eig(covaraiance_matrix);

  // get principal components sort in descending order
  auto const& eigen_vectors = eig.eigenvectors();

  // copy the first m_components principal components to m_components matrix  
  m_components_data =
      eigen_vectors.block(0, 0, eigen_vectors.rows(), m_components);

  // project the data onto the principal components
  //target = m_data * m_components_matrix;
}


//============================================================================
void PCAModel::reset() noexcept {
  for (auto &feature : m_features) {
    feature->reset();
  }
  m_data.setZero();
}

} // namespace AST

} // namespace Atlas
