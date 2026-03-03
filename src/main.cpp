#include "app/Application.h"
#include "core/Logger.h"
#include "core/Settings.h"

#include <QMessageBox>
#include <QTranslator>
#include <QLocale>

using namespace qnote;

int main(int argc, char* argv[]) {
    // 创建应用实例
    Application app(argc, argv);
    
    // 初始化
    if (!app.initialize()) {
        return 1;
    }
    
    // 首次运行提示
    if (app.isFirstRun()) {
        LOG_INFO("First run detected");
        // TODO: 显示欢迎向导
        app.setFirstRunComplete();
    }
    
    // TODO: 创建主窗口（M04 模块）
    // MainWindow* window = new MainWindow();
    // window->show();
    
    LOG_INFO("Application ready");
    
    // 临时：显示提示（开发阶段）
    QMessageBox::information(
        nullptr,
        QApplication::tr("QNote"),
        QApplication::tr("QNote is running in development mode.\n\n"
                        "M00 (Project Base) is complete!\n\n"
                        "Next: M01 (Data Layer)")
    );
    
    return app.exec();
}
