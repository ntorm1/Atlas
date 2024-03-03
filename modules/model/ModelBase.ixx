module;
#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API  __declspec(dllimport)
#endif
export module ModelBaseModule;


import AtlasCore;


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
export class ModelBase
{
private:
	ModelBaseImpl* m_impl;

protected:
	SharedPtr<Exchange> m_exchange;
	SharedPtr<ModelConfig> m_config;

	size_t getAssetCount() const noexcept;
	Vector<SharedPtr<AST::StrategyBufferOpNode>> const& getFeatures() const noexcept;

public:
	ModelBase(
		String id,
		Vector<SharedPtr<AST::StrategyBufferOpNode>> features,
		SharedPtr<AST::StrategyBufferOpNode> target,
		SharedPtr<ModelConfig> config
	) noexcept;
	~ModelBase() noexcept;

	virtual void train() noexcept = 0;
	virtual void reset() noexcept = 0;

};



}

}