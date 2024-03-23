module;
#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API __declspec(dllimport)
#endif
export module MetaStrategyModule;

import AtlasCore;
import AtlasAllocatorModule;

namespace Atlas {

//============================================================================
struct MetaStrategyImpl;

//============================================================================
export class MetaStrategy : public Allocator {
  friend class Hydra;

private:
  UniquePtr<MetaStrategyImpl> m_impl;

  ATLAS_API void step() noexcept;
  ATLAS_API void step(LinAlg::EigenRef<LinAlg::EigenVectorXd>
                          target_weights_buffer) noexcept override;

  ATLAS_API void reset() noexcept override;
  ATLAS_API void load() noexcept override {}
  void enableCopyWeightsBuffer() noexcept override;
  size_t getWarmup() const noexcept override;

protected:
public:
  ~MetaStrategy() noexcept;
  ATLAS_API MetaStrategy(String name, SharedPtr<Exchange> exchange,
                         Option<SharedPtr<Allocator>>, double cash) noexcept;
  Vector<SharedPtr<Allocator>> getStrategies() const noexcept;
  const LinAlg::EigenRef<const LinAlg::EigenVectorXd>
  getAllocationBuffer() const noexcept override;
  const LinAlg::EigenRef<const LinAlg::EigenVectorXd>
  getAllocationBuffer(Allocator const *strategy) const noexcept;

  ATLAS_API SharedPtr<Allocator> pyAddStrategy(SharedPtr<Allocator> allocator,
                                               bool replace_if_exists = false);
};

} // namespace Atlas
