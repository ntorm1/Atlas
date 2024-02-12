#pragma once

#include <QMainWindow>
#include <QtDataVisualization/Q3DSurface>
#include <QtDataVisualization/QSurfaceDataProxy>
#include <QtDataVisualization/QHeightMapSurfaceDataProxy>
#include <QtDataVisualization/QSurface3DSeries>
#include <QtWidgets/QSlider>

#include "AtlasXTypes.h"



namespace AtlasX
{

struct GridState;


class SurfaceGraph : public QObject
{
    Q_OBJECT
public:
    explicit SurfaceGraph(Q3DSurface* surface);
    ~SurfaceGraph();

    void fillGridProxy();
    void enableGrid(bool enable);
    void enableHeightMapModel(bool enable);
    void enableSqrtSinModel(bool enable);

    void toggleModeNone() { m_graph->setSelectionMode(QAbstract3DGraph::SelectionNone); }
    void toggleModeItem() { m_graph->setSelectionMode(QAbstract3DGraph::SelectionItem); }
    void toggleModeSliceRow() {
        m_graph->setSelectionMode(QAbstract3DGraph::SelectionItemAndRow
            | QAbstract3DGraph::SelectionSlice);
    }
    void toggleModeSliceColumn() {
        m_graph->setSelectionMode(QAbstract3DGraph::SelectionItemAndColumn
            | QAbstract3DGraph::SelectionSlice);
    }

    void setBlackToYellowGradient();
    void setGreenToRedGradient();

    void setAxisMinSliderX(QSlider* slider) { m_axisMinSliderX = slider; }
    void setAxisMaxSliderX(QSlider* slider) { m_axisMaxSliderX = slider; }
    void setAxisMinSliderZ(QSlider* slider) { m_axisMinSliderZ = slider; }
    void setAxisMaxSliderZ(QSlider* slider) { m_axisMaxSliderZ = slider; }

    void adjustXMin(int min);
    void adjustXMax(int max);
    void adjustZMin(int min);
    void adjustZMax(int max);
    void setGridState(SharedPtr<GridState> state);

public Q_SLOTS:
    void changeTheme(int theme);

private:
    Q3DSurface* m_graph;
    QHeightMapSurfaceDataProxy* m_heightMapProxy;
    QSurfaceDataProxy* proxy;
    QSurface3DSeries* m_heightMapSeries;
    QSurface3DSeries* series;
    Option<SharedPtr<GridState>> m_gridState = std::nullopt;

    QSlider* m_axisMinSliderX;
    QSlider* m_axisMaxSliderX;
    QSlider* m_axisMinSliderZ;
    QSlider* m_axisMaxSliderZ;
    float m_rangeMinX;
    float m_rangeMinZ;
    float m_stepX;
    float m_stepZ;
    int m_heightMapWidth;
    int m_heightMapHeight;

    double m_sampleMin = -8.0f;
    double m_sampleMax = 8.0f;

    void setAxisXRange(float min, float max);
    void setAxisZRange(float min, float max);
    void fillSqrtSinProxy();
};


class AtlasXOptimizer : public QMainWindow
{
    Q_OBJECT

private:
    SurfaceGraph* modifier;
    Option<SharedPtr<GridState>> m_gridState = std::nullopt;

public:

    void setGridState(SharedPtr<GridState> state);

    AtlasXOptimizer(QWidget* parent);
	~AtlasXOptimizer();
};


}