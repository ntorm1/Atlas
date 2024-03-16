module;
#include <Eigen/Dense>
module PCAModule;

import RiskNodeModule;

namespace Atlas {

namespace AST {

//============================================================================
PCAModel::PCAModel(String id,
                   Vector<SharedPtr<AST::StrategyBufferOpNode>> features,
                   size_t components) noexcept
    : StrategyBufferOpNode(NodeType::ASSET_PCA, features[0]->getExchange(),
                           features[0].get()),
      m_components(components) {
  m_id = std::move(id);
  m_features = std::move(features);
  m_data.resize(getAssetCount(), features.size());
  m_components_data.resize(features.size(), components);
  m_components_data.setZero();
  m_data.setZero();
  m_last_index = std::numeric_limits<size_t>::max();
  m_warmup = 0;
  for (auto &feature : m_features) {
    feature->addChild(this);
    m_warmup = std::max(m_warmup, feature->getWarmup());
  }
}

//============================================================================
PCAModel::~PCAModel() noexcept {}

//============================================================================
void PCAModel::build() noexcept {
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
  auto centered = m_data.rowwise() - m_data.colwise().mean();
  auto cov = (centered.adjoint() * centered) / double(m_data.rows() - 1);

  // eigen decomposition of the covariance matrix
  Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> eig(cov);

  // get principal components sort in descending order
  auto const &eigen_vectors = eig.eigenvectors();

  // copy the first m_components principal components to m_components matrix
  m_components_data =
      eigen_vectors.block(0, 0, eigen_vectors.rows(), m_components);
}

//============================================================================
bool PCAModel::isSame(SharedPtr<StrategyBufferOpNode> other) const noexcept {
  if (other->getType() != NodeType::ASSET_PCA) {
    return false;
  }
  auto other_pca = static_cast<PCAModel *>(other.get());
  auto const &other_features = other_pca->getFeatures();
  if (m_features.size() != other_features.size()) {
		return false;
	}
  for (size_t i = 0; i < m_features.size(); ++i) {
    if (!m_features[i]->isSame(other_features[i])) {
			return false;
		}
	}
  return true;
}

//============================================================================
void PCAModel::evaluate(
    LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept {
  if (m_last_index != getCurrentIdx()) {
    build();
    m_last_index = getCurrentIdx();
  }
  target = (m_data * m_components_data).col(0);
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
