#include "../include/AtlasXScene.h"
#include "../include/AtlasInter.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <Q3DSurface>
#include <QtGui/QImage>
#include <QtCore/qmath.h>
#include <QtDataVisualization/QValue3DAxis>
#include <QtDataVisualization/Q3DTheme>
#include <QRadioButton>
#include <QGroupBox>
#include <QSlider>
#include <QComboBox>
#include <QtGui/QPainter>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>


namespace AtlasX
{


const int sampleCountX = 50;
const int sampleCountZ = 50;
const int heightMapGridStepX = 6;
const int heightMapGridStepZ = 6;


SurfaceGraph::SurfaceGraph(Q3DSurface* surface)
    : m_graph(surface)
{
    m_graph->setAxisX(new QValue3DAxis);
    m_graph->setAxisY(new QValue3DAxis);
    m_graph->setAxisZ(new QValue3DAxis);

    //! [0]
    proxy = new QSurfaceDataProxy();
    series = new QSurface3DSeries(proxy);
    //! [0]
    fillSqrtSinProxy();

    //! [2]
    QImage heightMapImage(":/maps/mountain");
    m_heightMapProxy = new QHeightMapSurfaceDataProxy(heightMapImage);
    m_heightMapSeries = new QSurface3DSeries(m_heightMapProxy);
    m_heightMapSeries->setItemLabelFormat(QStringLiteral("(@xLabel, @zLabel): @yLabel"));
    m_heightMapProxy->setValueRanges(34.0f, 40.0f, 18.0f, 24.0f);
    //! [2]
    m_heightMapWidth = heightMapImage.width();
    m_heightMapHeight = heightMapImage.height();
}

SurfaceGraph::~SurfaceGraph()
{
    delete m_graph;
}

//============================================================================
void
SurfaceGraph::fillSqrtSinProxy()
{
    float stepX = (m_sampleMax - m_sampleMin) / float(sampleCountX - 1);
    float stepZ = (m_sampleMax - m_sampleMin) / float(sampleCountZ - 1);

    QSurfaceDataArray* dataArray = new QSurfaceDataArray;
    dataArray->reserve(sampleCountZ);
    for (int i = 0; i < sampleCountZ; i++) {
        QSurfaceDataRow* newRow = new QSurfaceDataRow(sampleCountX);
        // Keep values within range bounds, since just adding step can cause minor drift due
        // to the rounding errors.
        float z = qMin(m_sampleMax, (i * stepZ + m_sampleMin));
        int index = 0;
        for (int j = 0; j < sampleCountX; j++) {
            float x = qMin(m_sampleMax, (j * stepX + m_sampleMin));
            float R = qSqrt(z * z + x * x) + 0.01f;
            float y = (qSin(R) / R + 0.24f) * 1.61f;
            (*newRow)[index++].setPosition(QVector3D(x, y, z));
        }
        *dataArray << newRow;
    }

    proxy->resetArray(dataArray);
}


//============================================================================
void
SurfaceGraph::fillGridProxy()
{
    if (!m_gridState) return;
    auto state = m_gridState.value();

    QSurfaceDataArray* dataArray = new QSurfaceDataArray;
    Vector<double> const& dim1 = state->dim1;
    Vector<double> const& dim2 = state->dim2;
    Vector<Vector<double>> const& data = state->data;

    for (int i = 0; i < dim2.size(); i++) {
		QSurfaceDataRow* newRow = new QSurfaceDataRow(dim1.size());
		for (int j = 0; j < dim1.size(); j++) {
			(*newRow)[j].setPosition(QVector3D(dim1[j], data[i][j], dim2[i]));
		}
		*dataArray << newRow;
	}
    proxy->resetArray(dataArray);
}


//============================================================================
void
SurfaceGraph::enableSqrtSinModel(bool enable)
{
    if (enable) {
        series->setDrawMode(QSurface3DSeries::DrawSurfaceAndWireframe);
        series->setFlatShadingEnabled(true);

        m_graph->axisX()->setLabelFormat("%.2f");
        m_graph->axisZ()->setLabelFormat("%.2f");
        m_graph->axisX()->setRange(m_sampleMin, m_sampleMax);
        m_graph->axisY()->setRange(0.0f, 2.0f);
        m_graph->axisZ()->setRange(m_sampleMin, m_sampleMax);
        m_graph->axisX()->setLabelAutoRotation(30);
        m_graph->axisY()->setLabelAutoRotation(90);
        m_graph->axisZ()->setLabelAutoRotation(30);

        m_graph->removeSeries(m_heightMapSeries);
        m_graph->addSeries(series);

        m_rangeMinX = m_sampleMin;
        m_rangeMinZ = m_sampleMin;
        m_stepX = (m_sampleMax - m_sampleMin) / float(sampleCountX - 1);
        m_stepZ = (m_sampleMax - m_sampleMin) / float(sampleCountZ - 1);
        m_axisMinSliderX->setMaximum(sampleCountX - 2);
        m_axisMinSliderX->setValue(0);
        m_axisMaxSliderX->setMaximum(sampleCountX - 1);
        m_axisMaxSliderX->setValue(sampleCountX - 1);
        m_axisMinSliderZ->setMaximum(sampleCountZ - 2);
        m_axisMinSliderZ->setValue(0);
        m_axisMaxSliderZ->setMaximum(sampleCountZ - 1);
        m_axisMaxSliderZ->setValue(sampleCountZ - 1);
    }
}


//============================================================================
void
SurfaceGraph::enableGrid(bool enable)
{
    if (!m_gridState)
    {
        return;
    }

    if (enable)
    {
        auto state = m_gridState.value();
        Vector<double> const& dim1 = state->dim1;
        Vector<double> const& dim2 = state->dim2;
        Vector<Vector<double>> const& data = state->data;

        double smallest = 1e10;
        double largest = -1e10;
        for (int i = 0; i < dim2.size(); i++) {
			for (int j = 0; j < dim1.size(); j++) {
				if (data[i][j] < smallest) {
					smallest = data[i][j];
				}
                if (data[i][j] > largest) {
					largest = data[i][j];
				}
			}
		}
        m_sampleMax = largest;
        m_sampleMin = smallest;
        enableSqrtSinModel(true);
    }
}


//============================================================================
void
SurfaceGraph::enableHeightMapModel(bool enable)
{
    if (enable) {
        //! [4]
        m_heightMapSeries->setDrawMode(QSurface3DSeries::DrawSurface);
        m_heightMapSeries->setFlatShadingEnabled(false);

        m_graph->axisX()->setLabelFormat("%.1f N");
        m_graph->axisZ()->setLabelFormat("%.1f E");
        m_graph->axisX()->setRange(34.0f, 40.0f);
        m_graph->axisY()->setAutoAdjustRange(true);
        m_graph->axisZ()->setRange(18.0f, 24.0f);

        m_graph->axisX()->setTitle(QStringLiteral("Latitude"));
        m_graph->axisY()->setTitle(QStringLiteral("Height"));
        m_graph->axisZ()->setTitle(QStringLiteral("Longitude"));

        m_graph->removeSeries(series);
        m_graph->addSeries(m_heightMapSeries);
        //! [4]

        // Reset range sliders for height map
        int mapGridCountX = m_heightMapWidth / heightMapGridStepX;
        int mapGridCountZ = m_heightMapHeight / heightMapGridStepZ;
        m_rangeMinX = 34.0f;
        m_rangeMinZ = 18.0f;
        m_stepX = 6.0f / float(mapGridCountX - 1);
        m_stepZ = 6.0f / float(mapGridCountZ - 1);
        m_axisMinSliderX->setMaximum(mapGridCountX - 2);
        m_axisMinSliderX->setValue(0);
        m_axisMaxSliderX->setMaximum(mapGridCountX - 1);
        m_axisMaxSliderX->setValue(mapGridCountX - 1);
        m_axisMinSliderZ->setMaximum(mapGridCountZ - 2);
        m_axisMinSliderZ->setValue(0);
        m_axisMaxSliderZ->setMaximum(mapGridCountZ - 1);
        m_axisMaxSliderZ->setValue(mapGridCountZ - 1);
    }
}

void SurfaceGraph::adjustXMin(int min)
{
    float minX = m_stepX * float(min) + m_rangeMinX;

    int max = m_axisMaxSliderX->value();
    if (min >= max) {
        max = min + 1;
        m_axisMaxSliderX->setValue(max);
    }
    float maxX = m_stepX * max + m_rangeMinX;

    setAxisXRange(minX, maxX);
}

void SurfaceGraph::adjustXMax(int max)
{
    float maxX = m_stepX * float(max) + m_rangeMinX;

    int min = m_axisMinSliderX->value();
    if (max <= min) {
        min = max - 1;
        m_axisMinSliderX->setValue(min);
    }
    float minX = m_stepX * min + m_rangeMinX;

    setAxisXRange(minX, maxX);
}

void SurfaceGraph::adjustZMin(int min)
{
    float minZ = m_stepZ * float(min) + m_rangeMinZ;

    int max = m_axisMaxSliderZ->value();
    if (min >= max) {
        max = min + 1;
        m_axisMaxSliderZ->setValue(max);
    }
    float maxZ = m_stepZ * max + m_rangeMinZ;

    setAxisZRange(minZ, maxZ);
}

void SurfaceGraph::adjustZMax(int max)
{
    float maxX = m_stepZ * float(max) + m_rangeMinZ;

    int min = m_axisMinSliderZ->value();
    if (max <= min) {
        min = max - 1;
        m_axisMinSliderZ->setValue(min);
    }
    float minX = m_stepZ * min + m_rangeMinZ;

    setAxisZRange(minX, maxX);
}


//============================================================================
void
SurfaceGraph::setGridState(SharedPtr<GridState> state)
{
    m_gridState = state;
	enableGrid(true);
}


//============================================================================
void SurfaceGraph::setAxisXRange(float min, float max)
{
    m_graph->axisX()->setRange(min, max);
}

void SurfaceGraph::setAxisZRange(float min, float max)
{
    m_graph->axisZ()->setRange(min, max);
}
//! [5]

//! [6]
void SurfaceGraph::changeTheme(int theme)
{
    m_graph->activeTheme()->setType(Q3DTheme::Theme(theme));
}
//! [6]

void SurfaceGraph::setBlackToYellowGradient()
{
    //! [7]
    QLinearGradient gr;
    gr.setColorAt(0.0, Qt::black);
    gr.setColorAt(0.33, Qt::blue);
    gr.setColorAt(0.67, Qt::red);
    gr.setColorAt(1.0, Qt::yellow);

    m_graph->seriesList().at(0)->setBaseGradient(gr);
    m_graph->seriesList().at(0)->setColorStyle(Q3DTheme::ColorStyleRangeGradient);
    //! [7]
}

void SurfaceGraph::setGreenToRedGradient()
{
    QLinearGradient gr;
    gr.setColorAt(0.0, Qt::darkGreen);
    gr.setColorAt(0.5, Qt::yellow);
    gr.setColorAt(0.8, Qt::red);
    gr.setColorAt(1.0, Qt::darkRed);

    m_graph->seriesList().at(0)->setBaseGradient(gr);
    m_graph->seriesList().at(0)->setColorStyle(Q3DTheme::ColorStyleRangeGradient);
}


//============================================================================
void
AtlasXOptimizer::setGridState(SharedPtr<GridState> state)
{
    m_gridState = state;
    modifier->setGridState(state);
}


//============================================================================
AtlasXOptimizer::AtlasXOptimizer(QWidget* parent)
{
    QWidget* widget = new QWidget(this);
    Q3DSurface* graph = new Q3DSurface();
    QWidget* container = QWidget::createWindowContainer(graph);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QHBoxLayout* hLayout = new QHBoxLayout(widget);
    QVBoxLayout* vLayout = new QVBoxLayout();
    hLayout->addWidget(container, 1);
    hLayout->addLayout(vLayout);
    vLayout->setAlignment(Qt::AlignTop);

    QGroupBox* modelGroupBox = new QGroupBox(QStringLiteral("Model"));

    QRadioButton* sqrtSinModelRB = new QRadioButton(widget);
    sqrtSinModelRB->setText(QStringLiteral("Sqrt && Sin"));
    sqrtSinModelRB->setChecked(false);

    QRadioButton* heightMapModelRB = new QRadioButton(widget);
    heightMapModelRB->setText(QStringLiteral("Height Map"));
    heightMapModelRB->setChecked(false);

    QRadioButton* reutrnsRB = new QRadioButton(widget);
    reutrnsRB->setText(QStringLiteral("Returns"));
    reutrnsRB->setChecked(false);

    QVBoxLayout* modelVBox = new QVBoxLayout;
    modelVBox->addWidget(sqrtSinModelRB);
    modelVBox->addWidget(heightMapModelRB);
    modelVBox->addWidget(reutrnsRB);
    modelGroupBox->setLayout(modelVBox);

    QGroupBox* selectionGroupBox = new QGroupBox(QStringLiteral("Selection Mode"));

    QRadioButton* modeNoneRB = new QRadioButton(widget);
    modeNoneRB->setText(QStringLiteral("No selection"));
    modeNoneRB->setChecked(false);

    QRadioButton* modeItemRB = new QRadioButton(widget);
    modeItemRB->setText(QStringLiteral("Item"));
    modeItemRB->setChecked(false);

    QRadioButton* modeSliceRowRB = new QRadioButton(widget);
    modeSliceRowRB->setText(QStringLiteral("Row Slice"));
    modeSliceRowRB->setChecked(false);

    QRadioButton* modeSliceColumnRB = new QRadioButton(widget);
    modeSliceColumnRB->setText(QStringLiteral("Column Slice"));
    modeSliceColumnRB->setChecked(false);

    QVBoxLayout* selectionVBox = new QVBoxLayout;
    selectionVBox->addWidget(modeNoneRB);
    selectionVBox->addWidget(modeItemRB);
    selectionVBox->addWidget(modeSliceRowRB);
    selectionVBox->addWidget(modeSliceColumnRB);
    selectionGroupBox->setLayout(selectionVBox);

    QSlider* axisMinSliderX = new QSlider(Qt::Horizontal, widget);
    axisMinSliderX->setMinimum(0);
    axisMinSliderX->setTickInterval(1);
    axisMinSliderX->setEnabled(true);
    QSlider* axisMaxSliderX = new QSlider(Qt::Horizontal, widget);
    axisMaxSliderX->setMinimum(1);
    axisMaxSliderX->setTickInterval(1);
    axisMaxSliderX->setEnabled(true);
    QSlider* axisMinSliderZ = new QSlider(Qt::Horizontal, widget);
    axisMinSliderZ->setMinimum(0);
    axisMinSliderZ->setTickInterval(1);
    axisMinSliderZ->setEnabled(true);
    QSlider* axisMaxSliderZ = new QSlider(Qt::Horizontal, widget);
    axisMaxSliderZ->setMinimum(1);
    axisMaxSliderZ->setTickInterval(1);
    axisMaxSliderZ->setEnabled(true);

    QComboBox* themeList = new QComboBox(widget);
    themeList->addItem(QStringLiteral("Qt"));
    themeList->addItem(QStringLiteral("Primary Colors"));
    themeList->addItem(QStringLiteral("Digia"));
    themeList->addItem(QStringLiteral("Stone Moss"));
    themeList->addItem(QStringLiteral("Army Blue"));
    themeList->addItem(QStringLiteral("Retro"));
    themeList->addItem(QStringLiteral("Ebony"));
    themeList->addItem(QStringLiteral("Isabelle"));

    QGroupBox* colorGroupBox = new QGroupBox(QStringLiteral("Custom gradient"));
    QLinearGradient grBtoY(0, 0, 1, 100);
    grBtoY.setColorAt(1.0, Qt::black);
    grBtoY.setColorAt(0.67, Qt::blue);
    grBtoY.setColorAt(0.33, Qt::red);
    grBtoY.setColorAt(0.0, Qt::yellow);
    QPixmap pm(24, 100);
    QPainter pmp(&pm);
    pmp.setBrush(QBrush(grBtoY));
    pmp.setPen(Qt::NoPen);
    pmp.drawRect(0, 0, 24, 100);
    QPushButton* gradientBtoYPB = new QPushButton(widget);
    gradientBtoYPB->setIcon(QIcon(pm));
    gradientBtoYPB->setIconSize(QSize(24, 100));

    QLinearGradient grGtoR(0, 0, 1, 100);
    grGtoR.setColorAt(1.0, Qt::darkGreen);
    grGtoR.setColorAt(0.5, Qt::yellow);
    grGtoR.setColorAt(0.2, Qt::red);
    grGtoR.setColorAt(0.0, Qt::darkRed);
    pmp.setBrush(QBrush(grGtoR));
    pmp.drawRect(0, 0, 24, 100);
    QPushButton* gradientGtoRPB = new QPushButton(widget);
    gradientGtoRPB->setIcon(QIcon(pm));
    gradientGtoRPB->setIconSize(QSize(24, 100));

    QHBoxLayout* colorHBox = new QHBoxLayout;
    colorHBox->addWidget(gradientBtoYPB);
    colorHBox->addWidget(gradientGtoRPB);
    colorGroupBox->setLayout(colorHBox);

    vLayout->addWidget(modelGroupBox);
    vLayout->addWidget(selectionGroupBox);
    vLayout->addWidget(new QLabel(QStringLiteral("Column range")));
    vLayout->addWidget(axisMinSliderX);
    vLayout->addWidget(axisMaxSliderX);
    vLayout->addWidget(new QLabel(QStringLiteral("Row range")));
    vLayout->addWidget(axisMinSliderZ);
    vLayout->addWidget(axisMaxSliderZ);
    vLayout->addWidget(new QLabel(QStringLiteral("Theme")));
    vLayout->addWidget(themeList);
    vLayout->addWidget(colorGroupBox);

    modifier = new SurfaceGraph(graph);
    QObject::connect(heightMapModelRB, &QRadioButton::toggled,
        modifier, &SurfaceGraph::enableHeightMapModel);
    QObject::connect(sqrtSinModelRB, &QRadioButton::toggled,
        modifier, &SurfaceGraph::enableSqrtSinModel);
    QObject::connect(reutrnsRB, &QRadioButton::toggled,
        modifier, &SurfaceGraph::enableGrid);
    QObject::connect(modeNoneRB, &QRadioButton::toggled,
        modifier, &SurfaceGraph::toggleModeNone);
    QObject::connect(modeItemRB, &QRadioButton::toggled,
        modifier, &SurfaceGraph::toggleModeItem);
    QObject::connect(modeSliceRowRB, &QRadioButton::toggled,
        modifier, &SurfaceGraph::toggleModeSliceRow);
    QObject::connect(modeSliceColumnRB, &QRadioButton::toggled,
        modifier, &SurfaceGraph::toggleModeSliceColumn);
    QObject::connect(axisMinSliderX, &QSlider::valueChanged,
        modifier, &SurfaceGraph::adjustXMin);
    QObject::connect(axisMaxSliderX, &QSlider::valueChanged,
        modifier, &SurfaceGraph::adjustXMax);
    QObject::connect(axisMinSliderZ, &QSlider::valueChanged,
        modifier, &SurfaceGraph::adjustZMin);
    QObject::connect(axisMaxSliderZ, &QSlider::valueChanged,
        modifier, &SurfaceGraph::adjustZMax);
    QObject::connect(themeList, SIGNAL(currentIndexChanged(int)),
        modifier, SLOT(changeTheme(int)));
    QObject::connect(gradientBtoYPB, &QPushButton::pressed,
        modifier, &SurfaceGraph::setBlackToYellowGradient);
    QObject::connect(gradientGtoRPB, &QPushButton::pressed,
        modifier, &SurfaceGraph::setGreenToRedGradient);

    modifier->setAxisMinSliderX(axisMinSliderX);
    modifier->setAxisMaxSliderX(axisMaxSliderX);
    modifier->setAxisMinSliderZ(axisMinSliderZ);
    modifier->setAxisMaxSliderZ(axisMaxSliderZ);

    sqrtSinModelRB->setChecked(true);
    modeItemRB->setChecked(true);
    themeList->setCurrentIndex(2);
    setCentralWidget(widget);
}

AtlasXOptimizer::~AtlasXOptimizer()
{
}

}