module;
#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API  __declspec(dllimport)
#endif
export module ModelNodeModule;

import AtlasCore;
import AtlasLinAlg;
import StrategyBufferModule;


namespace Atlas
{

namespace AST
{

struct ModelNodeImpl;

export class ModelNode final
	: public StrategyBufferOpNode
{
private:
	ModelNodeImpl* m_impl;
public:
	ModelNode(Exchange& exchange) noexcept;
	~ModelNode() noexcept;

};




}

}