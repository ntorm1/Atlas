#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API  __declspec(dllimport)
#endif
#include "standard/AtlasCore.hpp"
#include "standard/AtlasLinAlg.hpp"
#include "standard/AtlasEnums.hpp"

namespace Atlas
{

struct ExchangeImpl;


//============================================================================
class Exchange
{
	friend class ExchangeMap;
	friend class AST::AssetReadNode;
	friend class AST::TriggerNode;
	friend class AST::StrategyBufferOpNode;
private:
	UniquePtr<ExchangeImpl> m_impl;
	String m_name;
	String m_source;
	size_t m_id;

	[[nodiscard]] Result<bool, AtlasException> initDir() noexcept;
	[[nodiscard]] Result<bool,AtlasException> init() noexcept;
	[[nodiscard]] Result<bool,AtlasException> validate() noexcept;
	[[nodiscard]] Result<bool,AtlasException> build() noexcept;

	[[nodiscard]] SharedPtr<AST::TriggerNode> registerTrigger(SharedPtr<AST::TriggerNode>&& trigger) noexcept;
	void reset() noexcept;
	void enableCache() noexcept;
	void step(Int64 global_time) noexcept;
	void cleanupCovarianceNodes() noexcept;
	void cleanupTriggerNodes() noexcept;
	void setExchangeOffset(size_t _offset) noexcept;

public:
	Exchange(
		String name,
		String source,
		size_t id,
		Option<String> datetime_format = std::nullopt
	) noexcept;


	// == THROWS == //
	Exchange(
		Vector<String> const& columns,
		Vector<Int64> const& timestamps,
		Vector<Vector<double>> const& data,
		size_t id
	);

	ATLAS_API ~Exchange();
	Exchange(const Exchange&) = delete;
	Exchange(Exchange&&) = delete;
	Exchange& operator=(const Exchange&) = delete;
	Exchange& operator=(Exchange&&) = delete;

	void registerAllocator(Allocator *strategy) noexcept;
	auto const& getSource() const noexcept{return m_source;}
	size_t currentIdx() const noexcept;
	Option<SharedPtr<AST::StrategyBufferOpNode>> getSameFromCache(SharedPtr<AST::StrategyBufferOpNode> a) noexcept;
	LinAlg::EigenMatrixXd const& getData() const noexcept;
	LinAlg::EigenVectorXd const& getReturnsScalar() const noexcept;
	LinAlg::EigenBlockView<double> getMarketReturnsBlock(size_t start_idex, size_t end_idx) const noexcept;
	LinAlg::EigenConstColView<double> getSlice(size_t column, int row_offset) const noexcept;
	Option<size_t> getColumnIndex(String const& column) const noexcept;
	Option<size_t> getCloseIndex() const noexcept;
	Option<String> getDatetimeFormat() const noexcept;
	size_t getExchangeOffset() const noexcept;

	// ======= PUBLIC API ======= //
	ATLAS_API SharedPtr<AST::CovarianceNodeBase> getCovarianceNode(
		String const& id,
		SharedPtr<AST::TriggerNode> trigger,
		size_t lookback,
		CovarianceType type
	) noexcept;

	ATLAS_API void registerModel(SharedPtr<Model::ModelBase> model) noexcept;
	ATLAS_API Option<SharedPtr<AST::AssetObserverNode>> getObserver(String const& id) noexcept;
	ATLAS_API SharedPtr<AST::AssetObserverNode> registerObserver(SharedPtr<AST::AssetObserverNode> observer) noexcept;
	ATLAS_API HashMap<String, SharedPtr<AST::StrategyBufferOpNode>> getASTCache() const noexcept;
	ATLAS_API LinAlg::EigenConstRowView<double> getAssetSlice(size_t asset_index) const noexcept;
	ATLAS_API LinAlg::EigenConstColView<double> getMarketReturns(int row_offset = 0) const noexcept;
	ATLAS_API String const& getName() const noexcept {return m_name;}
	ATLAS_API Option<size_t> getAssetIndex(String const& asset) const noexcept;
	ATLAS_API HashMap<String, size_t> const& getAssetMap() const noexcept;
	ATLAS_API HashMap<String, size_t> const& getHeaders() const noexcept;
	ATLAS_API HashMap<String, SharedPtr<AST::StrategyBufferOpNode>> getNodeCache() const noexcept;
	ATLAS_API size_t getAssetCount() const noexcept;
	ATLAS_API Int64 getCurrentTimestamp() const noexcept;
	ATLAS_API Vector<Int64> const& getTimestamps() const noexcept;
	ATLAS_API void enableNodeCache(String const& name, SharedPtr<AST::StrategyBufferOpNode> p, bool eager = false) noexcept;
};

}