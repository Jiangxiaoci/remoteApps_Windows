#include "widget.h"
#include <QFileSystemWatcher>
#include <QApplication>
#include "QDragEnterEvent"
#include "QDropEvent"
QStringList getUsbDriveLetters();
void shareUsbDrive(const QString &driveLetter,const QString &shareName);
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    w.show();
    return a.exec();
}

