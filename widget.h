#ifndef WIDGET_H
#define WIDGET_H
#include "QTcpSocket"
#include "QTcpServer"
#include <QWidget>
#include "QDragEnterEvent"
#include "QDropEvent"
#include "QFileDialog"
#include "QListView"
#include "QFileSystemModel"
#include <QDir>
#include<QCoreApplication>
#include<QProcess>
#include<QStringList>
#include<QString>
#include<QStorageInfo>
#include<windows.h>
#include<QtGlobal>
#include<QSettings>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<QRegularExpression>
#include<QtNetwork>
QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private:
    Ui::Widget *ui;
    QListView *fileListView;
    QFileSystemModel *fileModel;
    QFile *file;
protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void NewConnectionHandler();
    void setUpFileView();
    void sendFileToClient(const QString &filePath);
    void createFolder(const QString &filepath);
    QStringList getUsbDriveLetters();
    void shareUsbDrive(const QString &driveLetter,const QString &shareName);
    QTcpSocket *socket1;
    QTcpSocket *socket2;
    QTcpServer *server;
    bool isRemovableDrive(const QString &drivePath);
    void unShareUsbDrive(const QString &letter);
    void allow();
    void usbup(const QStringList &removable);
    QStringList getDrives();
    void getIP();
private slots:
    void on_openFileButton_clicked();
    void on_OpenDesktop_clicked();
};
#endif // WIDGET_H
