#pragma once


#include "../include/AtlasXTypes.h"
#include "../include/AtlasXExchangeManager.h"


import AtlasException;

namespace AtlasX
{

struct AtlasXAppImpl
{
	friend class AtlasXApp;
private:
	SharedPtr<Atlas::Hydra> hydra;

	AtlasXExchangeManager* exchange_manager;

public:
	AtlasXAppImpl();
	~AtlasXAppImpl();


	//============================================================================
	Result<SharedPtr<Atlas::Exchange>, Atlas::AtlasException> addExchange(
		String name,
		String source
	) noexcept;

	//============================================================================
	Result<SharedPtr<Atlas::Exchange>, Atlas::AtlasException> getExchange(
		String name
	) noexcept;

	//============================================================================
	Result<bool, Atlas::AtlasException> deserialize(
		String path
	) noexcept;

	//============================================================================
	Result<bool, Atlas::AtlasException> serialize(
		String path
	) noexcept;

	//============================================================================
	HashMap<String, size_t> getExchangeIds() noexcept;

	//============================================================================
	HashMap<String, size_t> getAssetMap(SharedPtr<Atlas::Exchange> e) noexcept;
};


}