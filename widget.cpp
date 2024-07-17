#include "widget.h"
#include "ui_widget.h"
#include "QTcpServer"
#include "QProcess"
#include "QString"
#include "QTcpSocket"
#include <QFileSystemWatcher>
#include "QDragEnterEvent"
#include "QDropEvent"
#include "qmimedata.h"
#include "QDebug"
#include "QList"
#include "QFile"
#include "QFileDialog"
#include <QMessageBox>
#include "Shortcut.h"
#include "QStandardPaths"
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    ,fileModel(new QFileSystemModel(this))
    ,file(nullptr)
{
    ui->setupUi(this);
    this->setFixedSize(600, 400);//固定
    this->setWindowTitle("RemoteApp");
    setUpFileView();
    ui->fileListView->setRootIndex(fileModel->index("C:\\Test"));
    setAcceptDrops(true);
    getIP();
    allow();
    createFolder("C:\\Test");
    sharefolder("Test","C:\\Test");
    QStringList usbDriveLetters = getDrives();
    if(!usbDriveLetters.isEmpty()){
        for(const QString& driveletter:usbDriveLetters){
            qDebug()<<driveletter;
            if(!driveletter.isEmpty())
            {
            shareUsbDrive(driveletter,driveletter[0]);
        }
    }

}
    usbup(usbDriveLetters);
}
Widget::~Widget()
{
    for(const QString driveletter:getDrives())
    {
        unShareUsbDrive(driveletter[0]);
    }
    delete ui;
}

void Widget::allow()//打开共享权限
{
    QString Path="HKEY_LOCAL_MACHINE\\SOFTWARE\\Policies\\Microsoft\\Windows NT\\Terminal Services";
    QString Name="fAllowUnlistedRemotePrograms";
    QString TYpe="REG_DWORD";
    QString value="1";
    QStringList arg;
    arg<<"add"<<Path<<"/v"<<Name<<"/t"<<TYpe<<"/d"<<value<<"/f";
    QProcess process;
    process.start("reg",arg);
    process.waitForFinished();
}
void  Widget::getIP() //获取ip地址
{
    QString localHostName = QHostInfo::localHostName();
    QHostInfo info = QHostInfo::fromName(localHostName);
    qDebug()<<"IP Address："<< info.addresses();
    for(const QHostAddress &address:info.addresses())
    {
        if(address.protocol() == QAbstractSocket::IPv4Protocol)
            ui->IP->setText(address.toString());
    }
}
void Widget::dragEnterEvent(QDragEnterEvent *event)
{
    if(event->mimeData()->hasUrls())
    {
        event->acceptProposedAction();//接受拖放操作
    }
    else
    {
        event->ignore();
    }
}

void Widget::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls())
    {
        QList<QUrl> urlList = mimeData->urls();
        if (!urlList.isEmpty())
        {
            QString filename = urlList[0].toLocalFile();//获取拖放文件的路径
            qDebug() << filename << Qt::endl;

            if (!filename.isEmpty())
            {
                QFileInfo fileInfo(filename);
                QString extension = fileInfo.suffix().toLower();//文件扩展名
                QString baseName = fileInfo.baseName();  // 获取不带扩展名的文件名
                qDebug() << "Base name:" << baseName;
                QString shortcutPath;

                if (extension == "lnk")
                {
                    // 处理 .lnk 文件的逻辑
                    qDebug() << "It's a .lnk file.";
                    // 例如：复制到目标位置
                    QFile file(filename);
                    shortcutPath="C:/Test/"+baseName + ".lnk";
                    if (file.copy(shortcutPath))
                    {
                        qDebug() << "Shortcut copied successfully.";
                    }
                    else
                    {
                        qDebug() << "Failed to copy the shortcut.";
                    }
                }
                else if (extension == "exe")
                {
                    // 处理 .exe 文件的逻辑
                    qDebug() << "It's an .exe file.";
                    // 例如：执行文件或其他处理
                    // 注意：为了安全起见，直接执行文件时请确保路径和文件内容是可信的
                    QString filePath=filename;
                    QString destination = "C:/Test/" + fileInfo.fileName();
                    QString shortcutDirectory = "C:/Test";
                    shortcutPath = shortcutDirectory + "/" + QFileInfo(filePath).baseName() + ".lnk";
                    if (createWindowsShortcut(filePath, shortcutPath)) {
                        QMessageBox::information(this, tr("Shortcut Created"), tr("Shortcut created in C:/Test."));//显示快捷方式创建成功
                    } else {
                        QMessageBox::warning(this, tr("Shortcut Creation Failed"), tr("Failed to create the shortcut."));
                    }
                }
                else
                {
                    QMessageBox::warning(this,"Warning","Unsupported file ");
                }
            }
        }
    }
}
void Widget::createFolder(const QString &filepath)//创建文件夹
{
    QProcess process;
    QString command ="mkdir " +filepath;
    qDebug()<<command;
    process.start("cmd", QStringList() << "/c" << command);
    process.waitForFinished();
}


void Widget::setUpFileView()
{
    fileModel->setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);
    QString initialPath = "C:\\Test"; // Initial folder path
    fileModel->setRootPath(initialPath); // Setting the initial root path
    ui->fileListView->setModel(fileModel);
    ui->fileListView->setViewMode(QListView::IconMode); // Set view mode to display icons
    ui->fileListView->setIconSize(QSize(64, 64)); // Set icon size
    ui->fileListView->setFlow(QListView::LeftToRight); // Arrange items from left to right
    ui->fileListView->setWrapping(true); // Enable wrapping
    ui->fileListView->setResizeMode(QListView::Adjust); // Automatically adjust item size
    ui->fileListView->setGridSize(QSize(100, 100)); // Set grid size for items

    // Disable dragging
    ui->fileListView->setDragEnabled(false); // Disable drag from the view
    ui->fileListView->setAcceptDrops(false); // Disable accepting drops in the view
    ui->fileListView->setDropIndicatorShown(false); // Disable drop indicator
}
void Widget::on_openFileButton_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Select a File"), QDir::homePath());
    QString path=filePath;
    QFileInfo fileInfo(path);
    if(fileInfo.suffix().toLower()=="exe"||fileInfo.suffix().toLower()=="lnk")
    {        if (!path.isEmpty()) {
            QString shortcutDirectory = "C:/Test";
            QDir dir(shortcutDirectory);
            if (!dir.exists()) {
                dir.mkpath(shortcutDirectory);
            }

            QString shortcutPath = shortcutDirectory + "/" + QFileInfo(filePath).baseName() + ".lnk";

            if (createWindowsShortcut(path, shortcutPath)) {

                QMessageBox::information(this, tr("Shortcut Created"), tr("Shortcut created in C:/Test."));
            } else {
                QMessageBox::warning(this, tr("Shortcut Creation Failed"), tr("Failed to create the shortcut."));
            }
        }

    }
    else
    {
        QMessageBox::warning(this,"Warning","Unsupported file ");
    }
}


void Widget::on_OpenDesktop_clicked()
{
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QStringList list;
    desktopPath.replace("/","\\");
    list.append(desktopPath);
    QProcess::execute("explorer.exe",list);
    qDebug() << "Desktop path:" << desktopPath;
}
QStringList Widget::getDrives()//获取所有盘符
{
    QList<QStorageInfo> storageList = QStorageInfo::mountedVolumes();
    QStringList removable;
    QString temp;
    foreach (const QStorageInfo &storage,storageList ) {
        temp=storage.rootPath().left(2);
        removable.append(temp);
    }
    return removable;
}
void Widget::shareUsbDrive(const QString &driveLetter,const QString &shareName)//打开共享
{
    QProcess process;
    QString command = QString("net share %1=%2 /grant:everyone,Full").arg(shareName).arg(driveLetter);
    process.start("cmd", QStringList() << "/c" << command);
    process.waitForFinished();
}

void Widget::unShareUsbDrive(const QString &letter)//关闭磁盘共享
{
    QProcess process;
    QString command="net share "+letter+" /delete";
    process.start("cmd", QStringList() << "/c" << command);
    process.waitForFinished();
}

void Widget::usbup(const QStringList &removable)//将盘符上传
{
    createFolder("C:\\Test\\config");
    QFile file("C:\\Test\\config\\output.txt");
    if(file.open(QIODevice::WriteOnly | QIODevice::Text)){
        QTextStream out(&file);
        for(const QString &str:removable){
            out<<str<<"\n";
        }
        file.close();
    }
}

void Widget::on_management_clicked()
{
    QProcess process;
    QString command="appwiz.cpl";
    process.start("cmd",QStringList()<<"/c"<<command);
    process.waitForFinished();
}

void Widget::sharefolder(const QString &sharename,const QString &folderpath)
{
    QProcess process;
    QString command=QString("net share %1=%2 /grant:everyone,Full").arg(sharename).arg(folderpath);
    process.start("cmd", QStringList() << "/c" << command);
    process.waitForFinished();
}


