#include <cassert>
#include "qcustomplot.h"
#include "AtlasXTypes.h"


namespace AtlasX
{

class AtlasXStrategyPlotBuilder;
class AtlasXAssetPlotBuilder;


//============================================================================
class AtlasPlot : public QCustomPlot
{
	Q_OBJECT

protected slots:
	virtual void contextMenuRequest(QPoint pos);
	void moveLegend();
	void mousePress();
	void mouseWheel();
	void selectionChanged();
	virtual void removeSelectedGraph();

public slots:
	virtual void removeGraphByName(std::string const& name);
	virtual void removeAllGraphs();

protected:
	std::optional<std::string> selected_line = std::nullopt;

public:
	AtlasPlot(QWidget* parent);
	~AtlasPlot();
	void setTitle(std::string title);
	void plot(
		Vector<Int64> const& x,
		Vector<double> const& y,
		std::string name,
		size_t start_idx = 0
	);
	void plot(
		std::span<const Int64> x,
		std::span<const double> y,
		std::string name,
		size_t start_idx = 0
	);
};


//============================================================================
class AtlasStrategyPlot final : public AtlasPlot
{
	Q_OBJECT

private:
	AtlasXStrategyPlotBuilder* m_builder = nullptr;
	String m_strategy_name;

	void addPlot(QString const& name);

protected slots:
	void contextMenuRequest(QPoint pos) override;

public:
	AtlasStrategyPlot(
		QWidget* parent,
		AtlasXStrategyPlotBuilder* builder,
		String const& strategy_name
	);
	~AtlasStrategyPlot();
};


//============================================================================
class AtlasAssetPlot final : public AtlasPlot
{
	Q_OBJECT

private:
	AtlasXAssetPlotBuilder* m_builder = nullptr;
	String m_asset_name;
	SharedPtr<Atlas::Exchange> m_exchange;
	Vector<double> m_asset_data;
	Set<String> m_column_names;
	Set<String> m_nodes;

	void addColumn(QString const& name);

protected slots:
	void contextMenuRequest(QPoint pos) override;

public:
	AtlasAssetPlot(
		QWidget* parent,
		AtlasXAssetPlotBuilder* builder,
		String const& asset_name
	) noexcept;
	~AtlasAssetPlot() noexcept;


};


}