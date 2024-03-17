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
class MetaStrategyImpl;

//============================================================================
export class MetaStrategy : public Allocator {
private:
  UniquePtr<MetaStrategyImpl> m_impl;

  ATLAS_API void step() noexcept override {}
  ATLAS_API void reset() noexcept override {}
  ATLAS_API void realize() noexcept override {}
  ATLAS_API void load() noexcept override {}

protected:
public:
  ATLAS_API MetaStrategy(String name, SharedPtr<Exchange> exchange,
                         Option<SharedPtr<Allocator>>, double cash) noexcept;

  ~MetaStrategy() noexcept;
};

} // namespace Atlas
