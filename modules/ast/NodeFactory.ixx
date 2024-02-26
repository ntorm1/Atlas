module;
#pragma once
#ifdef ATLAS_EXPORTS
#define ATLAS_API __declspec(dllexport)
#else
#define ATLAS_API  __declspec(dllimport)
#endif
export module NodeFactoryModule;

import AtlasCore;


namespace Atlas
{

namespace AST
{

export class NodeFactory
{
private:
	String m_strategy_id;
	SharedPtr<Exchange> m_exchange;

public:
	NodeFactory(
		String strategy_id,
		SharedPtr<Exchange> exchange
	) noexcept;
	~NodeFactory() noexcept;

};




}

}