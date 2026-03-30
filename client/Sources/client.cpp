#include <QApplication>
#include <QCoreApplication>
#include <QFile>
#include <iostream>

#include "mainwindow.hpp"
#include "utils/message_qt_types.hpp"

int main(int argc, char** argv) {
  std::cout << "========================================\n";
  std::cout << " WizzMania Client\n";
  std::cout << "========================================\n";

  QApplication app(argc, argv);

  // Load centralized stylesheet if available (client/resources/app.qss)
  // const QString qssPath =
  //     QCoreApplication::applicationDirPath() + "/../resources/app.qss";
  // QFile qssFile(qssPath);

  QFile qssFile(":/app.qss");
  if (qssFile.open(QFile::ReadOnly | QFile::Text)) {
    qApp->setStyleSheet(qssFile.readAll());
  }

  qRegisterMetaType<AuthMessages::WSAuthResponse>();
  qRegisterMetaType<ServerSend::InitialDataResponse>();
  qRegisterMetaType<ServerSend::SendMessageResponse>();
  qRegisterMetaType<ServerSend::ErrorResponse>();

  MainWindow window;
  window.show();

  return app.exec();
}
