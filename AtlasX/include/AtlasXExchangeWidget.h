#pragma once
#include <QWidget>

#include <DockWidget.h>

#include "../include/AtlasXTypes.h"

namespace AtlasX
{

struct AtlasXExchangeImpl;

class AtlasXExchange : public QWidget
{
	friend class AtlasXAppImpl;

	Q_OBJECT
private:
	AtlasXExchangeImpl* m_impl;

	void initUi() noexcept;

public:
	AtlasXExchange(
		QWidget* parent,
		AtlasXAppImpl* app,
		SharedPtr<Atlas::Exchange>
	) noexcept;
	~AtlasXExchange() noexcept;

};

}