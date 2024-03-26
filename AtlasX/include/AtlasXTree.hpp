#pragma once
#include "AtlasXComponent.hpp"
#include <QApplication>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QTreeView>

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
  friend class App;

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
  void onEditorIdsRequest(Vector<String> const &ids) noexcept;

signals:
  void fileOpenRequest(String const& editor_id, fs::path const &file);

public:
  AtlasXFileBrowser(String id, QWidget *parent,
                    fs::path const &env_dir) noexcept;
  ~AtlasXFileBrowser() noexcept;

  static ComponentType compType() noexcept {
    return ComponentType::FILE_BROWSER;
  }
};

END_NAMESPACE_ATLASX