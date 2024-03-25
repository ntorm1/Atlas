#pragma once
#include <QtWidgets/QWidget>

#include "AtlasXTypes.hpp"
#include "AppUtil.hpp"

BEGIN_NAMESPACE_ATLASX


//============================================================================
class AtlasXComponent : public QWidget {
  Q_OBJECT
  friend class App;

private:
  String m_id;
  ComponentType m_type;
  Vector<UniquePtr<AtlasXComponent>> m_children;
  
private slots:
  virtual void onHydraLoad() noexcept = 0;
  virtual void onNewSpace(String const& env_dir) noexcept = 0;

public:

  AtlasXComponent(String id, ComponentType type, QWidget *parent) noexcept
      : QWidget(parent), m_id(id), m_type(type) {}
  
  String const& id() const noexcept { return m_id; }
  ComponentType type() const noexcept { return m_type; }

};

END_NAMESPACE_ATLASX