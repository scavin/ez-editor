#include <QtGui/QApplication>
#include "mainwindow.h"
//#include <QtCore/QDebug>
#include <QTranslator>
#include <QLocale>

//#include <ctime>
//using namespace std;

int main(int argc, char *argv[])
{
//    time_t start, ends;
//    clock_t cstart, cends;
//    start = time(NULL);
//    cstart = clock();

    QApplication a(argc, argv);

    QString lang_country(QLocale::system().name());
    QTranslator qt_ts;
    if(qt_ts.load(QObject::tr(":/qt_%1").arg(lang_country)))
        a.installTranslator( &qt_ts );

    QTranslator app_ts;
    if(app_ts.load(QObject::tr(":/editor_%1").arg(lang_country)))
        a.installTranslator( &app_ts );

    MainWindow w;
    w.show();

    a.processEvents(QEventLoop::ExcludeUserInputEvents);

#ifdef _NOTICE
    QString filepath = "E:\\tmp.txt";////
    w.openFile(filepath);
#else
    QStringList args(a.arguments());
    args.removeFirst(); // remove name of executable
    if(args.size() > 0 && !args.first().isEmpty())
        w.openFile(args.first());
#endif //_NOTICE

//    ends = time(NULL);
//    cends = clock();
//    qDebug() << "time diff : " << difftime(ends,start) << " s";
//    qDebug() << "Clock diff : " << cends-cstart << " cpu time counting unit";
//    qDebug() << double(clock()-cstart)/CLOCKS_PER_SEC;

    return a.exec();
}
