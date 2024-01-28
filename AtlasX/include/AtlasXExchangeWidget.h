#pragma once
#include <QWidget>


#include "../include/AtlasXTypes.h"

namespace AtlasX
{

struct AtlasXExchangeImpl;


//============================================================================
class AtlasXExchange : public QWidget
{
	friend class AtlasXAppImpl;

	Q_OBJECT


signals:
	void hydraStep();
	void hydraReset();

public slots:
	void onHydraStep();
	void onHydraReset();

private:
	AtlasXExchangeImpl* m_impl;

	void initUi() noexcept;
	void onAssetSelected() noexcept;
	void connectAsset(AtlasXAsset* asset_widget) noexcept;

public:
	AtlasXExchange(
		QWidget* parent,
		AtlasXAppImpl* app,
		SharedPtr<Atlas::Exchange>,
		String const& name
	) noexcept;
	~AtlasXExchange() noexcept;
};


}