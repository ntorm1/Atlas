module;
#include <Eigen/Dense>
#include <random>
module ClusterNodeModule;

import HelperNodesModule;

namespace Atlas {

namespace AST {

//============================================================================
static void cluster(Eigen::Ref<Eigen::MatrixXd> data,
                    Eigen::Ref<Eigen::VectorXi> cluster, size_t max_iter,
                    size_t cluster_count) noexcept {
  int rows = static_cast<int>(data.rows());
  int cols = static_cast<int>(data.cols());

  // init centroids randomly
  Eigen::MatrixXd centroids = Eigen::MatrixXd(cluster_count, cols);
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> dis(0, rows - 1);
  for (int i = 0; i < cluster_count; ++i) {
    int idx = dis(gen);
    centroids.row(i) = data.row(idx);
  }

  // k-means algorithm
  cluster.setZero();
  assert(cluster.rows() == data.rows());
  for (size_t iter = 0; iter < max_iter; ++iter) {
    // assign each data point to the nearest centroid
    for (int i = 0; i < rows; ++i) {
      double min_dist = std::numeric_limits<double>::max();
      int min_idx = 0;
      for (int j = 0; j < cluster_count; ++j) {
        double dist = (data.row(i) - centroids.row(j)).squaredNorm();
        if (dist < min_dist) {
          min_dist = dist;
          min_idx = j;
        }
      }
      cluster(i) = min_idx;
    }

    // update centroids
    Eigen::MatrixXd new_centroids = Eigen::MatrixXd(cluster_count, cols);
    Eigen::VectorXi counts = Eigen::VectorXi(cluster_count);
    counts.setZero();
    new_centroids.setZero();
    for (int i = 0; i < rows; ++i) {
      int idx = cluster(i);
      new_centroids.row(idx) += data.row(i);
      counts(idx) += 1;
    }
    for (int i = 0; i < cluster_count; ++i) {
      if (counts(i) > 0) {
        new_centroids.row(i) /= counts(i);
      }
    }
    if ((new_centroids - centroids).squaredNorm() < 1e-6) {
      break;
    }
    centroids = new_centroids;
  }
}

//============================================================================
static void deMean(Eigen::Ref<Eigen::VectorXd> data,
                   Eigen::Ref<Eigen::VectorXi> cluster, bool scale) noexcept {
  int rows = static_cast<int>(data.rows());
  Eigen::VectorXd means = Eigen::VectorXd::Zero(cluster.size());
  Eigen::VectorXd variance = Eigen::VectorXd::Zero(cluster.size());
  Eigen::VectorXd counts = Eigen::VectorXd::Zero(cluster.size());
  Eigen::VectorXd meansSquared = Eigen::VectorXd::Zero(cluster.size());
  for (int i = 0; i < rows; ++i) {
    means(cluster(i)) += data(i);
    meansSquared(cluster(i)) += data(i) * data(i);
    counts(cluster(i)) += 1;
  }
  means = means.cwiseQuotient(counts);
  meansSquared = meansSquared.cwiseQuotient(counts);
  for (int i = 0; i < rows; ++i) {
    data(i) -= means(cluster(i));
  }
  if (scale) {
    variance = meansSquared - means.cwiseProduct(means);
    for (int i = 0; i < rows; ++i) {
      data(i) /= std::sqrt(variance(cluster(i)));
    }
  }
}

//============================================================================
ClusterNode::ClusterNode(Vector<SharedPtr<StrategyBufferOpNode>> features,
                         SharedPtr<AST::StrategyBufferOpNode> target,
                         ClusterNodeConfig config) noexcept
    : StrategyBufferOpNode(NodeType::CLUSTER, target->getExchange(),
                           std::nullopt),
      m_features(std::move(features)), m_config(config),
      m_target(std::move(target)) {
  m_warmup = 0;
  for (auto const &feature : m_features) {
    m_warmup = std::max(m_warmup, feature->getWarmup());
  }
  m_cluster.resize(m_features.size());
  m_cluster.setZero();
  m_data.resize(getAssetCount(), m_features.size());
  m_last_index = std::numeric_limits<size_t>::max();
}
//============================================================================
ClusterNode::~ClusterNode() noexcept {}

//============================================================================
void ClusterNode::evaluate(
    LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept {
  if (m_config.trigger->evaluate() && getCurrentIdx() != m_last_index) {
    m_last_index = getCurrentIdx();
    for (size_t i = 0; i < m_features.size(); ++i) {
      m_features[i]->evaluate(m_data.col(i));
    }
    cluster(m_data, m_cluster, m_config.max_iterations, m_config.cluster_count);
  }
  m_target->evaluate(target);
  deMean(target, m_cluster, m_config.cluster_op == ClusterOp::STANDARDIZE);
}

} // namespace AST

} // namespace Atlas