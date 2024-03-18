module;
#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API __declspec(dllimport)
#endif
export module RiskNodeModule;

import AtlasLinAlg;
import AtlasCore;
import BaseNodeModule;
import AtlasTimeModule;
import StrategyBufferModule;

namespace Atlas {

namespace AST {

//============================================================================
export class CovarianceNodeBase : public StatementNode {
private:
  bool m_cached = false;
  bool m_incremental = false;
  size_t m_warmup = 0;

protected:
  size_t m_lookback_window = 0;
  SharedPtr<TriggerNode> m_trigger;
  Exchange &m_exchange;
  LinAlg::EigenMatrixXd m_covariance;

  void enableIncremental() noexcept { m_incremental = true; }

public:
  CovarianceNodeBase(Exchange &exchange, SharedPtr<TriggerNode> trigger,
                     size_t lookback_window) noexcept;

  void reset() noexcept;
  virtual void evaluateChild() noexcept = 0;
  virtual void resetChild() noexcept = 0;
  void evaluate() noexcept override;
  bool getIsCached() const noexcept { return m_cached; }
  size_t getWarmup() const noexcept override { return m_warmup; }
  SharedPtr<TriggerNode> getTrigger() const noexcept { return m_trigger; }
  Exchange &getExchange() const noexcept { return m_exchange; }
  LinAlg::EigenMatrixXd const &getCovariance() const noexcept {
    return m_covariance;
  }
};

//============================================================================
export class CovarianceNode : public CovarianceNodeBase {
  friend class Exchange;

private:
  LinAlg::EigenMatrixXd m_centered_returns;

  CovarianceNode(Exchange &exchange, SharedPtr<TriggerNode> trigger,
                 size_t lookback_window) noexcept;

public:
  ~CovarianceNode() noexcept;

  template <typename... Arg>
  SharedPtr<CovarianceNode> static make(Arg &&...arg) {
    struct EnableMakeShared : public CovarianceNode {
      EnableMakeShared(Arg &&...arg)
          : CovarianceNode(std::forward<Arg>(arg)...) {}
    };
    return std::make_shared<EnableMakeShared>(std::forward<Arg>(arg)...);
  }

  void evaluateChild() noexcept override;
  void resetChild() noexcept override;
};

//============================================================================
export class IncrementalCovarianceNode : public CovarianceNodeBase {
  friend class Exchange;

private:
  size_t m_counter = 0;
  LinAlg::EigenMatrixXd m_sum;
  LinAlg::EigenMatrixXd m_product_buffer;
  LinAlg::EigenMatrixXd m_sum_product;

public:
  IncrementalCovarianceNode(Exchange &exchange, SharedPtr<TriggerNode> trigger,
                            size_t lookback_window) noexcept;
  ~IncrementalCovarianceNode() noexcept;

  template <typename... Arg>
  SharedPtr<IncrementalCovarianceNode> static make(Arg &&...arg) {
    struct EnableMakeShared : public IncrementalCovarianceNode {
      EnableMakeShared(Arg &&...arg)
          : IncrementalCovarianceNode(std::forward<Arg>(arg)...) {}
    };
    return std::make_shared<EnableMakeShared>(std::forward<Arg>(arg)...);
  }

  void evaluateChild() noexcept override;
  void resetChild() noexcept override;
};

//============================================================================
export class AllocationWeightNode : public StrategyBufferOpNode {
protected:
  SharedPtr<CovarianceNodeBase> m_covariance = nullptr;
  Option<double> m_vol_target = std::nullopt;

protected:
  void targetVol(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) const noexcept;

public:
  virtual ~AllocationWeightNode() noexcept;

  AllocationWeightNode(SharedPtr<CovarianceNodeBase> covariance,
                       Option<double> vol_target) noexcept;

  virtual void
  evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept = 0;
  void reset() noexcept final override {}
  bool getIsCached() const noexcept { return m_covariance->getIsCached(); }
};

//============================================================================
export class InvVolWeight final : public AllocationWeightNode {
private:
public:
  ATLAS_API ~InvVolWeight() noexcept;

  ATLAS_API InvVolWeight(SharedPtr<CovarianceNodeBase> covariance,
                         Option<double> vol_target = std::nullopt) noexcept;

  size_t getWarmup() const noexcept override {
    return m_covariance->getWarmup();
  }
  void
  evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept override;
  [[nodiscard]] bool
  isSame(SharedPtr<StrategyBufferOpNode> other) const noexcept override {
    return false;
  }
};

//============================================================================
export class RiskParityWeight final : public AllocationWeightNode {
private:
  double m_tol = 1e-6;
  int m_max_iter = 100;
  bool m_chinu = true;

public:
  ATLAS_API ~RiskParityWeight() noexcept;

  ATLAS_API RiskParityWeight(SharedPtr<CovarianceNodeBase> covariance,
                             Option<double> vol_target = std::nullopt,
                             double tol = 1e-6, int max_iter = 100,
                             bool m_chinu = true) noexcept;

  [[nodiscard]] double getTol() const noexcept { return m_tol; }
	[[nodiscard]] size_t getMaxIter() const noexcept { return m_max_iter; }
	[[nodiscard]] bool getChinu() const noexcept { return m_chinu; }

  size_t getWarmup() const noexcept override {
    return m_covariance->getWarmup();
  }
  void
  evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept override;
  [[nodiscard]] bool
  isSame(SharedPtr<StrategyBufferOpNode> other) const noexcept override;
};

} // namespace AST

} // namespace Atlas
