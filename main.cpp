#include <QGuiApplication>
#include "AppController.hpp"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    AppController appc();
    Q_UNUSED(appc);

    return app.exec();
}
