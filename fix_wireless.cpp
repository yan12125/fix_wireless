#include <iostream>
#include <QDebug>
#include <QCoreApplication>
#include <QtSystemd/sdmanager.h>

bool restart_services()
{
    QString services[] { "named.service", "dhcpd4.service" };
    for(QString service : services)
    {
         Systemd::Job::Ptr job = Systemd::restartUnit(service, Systemd::Replace);
         if(!job)
         {
             return false;
         }
    }
    return true;
}

/*
 * http://stackoverflow.com/questions/4954140/how-to-redirect-qdebug-qwarning-qcritical-etc-output
 * http://stackoverflow.com/questions/14643293/how-does-qt5-redirect-qdebug-statements-to-the-qt-creator-2-6-console
 */
void myMessageOutput(QtMsgType type, const QMessageLogContext& Context, const QString& msg)
{
    Q_UNUSED(Context)

    switch (type)
    {
    case QtDebugMsg:
        std::cout << "Debug: " << msg.toStdString() << std::endl;
        break;
    case QtWarningMsg:
        std::cout << "Warning: " << msg.toStdString() << std::endl;
        break;
    case QtCriticalMsg:
        std::cout << "Critical: " << msg.toStdString() << std::endl;
        break;
    case QtFatalMsg:
        std::cout << "Fatal: " << msg.toStdString() << std::endl;
        abort();
    }
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    qInstallMessageHandler(myMessageOutput);

    if(!restart_services())
    {
        return 1;
    }
    return 0;
}

