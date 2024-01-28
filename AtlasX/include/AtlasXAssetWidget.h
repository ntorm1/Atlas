#pragma once
#include <QWidget>
#include <QHeaderView>
#include <QMenu>
#include <QContextMenuEvent>

#include "../include/AtlasXTypes.h"

namespace AtlasX
{

struct AtlasXAssetImpl;



//============================================================================
class AtlasXAsset : public QWidget
{
	Q_OBJECT

public slots:
	void onHydraStep();
	void onHydraReset();

private slots:
	void showHeaderContextMenu(QPoint pos) noexcept;


private:
	AtlasXAssetImpl* impl = nullptr;

	void initUI() noexcept;
	void initPlot() noexcept;
    void initSignals() noexcept;
	void initData() noexcept;

    void plotColumn(int columnIndex) noexcept;
	void removeColumn(int columnIndex) noexcept;

public:
	AtlasXAsset(
		QWidget* parent,
		AtlasXAppImpl* app,
		SharedPtr<Atlas::Exchange> headers,
		Option<String> asset_name = std::nullopt
	) noexcept;
	~AtlasXAsset() noexcept;

};


}