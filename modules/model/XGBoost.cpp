module;
#include "AtlasFeature.hpp"
#include <cassert>

#ifdef ATLAS_XGBOOST
#include <xgboost/c_api.h>
#endif

module XGBoostModule;

#ifdef ATLAS_XGBOOST


namespace Atlas {

namespace Model {


//============================================================================
XGBoostModelConfig::XGBoostModelConfig(
    SharedPtr<ModelConfig> base_config) noexcept
    : m_base_config(std::move(base_config)) {}

//============================================================================
struct XGBoostModelImpl {
  DMatrixHandle dmatrix;
  BoosterHandle booster;

  XGBoostModelImpl() noexcept;
  ~XGBoostModelImpl() noexcept;
};

//============================================================================
XGBoostModelImpl::XGBoostModelImpl() noexcept
    : dmatrix(nullptr), booster(nullptr) {}

//============================================================================
XGBoostModelImpl::~XGBoostModelImpl() noexcept {
  if (dmatrix != nullptr)
    XGDMatrixFree(dmatrix);
  if (booster != nullptr)
    XGBoosterFree(booster);
}

//============================================================================
XGBoostModel::XGBoostModel(
    String id, Vector<SharedPtr<AST::StrategyBufferOpNode>> features,
    SharedPtr<ModelTarget> target,
    SharedPtr<const XGBoostModelConfig> config) noexcept
    : ModelBase(std::move(id), std::move(features), std::move(target),
                config->m_base_config),
      m_xgb_config(config) {
  m_impl = new XGBoostModelImpl();
  XGBoosterSetParam(m_impl->booster, "seed", "0");
}

//============================================================================
XGBoostModel::~XGBoostModel() noexcept { delete m_impl; }

//============================================================================
void XGBoostModel::train() noexcept {
  size_t look_forward = getTarget()->getLookForward();
  size_t training_window = m_config->training_window;
  LinAlg::EigenMatrixXf m_X_train(training_window - look_forward,
                                  getFeatures().size());
  LinAlg::EigenVectorXf m_y_train(training_window - look_forward);
  copyBlocks<float, LinAlg::EigenMatrixXf, LinAlg::EigenVectorXf>(m_X_train,
                                                                  m_y_train);

  auto res = XGDMatrixCreateFromMat(m_X_train.data(), m_X_train.rows(),
                                    m_X_train.cols(), 0, &m_impl->dmatrix);
  assert(res == 0);

  res = XGDMatrixSetFloatInfo(m_impl->dmatrix, "label", m_y_train.data(),
                              m_y_train.size());
  assert(res == 0);
  XGBoosterCreate(&m_impl->dmatrix, 1, &m_impl->booster);
  assert(!(XGBoosterSetParam(m_impl->booster, "booster", "gblinear")));
  assert(!(XGBoosterSetParam(m_impl->booster, "max_depth", "3")));
  assert(!(XGBoosterSetParam(m_impl->booster, "eta", "0.1")));

  int num_of_iterations = 20;
  for (int i = 0; i < num_of_iterations; ++i) {
    assert(!(XGBoosterUpdateOneIter(m_impl->booster, i, m_impl->dmatrix)));
  }
}

//============================================================================
void XGBoostModel::reset() noexcept {
  XGBoosterFree(m_impl->booster);
  m_impl->booster = BoosterHandle();
}

//============================================================================
void XGBoostModel::predict() noexcept {
  auto x_block = getXPredictionBlock();
  LinAlg::EigenMatrixXf x_block_float = x_block.cast<float>();
  auto res = XGDMatrixCreateFromMat(x_block_float.data(), x_block_float.rows(),
                                    x_block_float.cols(), 0, &m_impl->dmatrix);
  assert(res == 0);
  char const config[] =
      "{\"training\": false, \"type\": 0, "
      "\"iteration_begin\": 0, \"iteration_end\": 0, \"strict_shape\": false}";

  // Shape of output prediction
  uint64_t const *out_shape;

  // Dimension of output prediction
  uint64_t out_dim;

  // Pointer to a thread local contiguous array, assigned in prediction
  // function.
  float const *out_result = nullptr;
  XGBoosterPredictFromDMatrix(m_impl->booster, m_impl->dmatrix, config,
                              &out_shape, &out_dim, &out_result);

  // copy prediction into signal buffer
  assert(out_dim == 1);
  assert(out_shape[0] == m_signal.rows());
  double *signal_buffer = m_signal.data();
  for (int i = 0; i < m_signal.rows(); ++i) {
    signal_buffer[i] = static_cast<double>(out_result[i]);
  }
}

//============================================================================
bool XGBoostModel::isSame(
    StrategyBufferOpNode const* other) const noexcept {
  return false;
}

}


} // namespace Atlas

#endif
