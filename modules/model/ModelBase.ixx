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
	LinearRegression,
};


//============================================================================
export class ModelConfig
{
public:
	size_t training_window;
	size_t walk_forward_window;
	ModelType type;
	SharedPtr<Exchange> exchange;
};


//============================================================================
export class ModelBase : public AST::StrategyBufferOpNode
{
private:
	ModelBaseImpl* m_impl;
	SharedPtr<Exchange> m_exchange;

protected:
	size_t m_asset_count;
	size_t m_feature_warmup = 0;
	size_t m_warmup = 0;
	LinAlg::EigenVectorXd m_signal;
	SharedPtr<ModelConfig> m_config;
	Vector<SharedPtr<AST::StrategyBufferOpNode>> const& getFeatures() const noexcept;

public:
	ModelBase(
		String id,
		Vector<SharedPtr<AST::StrategyBufferOpNode>> features,
		SharedPtr<AST::StrategyBufferOpNode> target,
		SharedPtr<ModelConfig> config
	) noexcept;
	~ModelBase() noexcept;

	[[nodiscard]] size_t getWarmup() const noexcept final override;
	void evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept final override;
	
	virtual void train() noexcept = 0;
	virtual void step() noexcept = 0;
	virtual void predict() noexcept = 0;
};



}

}