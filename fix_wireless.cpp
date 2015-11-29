#include <array>
#include <iostream>
#include <cstring>
#include <clocale>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <unistd.h>
#include <libusb.h>
#include <QDebug>
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>

// find the USB wlan adaptor 2001:3c1b
// http://www.jollen.org/blog/2008/01/libusb_hello_world.html
bool check_wlan_adaptor()
{
    libusb_init(NULL);

    libusb_device **devs;
    libusb_get_device_list(NULL, &devs);

    libusb_device *dev;
    int i = 0;
    while ((dev = devs[i++]) != NULL) {
        libusb_device_descriptor desc;
        libusb_get_device_descriptor(dev, &desc);
        if ((desc.idVendor == 0x2001) && (desc.idProduct == 0x3c1b))
        {
            std::cout << "已偵測到無線網卡" << std::endl;
            return true;
        }
    }
    std::cout << "找不到無線網卡" << std::endl;
    return false;
}

// ifconfig up wlan0
// http://fred-zone.blogspot.tw/2008/01/netdevice.html
bool check_interface()
{
    bool retval = false;

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, "wlan0");

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    ioctl(sockfd, SIOCGIFFLAGS, &ifr);
    ifr.ifr_flags |= (IFF_UP | IFF_RUNNING);
    if(ioctl(sockfd, SIOCSIFFLAGS, &ifr) != -1)
    {
        retval = true;
        std::cout << "已啟動無線網卡" << std::endl;
    }
    else
    {
        std::string reason(strerror(errno));
        std::cout << "無線網卡啟動失敗。原因：" << reason << std::endl;
    }
    close(sockfd);

    return retval;
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
    case QtInfoMsg:
        std::cout << "Info: " << msg.toStdString() << std::endl;
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

bool restart_services()
{
    QString services[] { "network@wlan0.service", "named.service", "dhcpd4.service", "hostapd.service", "NAT.service" };
    for(QString service : services)
    {
        // References:
        // https://zignar.net/2014/09/08/getting-started-with-dbus-python-systemd/
        // http://www.freedesktop.org/wiki/Software/systemd/dbus/
        // https://techbase.kde.org/Development/Tutorials/D-Bus/Accessing_Interfaces
        // http://comments.gmane.org/gmane.comp.lib.qt.general/19484
        QDBusConnection bus = QDBusConnection::systemBus();
        QDBusInterface* interface = new QDBusInterface("org.freedesktop.systemd1", "/org/freedesktop/systemd1", "org.freedesktop.systemd1.Manager", bus);
        QDBusReply<QDBusObjectPath> reply = interface->call("RestartUnit", service, "replace");

        if(!reply.isValid())
        {
            std::cout << "重新啟動服務" << service.toStdString() << "失敗: "
                      << reply.error().message().toStdString() << std::endl;
            return false;
        }
        std::cout << "服務" << service.toStdString() << "啟動成功" << std::endl;
    }
    return true;
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    qInstallMessageHandler(myMessageOutput);

    setlocale(LC_ALL, "zh_TW.UTF-8");

    if(!check_wlan_adaptor())
    {
        return 1;
    }
    if(!check_interface())
    {
        return 1;
    }
    if(!restart_services())
    {
        return 1;
    }
    return 0;
}
