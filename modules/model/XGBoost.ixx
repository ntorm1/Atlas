module;
#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API  __declspec(dllimport)
#endif

#include "AtlasFeature.hpp"

export module XGBoostModule;

#ifdef ATLAS_XGBOOST

import ModelBaseModule;
import AtlasLinAlg;
import AtlasCore;

namespace Atlas
{

namespace Model
{

class XGBoostModel;

//============================================================================
export class XGBoostModelConfig
{
	friend class XGBoostModel;
private:
	SharedPtr<ModelConfig> m_base_config;

public:
	ATLAS_API XGBoostModelConfig(
		SharedPtr<ModelConfig> base_config
	) noexcept;
	ATLAS_API ~XGBoostModelConfig() noexcept = default;
};


struct XGBoostModelImpl;

//============================================================================
export class XGBoostModel : public ModelBase
{
private:
	XGBoostModelImpl* m_impl;
	SharedPtr<const XGBoostModelConfig> m_xgb_config;
public:
	ATLAS_API XGBoostModel(
		String id,
		Vector<SharedPtr<AST::StrategyBufferOpNode>> features,
		SharedPtr<ModelTarget> target,
		SharedPtr<const XGBoostModelConfig> config
	) noexcept;
	ATLAS_API ~XGBoostModel() noexcept;

	void train() noexcept override;
	void reset() noexcept override;
	void predict() noexcept override;
	[[nodiscard]] bool isSame(StrategyBufferOpNode const* other) const noexcept override;
};



}

}

#endif
