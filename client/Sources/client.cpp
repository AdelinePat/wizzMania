#include <QApplication>
#include <iostream>

#include "mainwindow.h"
#include "message_qt_types.h"

int main(int argc, char** argv) {
  std::cout << "========================================\n";
  std::cout << " WizzMania Client\n";
  std::cout << "========================================\n";

  QApplication app(argc, argv);

  qRegisterMetaType<AuthMessages::WSAuthResponse>();
  qRegisterMetaType<ServerSend::InitialDataResponse>();
  qRegisterMetaType<ServerSend::SendMessageResponse>();
  qRegisterMetaType<ServerSend::ErrorResponse>();

  MainWindow window;
  window.show();

  return app.exec();
}
