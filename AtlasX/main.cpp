#include "App.hpp"
#include <QtWidgets/QApplication>
#include <QFile>


using namespace AtlasX;

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  App w;
  w.resize(800, 600);
  w.show();
  return a.exec();
}
