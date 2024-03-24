#pragma once
#include "AtlasXComponent.hpp"
#include <QApplication>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QTreeView>
#include <filesystem>

namespace fs = std::filesystem;

BEGIN_NAMESPACE_ATLASX

struct AtlasXTreeImpl;

//============================================================================
class AtlasXTree : public QTreeView {
  Q_OBJECT

protected:
  virtual void contextMenuEvent(QContextMenuEvent *event) override;
public:
  AtlasXTree(QWidget *parent) noexcept;
signals:
  void onContextMenuEvent(QContextMenuEvent const* event);

};

//============================================================================
class AtlasXFileBrowser : public AtlasXComponent {
  Q_OBJECT
private:
  std::unique_ptr<AtlasXTreeImpl> m_impl;

  void loadSpace() noexcept;
  void refreshTreeView() noexcept;
  void createFolder(const fs::path &parentPath) noexcept;
  void createFile(const fs::path &parentPath) noexcept;

private slots:
  void onHydraLoad() noexcept override;
  void showContextMenu(QContextMenuEvent const *event);
  void onNewSpace(String const &env_dir) noexcept override;
  void onDirectoryChanged(QString change) noexcept;

public:
  AtlasXFileBrowser(String id, QWidget *parent,
                    fs::path const &env_dir) noexcept;
  ~AtlasXFileBrowser() noexcept;

  static ComponentType compType() noexcept {
    return ComponentType::FILE_BROWSER;
  }
};

END_NAMESPACE_ATLASX