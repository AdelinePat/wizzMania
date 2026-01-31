// #include <iostream>

// int main()
// {
//     std::cout << "========================================\n";
//     std::cout << " WizzMania Client - Test Bootstrap\n";
//     std::cout << "========================================\n";

//     std::cout << "[INFO] Client executable started successfully\n";
//     std::cout << "[INFO] Toolchain, linker, and runtime OK\n";

//     return 0;
// }


#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QPalette>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Main window
    QWidget window;
    window.setWindowTitle("Mini Qt App");
    window.resize(400, 200);

    // Set background color to blue
    QPalette palette = window.palette();
    palette.setColor(QPalette::Window, Qt::blue);
    window.setAutoFillBackground(true);
    window.setPalette(palette);

    // Create a label
    QLabel *label = new QLabel("Welcome", &window);
    label->setStyleSheet("QLabel { color : white; font-size: 24px; }");
    label->setAlignment(Qt::AlignCenter);

    // Layout
    QVBoxLayout *layout = new QVBoxLayout(&window);
    layout->addWidget(label);
    window.setLayout(layout);

    window.show();

    return app.exec();
}

