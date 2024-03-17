module;
module MetaStrategyModule;

namespace Atlas {

//============================================================================
class MetaStrategyImpl {
  
};

//============================================================================
MetaStrategy::MetaStrategy(String name, SharedPtr<Exchange> exchange,
                           Option<SharedPtr<Allocator>> parent,
                           double cash) noexcept
    : Allocator(name, *exchange, parent, cash) {
  m_impl = std::make_unique<MetaStrategyImpl>();
}

//============================================================================
MetaStrategy::~MetaStrategy() noexcept {}

} // namespace Atlas