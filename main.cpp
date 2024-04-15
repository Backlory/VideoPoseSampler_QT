#include "main.h"

int main(int argc, char *argv[])
{
    //
    QApplication a(argc, argv);
    QCoreApplication::setApplicationName("VideoPoseSamplerQT");
    QCoreApplication::setApplicationVersion(QT_VERSION_STR);

    // -p代表端口，-a代表地址
    QCommandLineParser parser;
    parser.setApplicationDescription("VideoPoseSamplerQT");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOptions({
        {{"a", "address"}, QCoreApplication::translate("main", "Set the address"), QCoreApplication::translate("main", "address")},
        {{"p", "port"}, QCoreApplication::translate("main", "Set the port"), QCoreApplication::translate("main", "port")},
        {{"d", "delay"}, QCoreApplication::translate("main", "Set the delay of camera"), QCoreApplication::translate("main", "delay")},
        {{"s", "savedir"}, QCoreApplication::translate("main", "Set save path"), QCoreApplication::translate("main", "savedir")}
    });
    QCommandLineOption roiOption("roi", QCoreApplication::translate("main", "define a region of interest(x,y,w,h)."), QCoreApplication::translate("main", "x,y,h,w"));
    parser.addOption(roiOption);
    parser.process(a);
    // 输出地址和端口
    QString address = parser.value("address");
    int port = parser.value("port").toInt();
    int delay = parser.value("delay").toInt();
    QString savedir = parser.value("savedir");
    // ROI裁剪
    QStringList roiValues;
    if (parser.isSet(roiOption)) {
        roiValues = parser.values("roi");
    }


    qDebug() << "address:" << address << ", port:" << port;
    qDebug() << "delay:" << delay << ", savedir:" << savedir;

    //
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "VideoPoseSampler_QT_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    MainWindow w(nullptr, address, port, delay, savedir, roiValues);
    w.show();
    return a.exec();
}
