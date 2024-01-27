module;
#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API  __declspec(dllimport)
#endif
export module ExchangeModule;

import AtlasCore;
import AtlasLinAlg;


namespace Atlas
{

struct ExchangeImpl;

using namespace LinAlg;

//============================================================================
export class Exchange
{
	friend class ExchangeMap;
	friend class AST::AssetReadNode;
	friend class AST::TriggerNode;
private:
	UniquePtr<ExchangeImpl> m_impl;
	String m_name;
	String m_source;
	size_t m_id;

	[[nodiscard]] Result<bool,AtlasException> init() noexcept;
	[[nodiscard]] Result<bool,AtlasException> validate() noexcept;
	[[nodiscard]] Result<bool,AtlasException> build() noexcept;

	void reset() noexcept;
	void step(Int64 global_time) noexcept;
	void registerTrigger(AST::TriggerNode* trigger) noexcept;

	Option<size_t> getCloseIndex() const noexcept;
	EigenConstColView<double> getSlice(size_t column, int row_offset) const noexcept;

public:
	Exchange(
		String name,
		String source,
		size_t id
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

	void registerStrategy(Strategy* strategy) noexcept;
	auto const& getSource() const noexcept{return m_source;}
	size_t currentIdx() const noexcept;
	EigenBlockView<double> getMarketReturnsBlock(size_t start_idex, size_t end_idx) const noexcept;
	Option<size_t> getColumnIndex(String const& column) const noexcept;
	size_t getNullCount(int row_offset) const noexcept;

	// ======= PUBLIC API ======= //
	ATLAS_API EigenConstColView<double> getMarketReturns(int row_offset = 0) const noexcept;
	ATLAS_API String const& getName() const noexcept {return m_name;}
	ATLAS_API Option<size_t> getAssetIndex(String const& asset) const noexcept;
	ATLAS_API size_t getAssetCount() const noexcept;
	ATLAS_API Vector<Int64> const& getTimestamps() const noexcept;
};

}