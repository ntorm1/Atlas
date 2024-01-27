#include <filesystem>

#include "../include/AtlasX.h"
#include "../include/AtlasXImpl.h"

#include <QLabel>
#include <QFile>
#include <QToolButton>
#include <qmessagebox.h>

#include "DockManager.h"
#include "DockComponentsFactory.h"
#include "DockAreaWidget.h"
#include "DockAreaTitleBar.h"

using namespace ads;

namespace fs = std::filesystem;

namespace AtlasX
{


//============================================================================
struct AtlasXAppState
{
    String exe_path;
    fs::path env_path;

    AtlasXAppState()
	{
		exe_path = QCoreApplication::applicationDirPath().toStdString();
	}
};


//============================================================================
static QIcon svgIcon(const QString& File)
{
    // This is a workaround, because in item views SVG icons are not
    // properly scaled and look blurry or pixelate
    QIcon SvgIcon(File);
    SvgIcon.addPixmap(SvgIcon.pixmap(92));
    return SvgIcon;
}


//============================================================================
class CCustomComponentsFactory : public ads::CDockComponentsFactory
{
public:
    using Super = ads::CDockComponentsFactory;
    ads::CDockAreaTitleBar* createDockAreaTitleBar(ads::CDockAreaWidget* DockArea) const override
    {
        auto TitleBar = new ads::CDockAreaTitleBar(DockArea);
        auto CustomButton = new QToolButton(DockArea);
        CustomButton->setToolTip(QObject::tr("Help"));
        //CustomButton->setIcon(svgIcon(":/adsdemo/images/help_outline.svg"));
        CustomButton->setAutoRaise(true);
        int Index = TitleBar->indexOf(TitleBar->button(ads::TitleBarButtonTabsMenu));
        TitleBar->insertWidget(Index + 1, CustomButton);
        return TitleBar;
    }
};


//============================================================================
AtlasXApp::AtlasXApp(QWidget* parent
) :
    QMainWindow(parent),
    ui(new Ui::AtlasXClass())
{
    ui->setupUi(this);
    
    m_state = new AtlasXAppState();
    m_impl = new AtlasXAppImpl();

    resize(1280, 720);
    setGeometry(QStyle::alignedRect(
        Qt::LeftToRight, Qt::AlignCenter, frameSize(),
        QGuiApplication::primaryScreen()->availableGeometry()
    ));

    // Create the dock manager after the ui is setup. Because the
    // parent parameter is a QMainWindow the dock manager registers
    // itself as the central widget as such the ui must be set up first.
    m_DockManager = new ads::CDockManager(this);

    //CDockManager::setConfigFlag(CDockManager::OpaqueSplitterResize, true);
    //sCDockManager::setConfigFlag(CDockManager::FocusHighlighting, true);
    //CDockManager::setConfigFlag(CDockManager::XmlCompressionEnabled, false);
    CDockManager::setConfigFlag(CDockManager::AllTabsHaveCloseButton, true);
    CDockManager::setConfigFlag(CDockManager::DragPreviewIsDynamic, true);
    //CDockManager::setConfigFlag(CDockManager::DragPreviewShowsContentPixmap, false);
    CDockManager::setConfigFlag(CDockManager::DragPreviewHasWindowFrame, false);
    //CDockManager::setAutoHideConfigFlags(CDockManager::DefaultAutoHideConfig);

    //ads::CDockComponentsFactory::setFactory(new CCustomComponentsFactory());

    // init exchange manager
    auto exchange_manager_dock = AtlasXExchangeManager::make(
        this,
        m_impl
    );
    m_impl->exchange_manager = static_cast<AtlasXExchangeManager*>(exchange_manager_dock->widget());
    exchange_manager_dock->setFeature(ads::CDockWidget::DockWidgetClosable, false);
    auto DockArea = m_DockManager->addDockWidget(
        ads::CenterDockWidgetArea,
        exchange_manager_dock
    );


    initStyle();
    initToolBar();
    initEnvironment();
    initSignals();
}


//============================================================================
void
AtlasXApp::initSignals() noexcept
{
    // connect hydraRestore signal to the exchange manager
    connect(this, &AtlasXApp::hydraRestore, m_impl->exchange_manager, &AtlasXExchangeManager::onHydraRestore);
}


//============================================================================
void
AtlasXApp::initStyle()
{
    qDebug() << "INIT STYLE APPLY";
    QFile file("./styles/vs_light.css");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream(&file);
        QString stylesheet = stream.readAll();
        file.close();

        // Apply the stylesheet to the main window
        m_DockManager->setStyleSheet(stylesheet);
    }
    else
    {
        assert(false);
    }
    qDebug() << "INIT STYLE COMPLETE";
}


//============================================================================
void
AtlasXApp::initToolBar() noexcept
{
    auto tool_bar = addToolBar("MainToolBar");
    tool_bar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    QAction* a = new QAction("Restore", tool_bar);
    a->setProperty("Floating", true);
    a->setToolTip("Restore hydra instance from env path");
    a->setIcon(svgIcon("./styles/icons/restore.png"));
    connect(a, &QAction::triggered, this, &AtlasXApp::initEnvironment);
    tool_bar->addAction(a);

    a = new QAction("Save", tool_bar);
    a->setToolTip("Save hydra instance to env path");
    a->setIcon(svgIcon("./styles/icons/save.png"));
    connect(a, &QAction::triggered, this, &AtlasXApp::saveEnvironment);
    tool_bar->addAction(a);
}


//============================================================================
void
AtlasXApp::saveEnvironment() noexcept
{
	qDebug() << "SAVE ENVIRONMENT";

	auto hydra_config_path = m_state->env_path / "hydra_config.json";
	auto res = m_impl->serialize(hydra_config_path.string());
    if (!res) {
        const auto& error = res.error();
        const QString errorMsg = QString::fromStdString(error.what());
        qCritical() << "FAILED TO SAVE HYDRA CONFIG TO: " << hydra_config_path.string().c_str() << " ERROR: " << errorMsg;
        QMessageBox::critical(this, "Error", "Failed to save hydra config: " + QString::fromStdString(hydra_config_path.string()) + "\nError: " + errorMsg);
        return;
    }
	qDebug() << "SAVE ENVIRONMENT COMPLETE";
}


//============================================================================
void
AtlasXApp::initEnvironment() noexcept
{
    String env_name = "default";
    qDebug() << "INIT ENVIRONMENT";

    fs::path exe_path(m_state->exe_path);

    // Check to see of env master folder exists yet
    auto env_parent_path = exe_path / "envs";
    if (!fs::exists(env_parent_path))
    {
        fs::create_directory(env_parent_path);
    }

    // Create folder for the actual env being created
    m_state->env_path = env_parent_path / env_name;
    if (!fs::exists(m_state->env_path))
    {
        fs::create_directory(m_state->env_path);
    }

    // load in the hydra config file if it exists
    auto hydra_config_path = m_state->env_path / "hydra_config.json";
    if (fs::exists(hydra_config_path))
	{
		qDebug() << "LOADING HYDRA CONFIG";
		auto res = m_impl->deserialize(hydra_config_path.string());
        if (!res)
        {
			qDebug() << "FAILED TO LOAD HYDRA CONFIG FROM: " << hydra_config_path.string().c_str();
            return;
        }
	}
    emit hydraRestore();
    qDebug() << "LOADING ENV: " + env_name + " COMPLETE";
}


//============================================================================


AtlasXApp::~AtlasXApp()
{
    delete ui;
    delete m_impl;
    delete m_state;
}

}