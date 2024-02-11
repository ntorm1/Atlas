#include <cassert>
#include "qcustomplot.h"
#include "AtlasXTypes.h"


namespace AtlasX
{

class AtlasXPlotBuilder;


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

protected:
	std::optional<std::string> selected_line = std::nullopt;

public:
	AtlasPlot(QWidget* parent);
	~AtlasPlot();
	void setTitle(std::string title);
	void plot(
		std::span<const Int64> x,
		std::span<const double> y,
		std::string name
	);
};


//============================================================================
class AtlasStrategyPlot final : public AtlasPlot
{
	Q_OBJECT

private:
	AtlasXPlotBuilder* m_builder = nullptr;

	void addPlot(QString const& name);

protected slots:
	void contextMenuRequest(QPoint pos) override;

public:
	AtlasStrategyPlot(
		QWidget* parent,
		AtlasXPlotBuilder* builder
	);
	~AtlasStrategyPlot();

};


}