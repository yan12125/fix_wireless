#include <array>
#include <iostream>
#include <cstring>
#include <clocale>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <usb.h>
#include <libusb.h>
#include <QDebug>
#include <QCoreApplication>
#include <QtSystemd/sdmanager.h>

// find the USB wlan adaptor 2001:3c1b
// http://www.jollen.org/blog/2008/01/libusb_hello_world.html
bool check_wlan_adaptor()
{
    usb_init();
    usb_find_busses();
    usb_find_devices();

    usb_bus* busses = usb_get_busses();

    for (usb_bus* bus = busses; bus; bus = bus->next)
    {
        for (struct usb_device* dev = bus->devices; dev; dev = dev->next)
        {
            usb_device_descriptor* desc = &(dev->descriptor);
            if ((desc->idVendor == 0x2001) && (desc->idProduct == 0x3c1b))
            {
                std::cout << "已偵測到無線網卡" << std::endl;
                return true;
            }
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
         Systemd::Job::Ptr job = Systemd::restartUnit(service, Systemd::Replace);
         if(!job)
         {
             std::cout << "重新啟動服務" << service.toStdString() << "失敗" << std::endl;
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
