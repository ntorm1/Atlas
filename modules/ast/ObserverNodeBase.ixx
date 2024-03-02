module;
#pragma once
#define NOMINMAX
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API  __declspec(dllimport)
#endif
export module ObserverNodeBaseModule;


import AtlasCore;
import BaseNodeModule;
import StrategyBufferModule;
import AtlasLinAlg;

namespace Atlas
{

namespace AST
{


class AssetObserverImpl;


//============================================================================
export enum class AssetObserverType : Uint8
{
	SUM = 0,
	MEAN = 1,
	ATR = 2,
	MAX = 3,
	TS_ARGMAX = 4,
	VARIANCE = 5,
};




//============================================================================
export class AssetObserverNode
	: public StrategyBufferOpNode
{
	friend class Exchange;
	friend class GridDimensionObserver;
	friend class StrategyGrid;
private:
	Option<String> m_id = std::nullopt;

	/// <summary>
	/// Warmup of the base StrategyBufferOpNode type. Defines the first step in exchange
	/// index where the node is valid
	/// </summary>
	size_t m_warmup = 0;

	/// <summary>
	///	Defines the first index in which to call the child's cacheObserver method
	/// </summary>
	size_t m_observer_warmup = 0;

	/// <summary>
	/// Rolling window size of observation
	/// </summary>
	size_t m_window = 0;

	/// <summary>
	/// Buffer matrix holding previous observations of the observer.
	/// </summary>
	LinAlg::EigenMatrixXd m_buffer_matrix;

	/// <summary>
	///	Current index into the buffer to write to. On cacheObserver, the m_buffer_idx - 1 
	/// column is overwritten with the new observation. Then onOutOfRange is called on m_buffer_idx
	/// to update the observer with the column that is about to be overwritten in the next step 
	/// </summary>
	size_t m_buffer_idx = 0;

	SharedPtr<StrategyBufferOpNode> parent() const noexcept { return m_parent; }

protected:
	LinAlg::EigenVectorXd m_signal;
	LinAlg::EigenVectorXd m_signal_copy;

	void setObserverWarmup(size_t warmup) noexcept { m_observer_warmup = warmup; }
	void setWarmup(size_t warmup) noexcept { m_warmup = warmup; }
	void setObserverBuffer(double c) noexcept;
	[[nodiscard]] size_t getBufferIdx() const noexcept { return m_buffer_idx; }
	[[nodiscard]] size_t getWindow() const noexcept { return m_window; }

public:
	AssetObserverNode(
		Option<String> name,
		SharedPtr<StrategyBufferOpNode> parent,
		AssetObserverType observer_type,
		size_t window
	) noexcept;
	ATLAS_API virtual ~AssetObserverNode() noexcept;

	SharedPtr<StrategyBufferOpNode> m_parent;
	AssetObserverType m_observer_type;

	[[nodiscard]] auto const& getBufferMatrix() const noexcept { return m_buffer_matrix; }

	/// <summary>
	/// Zero out the buffer matrix and call reset on derived implementations
	/// </summary>
	void resetBase() noexcept;

	/// <summary>
	/// On exchange step method is called to update the observer with the new observation
	/// </summary>
	void cacheBase() noexcept;

	/// <summary>
	/// Enable signal copy of observer has child observer that needs to copy the signal before 
	/// out of range is called 
	/// </summary>
	void enableSignalCopy() noexcept;

	/// <summary>
	/// Get const ref to the signal copy
	/// </summary>
	/// <returns></returns>
	auto const& getSignalCopy() const noexcept { return m_signal_copy; }

	/// <summary>
	/// Get a mutable ref into the observers buffer matrix at the current index
	/// </summary>
	/// <returns></returns>
	LinAlg::EigenRef<LinAlg::EigenVectorXd> buffer() noexcept;

	/// <summary>
	/// Called when column is out of range from the rolling window
	/// </summary>
	/// <param name="buffer_old"></param>
	virtual void onOutOfRange(LinAlg::EigenRef<LinAlg::EigenVectorXd> buffer_old) noexcept = 0;

	/// <summary>
	/// Pure virtual method defining the observer's evaluation method
	/// </summary>
	virtual void cacheObserver() noexcept = 0;

	/// <summary>
	/// Pur virtual method defining the observer's reset method called on exchange reset
	/// </summary>
	virtual void reset() noexcept = 0;


	[[nodiscard]] auto const& getId() const noexcept { return m_id; }
	[[nodiscard]] bool isSame(SharedPtr<StrategyBufferOpNode> other) const noexcept final override;
	[[nodiscard]] size_t getWarmup() const noexcept final override { return m_warmup; }
	[[nodiscard]] AssetObserverType getObserverType() const noexcept { return m_observer_type; }
	[[nodiscard]] size_t window() const noexcept { return m_window; }
};


}

}