#include "AtlasXTree.hpp"
#include <QFileSystemModel>
#include <QFileSystemWatcher>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QVBoxLayout>
#include <fstream>

BEGIN_NAMESPACE_ATLASX

//============================================================================
AtlasXTree::AtlasXTree(QWidget *parent) noexcept : QTreeView(parent) {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

//============================================================================
void AtlasXTree::contextMenuEvent(QContextMenuEvent *event) {
  emit onContextMenuEvent(event);
}

//============================================================================
struct AtlasXTreeImpl {
  fs::path env_dir;
  QFileSystemWatcher watcher;
  UniquePtr<QVBoxLayout> layout;
  UniquePtr<AtlasXTree> tree_view;
  Vector<String> editor_ids;

  AtlasXTreeImpl(fs::path const &_env_dir) noexcept : env_dir(_env_dir) {
    watcher.addPath(QFORMAT("{}", env_dir.string()));
  };
};

//============================================================================
AtlasXFileBrowser::AtlasXFileBrowser(String id, QWidget *parent,
                                     fs::path const &env_dir) noexcept
    : AtlasXComponent(id, ComponentType::FILE_BROWSER, parent) {
  m_impl = std::make_unique<AtlasXTreeImpl>(env_dir);
  m_impl->layout = std::make_unique<QVBoxLayout>(this);
  m_impl->tree_view = std::make_unique<AtlasXTree>(this);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  connect(&m_impl->watcher, SIGNAL(directoryChanged(QString)), this,
          SLOT(onDirectoryChanged(QString)));
}

//============================================================================
AtlasXFileBrowser::~AtlasXFileBrowser() noexcept {}

//============================================================================
void AtlasXFileBrowser::onDirectoryChanged(QString change) noexcept {
  ATLASX_DEBUG("file changed: {}", change.toStdString());
}

//============================================================================
void AtlasXFileBrowser::onHydraLoad() noexcept {}

//============================================================================
void AtlasXFileBrowser::loadSpace() noexcept {
  auto newLayout = std::make_unique<QVBoxLayout>(this);
  auto model = new QFileSystemModel(this);
  model->setRootPath(QFORMAT("{}", m_impl->env_dir.string()));
  model->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
  model->setReadOnly(false);

  m_impl->tree_view = std::make_unique<AtlasXTree>(this);
  m_impl->tree_view->setModel(model);
  m_impl->tree_view->setRootIndex(
      model->index(QFORMAT("{}", m_impl->env_dir.string())));
  m_impl->tree_view->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_impl->tree_view->setSelectionMode(QAbstractItemView::ExtendedSelection);
  m_impl->tree_view->setEditTriggers(QAbstractItemView::SelectedClicked |
                                     QAbstractItemView::DoubleClicked);

  m_impl->tree_view->setColumnWidth(0, 200);
  newLayout->addWidget(m_impl->tree_view.get());
  m_impl->layout = std::move(newLayout);
  setLayout(m_impl->layout.get());
  connect(m_impl->tree_view.get(), &AtlasXTree::onContextMenuEvent, this,
          &AtlasXFileBrowser::showContextMenu);
}

//============================================================================
void AtlasXFileBrowser::onNewSpace(String const &env_dir) noexcept {
  ATLASX_DEBUG("AtlasXTree::onNewSpace, id: {}, env_dir: {}", id(), env_dir);
  m_impl->env_dir = fs::path(env_dir);
  loadSpace();
  ATLASX_DEBUG("AtlasXTree::onNewSpace, id: {}, env_dir: {} done", id(),
               env_dir);
}

//============================================================================
void AtlasXFileBrowser::onEditorIdsRequest(
    std::vector<String> const &ids) noexcept {
  m_impl->editor_ids = ids;
}

//============================================================================
void AtlasXFileBrowser::showContextMenu(QContextMenuEvent const *event) {
  QPoint pos = event->pos();
  QPoint globalPos = m_impl->tree_view->viewport()->mapToGlobal(pos);

  // Get the index where the context menu occurred
  QModelIndex index = m_impl->tree_view->indexAt(pos);
  fs::path file_path;
  if (index.isValid()) {
    // Get the item from the model using the index
    QFileSystemModel *model =
        dynamic_cast<QFileSystemModel *>(m_impl->tree_view->model());
    QFileInfo fileInfo = model->fileInfo(index);
    file_path = fileInfo.filePath().toStdString();
  } else {
    file_path = m_impl->env_dir;
  }

  ATLASX_DEBUG("AtlasXTree::showContextMenu, file: {}", file_path.string());

  QMenu contextMenu;
  QAction *addFolderAction = contextMenu.addAction("Add New Folder");
  QAction *addFileAction = contextMenu.addAction("Add New File");

  if (!fs::is_directory(file_path)) {
    auto separator = new QAction(&contextMenu);
    separator->setSeparator(true);
    contextMenu.addAction(separator);

    QAction *openAction = contextMenu.addAction("Open");

    // Create a submenu for the "Open" action
    QMenu *openSubMenu = new QMenu("Open With", &contextMenu);
    openAction->setMenu(openSubMenu);

    // Assuming 'ids' is your vector of string IDs
    for (const auto &id : m_impl->editor_ids) {
      QAction *openWithAction = openSubMenu->addAction(id.c_str());
      connect(openWithAction, &QAction::triggered, this,
              [this, id, file_path]() {
                // Emit the fileOpenRequest signal with the chosen ID and file
                // path
                emit fileOpenRequest(id, file_path);
              });
    }
    QAction *deleteAction = contextMenu.addAction("Delete");
    connect(openAction, &QAction::triggered, this, [=]() {
      if (QMessageBox::question(this, "Delete", "Are you sure?") ==
          QMessageBox::Yes) {
        if (fs::remove(file_path)) {
        } else {
          QMessageBox::critical(this, "Error", "Failed to delete file");
        }
      }
    });
  }

  // Connect actions to slots
  connect(addFolderAction, &QAction::triggered, this,
          [=]() { createFolder(file_path); });
  connect(addFileAction, &QAction::triggered, this,
          [=]() { createFile(file_path); });

  QAction *selectedAction = contextMenu.exec(globalPos);
  if (selectedAction == addFolderAction) {
    if (!fs::is_directory(file_path)) {
      QMessageBox::critical(this, "Error", "Selected item is not a directory");
    }
  }
}

//============================================================================
void AtlasXFileBrowser::createFolder(const fs::path &parentPath) noexcept {
  QString folderName =
      QInputDialog::getText(this, tr("New Folder"), tr("Folder Name:"));
  if (!folderName.isEmpty()) {
    fs::path newFolderPath = parentPath / folderName.toStdString();
    if (!fs::exists(newFolderPath)) {
      if (fs::create_directory(newFolderPath)) {
      } else {
        QMessageBox::critical(this, "Error", "Failed to create folder");
      }
    } else {
      QMessageBox::critical(this, "Error", "Folder already exists");
    }
  }
}

//============================================================================
void AtlasXFileBrowser::createFile(const fs::path &parentPath) noexcept {
  QString fileName =
      QInputDialog::getText(this, tr("New File"), tr("File Name:"));
  if (!fileName.isEmpty()) {
    fs::path newFilePath = parentPath / fileName.toStdString();
    if (!fs::exists(newFilePath)) {
      std::ofstream newFile(newFilePath);
      if (newFile.is_open()) {
        newFile.close();
      } else {
        QMessageBox::critical(this, "Error", "Failed to create file");
      }
    } else {
      QMessageBox::critical(this, "Error", "File already exists");
    }
  }
}

END_NAMESPACE_ATLASX
