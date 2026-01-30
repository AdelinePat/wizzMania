#include <QApplication>
#include <iostream>

#include "Headers/mainwindow.h"

int main(int argc, char** argv) {
  std::cout << "========================================\n";
  std::cout << " WizzMania Client\n";
  std::cout << "========================================\n";

  QApplication app(argc, argv);

  MainWindow window;
  window.show();

  return app.exec();
}
