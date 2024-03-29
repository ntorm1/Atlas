#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API __declspec(dllimport)
#endif
#include "standard/AtlasCore.hpp"

namespace Atlas {

struct ExchangeMapImpl;

class ExchangeMap {
  friend class Hydra;

private:
  UniquePtr<ExchangeMapImpl> m_impl;

  void build() noexcept;
  void reset() noexcept;
  void step() noexcept;
  void cleanup() noexcept;

  Result<SharedPtr<Exchange>, AtlasException>
  addExchange(String name, String source,
              Option<String> datetime_format = std::nullopt) noexcept;
  Result<SharedPtr<Exchange>, AtlasException>
  getExchange(String const &name) const noexcept;

  size_t getCurrentIdx() const noexcept;
  size_t *getCurrentIdxPtr() const noexcept;
  Vector<Int64> const &getTimestamps() const noexcept;

public:
  ExchangeMap() noexcept;
  ATLAS_API ~ExchangeMap() noexcept;
  ExchangeMap(const ExchangeMap &) = delete;
  ExchangeMap(ExchangeMap &&) = delete;
  ExchangeMap &operator=(const ExchangeMap &) = delete;
  ExchangeMap &operator=(ExchangeMap &&) = delete;

  Result<SharedPtr<const Exchange>, AtlasException>
  getExchangeConst(String const &name) const noexcept;
  Option<String> getParentExchangeName(String const &asset_name) const noexcept;

  void removeExchange(String const &name) noexcept;

  //============================================================================
  ATLAS_API HashMap<String, size_t> const &getExchangeIds() const noexcept;
};

} // namespace Atlas