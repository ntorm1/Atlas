#include "fstream"
#include <QAction>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFile>
#include <QFileDialog>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPushButton>

#include "App.hpp"
#include "AppUtil.hpp"

#include "AtlasXEditor.hpp"
#include "AtlasXTree.hpp"
#include "DockAreaWidget.h"
#include "DockManager.h"

namespace fs = std::filesystem;
using namespace ads;

BEGIN_NAMESPACE_ATLASX

//============================================================================
static QIcon svgIcon(const QString &File) {
  // This is a workaround, because in item views SVG icons are not
  // properly scaled and look blurry or pixelate
  QIcon SvgIcon(File);
  SvgIcon.addPixmap(SvgIcon.pixmap(92));
  return SvgIcon;
}

//============================================================================
struct AppImpl {
  String exe_path;
  fs::path env_path;
  ads::CDockManager *m_DockManager;
  Map<String, UniquePtr<AtlasXEditor>> editors;
  Map<String, UniquePtr<AtlasXComponent>> components;
  Map<String, CDockAreaWidget *> areas;

  fs::path envBase() const noexcept {
    auto parent_dir = fs::path(exe_path).parent_path();
    return parent_dir / "envs";
  }

  void initSpace(String env_name) noexcept {
    ATLASX_DEBUG("init space: {}", env_name);
    env_path = envBase() / env_name;
    if (!fs::exists(env_path)) {
      if (env_name == "default") {
        fs::create_directory(env_path);
      }
      ATLASX_CRITICAL("expected dir: {}", env_path.string());
      return;
    }
    static const char *hydra_toml = R"([hydra]
version = "0.11.0"
)";
    auto toml_path = env_path / "hydra.toml";
    if (!fs::exists(toml_path)) {
      try {
        std::ofstream toml_file(toml_path);
        toml_file << hydra_toml;
      } catch (std::exception &e) {
        ATLASX_CRITICAL("failed to write hydra.toml: {}", toml_path.string());
      }
    }
    ATLASX_DEBUG("loaded space {} from {}", env_name, env_path.string());
  }

  AppImpl() noexcept {
    exe_path = QCoreApplication::applicationDirPath().toStdString();
    env_path = envBase() / "default";
  }
};

//============================================================================
App::App(QWidget *parent) noexcept
    : QMainWindow(parent), ui(new Ui::AtlasXClass()) {
  ui->setupUi(this);
  m_impl = std::make_unique<AppImpl>();
  CDockManager::setConfigFlag(CDockManager::OpaqueSplitterResize, true);
  CDockManager::setConfigFlag(CDockManager::AllTabsHaveCloseButton, true);
  CDockManager::setConfigFlag(CDockManager::DragPreviewIsDynamic, true);
  CDockManager::setConfigFlag(CDockManager::DragPreviewHasWindowFrame, false);
  CDockManager::setConfigFlag(CDockManager::FocusHighlighting, true);
  m_impl->m_DockManager = new ads::CDockManager(this);

  initState();
  initStyle();
  initToolbar();
  initMenu();
  m_impl->initSpace("default");
  emit onNewSpace(m_impl->env_path.string());
}

//============================================================================
App::~App() noexcept { delete ui; }

//============================================================================
void App::initState() noexcept {
  ads::CDockWidget *dock = new ads::CDockWidget("AtlasX Space", this);
  auto tree = std::make_unique<AtlasXFileBrowser>(
      nextComponentId(AtlasXFileBrowser::compType()), dock, m_impl->envBase());
  dock->setWidget(tree.get());
  auto dock_area =
      m_impl->m_DockManager->addDockWidget(ads::LeftDockWidgetArea, dock);
  dock->setIcon(QIcon("./styles/icons/fileopen.png"));
  linkComponent(std::move(tree));

  auto editor = createEditor();
}

//============================================================================
void App::initStyle() noexcept {
  qDebug() << "INIT STYLE APPLY";
  QFile file("./styles/vs_light.css");
  if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream stream(&file);
    QString stylesheet = stream.readAll();
    file.close();
    m_impl->m_DockManager->setStyleSheet(stylesheet);
  } else {
    qCritical() << "Failed to open stylesheet file";
  }
  qDebug() << "INIT STYLE COMPLETE";
}

//============================================================================
void App::initToolbar() noexcept {
  auto tool_bar = new QToolBar(this);
  tool_bar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
  auto a = new QAction("Build", tool_bar);
  a->setProperty("Floating", true);
  a->setToolTip("Build hydra state");
  a->setIcon(svgIcon("./styles/icons/build.png"));
  connect(a, &QAction::triggered, this, &App::build);
  tool_bar->addAction(a);

  a = new QAction("Step", tool_bar);
  a->setProperty("Floating", true);
  a->setToolTip("Step simulation forward one step");
  a->setIcon(svgIcon("./styles/icons/next.png"));
  connect(a, &QAction::triggered, this, &App::step);
  tool_bar->addAction(a);

  a = new QAction("Run", tool_bar);
  a->setProperty("Floating", true);
  a->setToolTip("Run simulation");
  a->setIcon(svgIcon("./styles/icons/run.png"));
  connect(a, &QAction::triggered, this, &App::run);
  tool_bar->addAction(a);

  a = new QAction("Reset", tool_bar);
  a->setProperty("Floating", true);
  a->setToolTip("Reset simulation");
  a->setIcon(svgIcon("./styles/icons/reset.png"));
  connect(a, &QAction::triggered, this, &App::reset);
  tool_bar->addAction(a);

  addToolBar(Qt::LeftToolBarArea, tool_bar);
}

//============================================================================
void App::initMenu() noexcept {
  QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
  QAction *newAction = fileMenu->addAction("&New");
  QAction *openAction = fileMenu->addAction("&Save");
  QAction *exitAction = fileMenu->addAction("&Exit");

  connect(newAction, &QAction::triggered, this, &App::createNewSpace);
}

//============================================================================
void App::build() noexcept {}

//============================================================================
void App::step() noexcept {}

//============================================================================
void App::run() noexcept {}

//============================================================================
void App::reset() noexcept {}

//============================================================================
void App::createNewSpace() noexcept {
  QDialog dialog(this);
  dialog.setWindowTitle("New Space");
  QLabel *nameLabel = new QLabel("Name:");
  QLineEdit *nameEdit = new QLineEdit(&dialog);
  QFormLayout *formLayout = new QFormLayout(&dialog);
  QHBoxLayout *nameLayout = new QHBoxLayout(&dialog);
  nameLayout->addWidget(nameLabel);
  nameLayout->addWidget(nameEdit);
  formLayout->addRow(nameLayout);
  QDialogButtonBox *buttonBox = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
  formLayout->addWidget(buttonBox);

  // Connect the dialog buttons
  connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

  // Execute the dialog and handle user input
  if (dialog.exec() == QDialog::Accepted) {
    auto name = nameEdit->text();
    auto env_dir = m_impl->envBase() / name.toStdString();
    if (fs::exists(env_dir)) {
      QMessageBox::StandardButton confirmOverride = QMessageBox::question(
          this, "Confirm Override",
          QFORMAT("{} Exsits, do you want to replace?", env_dir.string()),
          QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
      if (confirmOverride == QMessageBox::No) {
        return;
      }
    }
    fs::create_directories(env_dir);
    m_impl->initSpace(name.toStdString());
    dialog.accept();
  }
}

//============================================================================
String App::nextComponentId(ComponentType comp_type) noexcept {
  if (comp_type == ComponentType::EDITOR) {
    return std::format("editor_{}", m_impl->editors.size());
  }
  int comp_count = 0;

  for (auto const &[id, comp] : m_impl->components) {
    if (comp->type() == comp_type) {
      comp_count++;
    }
  }
  return std::format("{}_{}", componentTypeToString(comp_type), comp_count);
}

//============================================================================
AtlasXEditor *App::createEditor() noexcept {
  auto editor_id = nextComponentId(ComponentType::EDITOR);
  auto dock = new ads::CDockWidget(QString::fromStdString(editor_id), this);
  auto editor = std::make_unique<AtlasXEditor>(dock);
  dock->setWidget(editor.get());
  auto dock_area =
      m_impl->m_DockManager->addDockWidget(ads::RightDockWidgetArea, dock);
  m_impl->areas[editor_id] = dock_area;
  dock->setIcon(QIcon("./styles/icons/editor.png"));
  linkEditor(editor.get());
  auto ptr = editor.get();
  m_impl->editors[editor_id] = std::move(editor);
  onEditorIdsRequest();
  return ptr;
}

//============================================================================
void App::onFileOpenRequest(String const &editor_id,
                            fs::path const &path) noexcept {
  ATLASX_DEBUG("file open request: {}", path.string());
  for (auto &[id, editor] : m_impl->editors) {
    if (id != editor_id) {
      continue;
    }
    auto qpath = QString::fromStdString(path.string());
    editor->loadFile(qpath);
  }
  ATLASX_CRITICAL("failed to find editor: {}", editor_id);
}

//============================================================================
void App::linkEditor(AtlasXEditor *editor) noexcept {}

//============================================================================
void App::linkFileBrowser(AtlasXFileBrowser *tree) noexcept {
  connect(tree, &AtlasXFileBrowser::fileOpenRequest, this,
          &App::onFileOpenRequest);
  connect(this, &App::onEditorIds, tree,
          &AtlasXFileBrowser::onEditorIdsRequest);
}

//============================================================================
void App::linkComponent(UniquePtr<AtlasXComponent> component) noexcept {
  connect(this, &App::onNewSpace, component.get(),
          &AtlasXComponent::onNewSpace);

  switch (component->type()) {
  case ComponentType::FILE_BROWSER:
    linkFileBrowser(static_cast<AtlasXFileBrowser *>(component.get()));
    break;
  default:
    ATLASX_CRITICAL("unknown component type: {}", component->id());
    break;
  }
  m_impl->components[component->id()] = std::move(component);
}

//============================================================================
void App::onEditorIdsRequest() noexcept {
  ATLASX_DEBUG("emitting new editor ids");
  Vector<String> editor_ids;
  for (auto const &[id, editor] : m_impl->editors) {
    editor_ids.push_back(id);
  }
  emit onEditorIds(std::move(editor_ids));
}

END_NAMESPACE_ATLASX