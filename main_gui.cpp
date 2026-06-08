#include "gui/GuiApp.h"
#include "include/Logger.h"

int main() {
    Logger::instance().setLogFile("logs/system.log");
    LOG_INFO("GUI 模式启动");
    GuiApp app;
    app.run();
    LOG_INFO("GUI 正常退出");
    return 0;
}
