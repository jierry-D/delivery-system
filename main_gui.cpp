#include <QApplication>
#include <QDir>
#include "gui/MainWindow.h"
#include "include/Logger.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // 切换到可执行文件所在目录，确保 data/ 相对路径正确
    QDir::setCurrent(QCoreApplication::applicationDirPath());

    Logger::instance().setLogFile("logs/system.log");
    LOG_INFO("Qt GUI 模式启动");

    MainWindow win;
    win.show();
    return app.exec();
}
