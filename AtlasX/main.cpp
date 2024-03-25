#include "App.hpp"
#include <QtWidgets/QApplication>
#include <QFile>


using namespace AtlasX;

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  a.setStyle("Fusion");
  App w;
  w.resize(1200, 800);
  w.show();
  return a.exec();
}
