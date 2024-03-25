#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API __declspec(dllimport)
#endif
#include "standard/AtlasCore.hpp"
#include "standard/AtlasLinAlg.hpp"
#include "ast/BaseNode.hpp"

namespace Atlas {

namespace AST {

//============================================================================
class StrategyBufferOpNode
    : public OpperationNode<void, LinAlg::EigenRef<LinAlg::EigenVectorXd>> {
  friend class Exchange;

private:
  Vector<StrategyBufferOpNode*> m_children;
  void setTakeFromCache(bool v) noexcept;

protected:
  Exchange &m_exchange;
  LinAlg::EigenMatrixXd m_cache;
  bool m_take_from_cache = false;

  
  StrategyBufferOpNode(NodeType t, Exchange &exchange,
                       Vector<SharedPtr<AST::StrategyBufferOpNode>> parent)
      : OpperationNode<void, LinAlg::EigenRef<LinAlg::EigenVectorXd>>(t,
                                                                      std::move(parent)),
        m_exchange(exchange) {}
  StrategyBufferOpNode(NodeType t, Exchange &exchange, Option<ASTNode *> parent)
      : OpperationNode<void, LinAlg::EigenRef<LinAlg::EigenVectorXd>>(t,
                                                                      parent),
        m_exchange(exchange) {}


  void enableCache(bool v = true) noexcept;
  [[nodiscard]] bool sameParents(Vector<ASTNode *> const &parent) const noexcept;
  [[nodiscard]] bool hasCache() const noexcept { return m_cache.cols() > 1; }
  [[nodiscard]] LinAlg::EigenRef<LinAlg::EigenVectorXd>
  cacheColumn(Option<size_t> col = std::nullopt) noexcept;

public:
  virtual ~StrategyBufferOpNode() = default;
  virtual size_t refreshWarmup() noexcept { return 0; }
  virtual [[nodiscard]] bool
  isSame(StrategyBufferOpNode const* other) const noexcept = 0;
  void addChild(StrategyBufferOpNode *child) noexcept;
  [[nodiscard]] size_t getAssetCount() const noexcept;
  [[nodiscard]] size_t getCurrentIdx() const noexcept;
  [[nodiscard]] Option<AllocationBaseNode *> getAllocationNode() const noexcept;
  [[nodiscard]] Exchange &getExchange() noexcept { return m_exchange; }

  ATLAS_API SharedPtr<StrategyBufferOpNode> lag(size_t lag) noexcept;
  ATLAS_API Option<Vector<double>>
  getAssetCacheSlice(size_t asset_index) const noexcept;
  ATLAS_API auto const &cache() noexcept { return m_cache; }
  ATLAS_API uintptr_t address() const noexcept {return reinterpret_cast<uintptr_t>(this); }
};

//============================================================================
class DummyNode : public StrategyBufferOpNode {
public:
  ATLAS_API DummyNode(SharedPtr<Exchange> exchange) noexcept
      : StrategyBufferOpNode(NodeType::NOP, *exchange, std::nullopt) {}
  ATLAS_API ~DummyNode() noexcept {}

  [[nodiscard]] size_t getWarmup() const noexcept override { return 0; }
  [[nodiscard]] bool
  isSame(StrategyBufferOpNode const* other) const noexcept override {
    return false;
  }
  void
  evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> _) noexcept override {}
  void reset() noexcept override {}
};

//============================================================================
class LagNode : public StrategyBufferOpNode {
private:
  size_t m_lag;
  size_t m_lag_cache_idx = 0;
  StrategyBufferOpNode *m_parent;

public:
  LagNode(StrategyBufferOpNode *parent, size_t lag) noexcept;
  ATLAS_API ~LagNode() noexcept {}

  [[nodiscard]] ATLAS_API size_t getWarmup() const noexcept override;
  [[nodiscard]] ATLAS_API bool
  isSame(StrategyBufferOpNode const* other) const noexcept override;
  ATLAS_API void
  evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept override;
  ATLAS_API void reset() noexcept override;
};

} // namespace AST

} // namespace Atlas