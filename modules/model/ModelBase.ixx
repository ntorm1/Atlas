module;
#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API  __declspec(dllimport)
#endif
export module ModelBaseModule;


import AtlasCore;
import StrategyBufferModule;

namespace Atlas
{

namespace Model
{


struct ModelBaseImpl;


//============================================================================
export enum ModelType
{
	LINEAR_REGRESSION,
};


//============================================================================//==
export enum ModelTargetType
{
	ABSOLUTE,
	DELTA,
	PERCENTAGE_CHANGE
};


//============================================================================
export class ModelTarget final : public AST::StrategyBufferOpNode
{
private:
	SharedPtr<AST::StrategyBufferOpNode> m_target;
	LinAlg::EigenMatrixXd m_target_buffer;
	ModelTargetType m_type;
	size_t m_lookforward;
	size_t m_buffer_idx = 0;
	bool m_in_lookforward = false;

public:
	ATLAS_API ModelTarget(
		SharedPtr<AST::StrategyBufferOpNode> target,
		ModelTargetType type,
		size_t lookforward
	) noexcept;
	ATLAS_API ~ModelTarget() noexcept;


	[[nodiscard]] bool isSame(SharedPtr<StrategyBufferOpNode> other) const noexcept override;
	[[nodiscard]] size_t getWarmup() const noexcept { return m_lookforward; }
	[[nodiscard]] size_t getLookForward() const noexcept { return m_lookforward; }
	void evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept override;
	void reset() noexcept override;
};



//============================================================================
export class ModelConfig
{
public:
	size_t training_window;
	size_t walk_forward_window;
	ModelType type;
	SharedPtr<Exchange> exchange;

	ModelConfig() = delete;
	ATLAS_API ModelConfig(
		size_t training_window,
		size_t walk_forward_window,
		ModelType model_type,
		SharedPtr<Exchange> exchange
	) noexcept;
	ATLAS_API ~ModelConfig() noexcept = default;
};


//============================================================================
export class ModelBase : public AST::StrategyBufferOpNode
{
	friend class Exchange;
private:
	ModelBaseImpl* m_impl;
	SharedPtr<Exchange> m_exchange;

	void stepBase() noexcept;

protected:
	size_t m_asset_count;
	size_t m_feature_warmup = 0;
	size_t m_warmup = 0;
	LinAlg::EigenVectorXd m_signal;
	SharedPtr<ModelConfig> m_config;
	[[nodiscard]] SharedPtr<ModelTarget> const& getTarget() const noexcept;
	[[nodiscard]] Vector<SharedPtr<AST::StrategyBufferOpNode>> const& getFeatures() const noexcept;
	[[nodiscard]] size_t getCurrentIdx() const noexcept;
public:
	ATLAS_API ModelBase(
		String id,
		Vector<SharedPtr<AST::StrategyBufferOpNode>> features,
		SharedPtr<ModelTarget> target,
		SharedPtr<ModelConfig> config
	) noexcept;
	ATLAS_API virtual ~ModelBase() noexcept;

	[[nodiscard]] String const& getId() const noexcept;
	[[nodiscard]] size_t getWarmup() const noexcept final override;
	void evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept final override;
	
	virtual void train() noexcept = 0;
	virtual void step() noexcept = 0;
	virtual void predict() noexcept = 0;
};


}

}