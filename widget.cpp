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
{
    ui->setupUi(this);
    this->setFixedSize(600, 400);//固定
    this->setWindowTitle("Remote Apps");
    setUpFileView();
    ui->fileListView->setRootIndex(fileModel->index("C:\\Test"));
    setAcceptDrops(true);
    server=new QTcpServer;
    socket1=new QTcpSocket;
    socket2=new QTcpSocket;
    server->listen(QHostAddress::AnyIPv4,4567);
    connect(server,&QTcpServer::newConnection,this,&Widget::NewConnectionHandler);
    QStringList usbDriveLetters = getUsbDriveLetters();
    QByteArray Usbs=QStringListToByteArray(usbDriveLetters);
    if(!usbDriveLetters.isEmpty()){
        for(const QString& driveletter:usbDriveLetters){
            shareUsbDrive(driveletter,"Shared_"+driveletter);
        }
    }
}

Widget::~Widget()
{
    delete ui;
}
void Widget::NewConnectionHandler()
{
    qDebug()<<"something connected";
    QTcpSocket *s=(QTcpSocket*)server->nextPendingConnection();//获取下一个等待连接的套接字
    connect(s,&QTcpSocket::readyRead,this,&Widget::Reader);
    qDebug()<<"reader activated";
    qDebug()<<s->peerAddress();
    socket1->connectToHost(s->peerAddress(),4567);
    qDebug()<<"socket connected";
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
                    QString target="C:/Test";
                    createFolder(target);
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
                sendFileToClient(shortcutPath);

            }
        }
    }
}
bool Widget::createFolder(const QString &filePath)
{
    QDir dir;
    if(!dir.exists(filePath))
    {
        if(dir.mkpath(filePath))
        {
            qDebug()<<"Folder created successfully at"<<filePath;
            return true;
        }else{
            qDebug()<<"Failed to create folder at"<<filePath;
            return false;
        }
    }else{
        return true;
    }
}
void Widget::sendFileToClient(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Could not open file for reading";
        return;
    }

    QByteArray fileData = file.readAll();
    socket1->write(fileData);//写入文件数据到套接字
    socket1->flush();//刷新套接字
    file.close();
    socket1->disconnectFromHost();//断开连接
    qDebug() << "File sent successfully";
}
void Widget::onConnected()
{
    QString filePath = QFileDialog::getOpenFileName(nullptr, "Select File to Send");//打开文件对话框选择文件
    if (filePath.isEmpty()) {
        qDebug() << "No file selected";
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Could not open file for reading";
        return;
    }

    QByteArray fileData = file.readAll();
    socket1->write(fileData);//写入文件数据到套接字
    socket1->flush();//刷新套接字
    file.close();
    socket1->disconnectFromHost();//断开连接
    qDebug() << "File sent successfully";
}

void Widget::Reader()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if(!clientSocket){
        return;
    }
    QByteArray fileData = clientSocket->readAll();
    QString filePath2 ="C:/executable";
    createFolder(filePath2);
    QString filePath =filePath2 +"/received_executable.exe";
    QFile file(filePath);
    if(!file.open(QIODevice::WriteOnly)){
        qDebug() << "Could not open file for writing";
        return;
    }
    file.write(fileData);
    file.close();
    qDebug()<<"File received and saved successfully";
    QProcess::startDetached(filePath);
}
void Widget::on_pushButton_clicked()
{
    onConnected();
}
void Widget::openFolderDialog() {
    QString folderPath = QFileDialog::getExistingDirectory(this, tr("Open Folder"), "",
    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!folderPath.isEmpty()) {
        fileListView->setRootIndex(fileModel->setRootPath(folderPath));
    }
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
QStringList Widget::getUsbDriveLetters()
{
    QString command = "wmic logicaldisk where drivetype=2 get deviceid";
    QProcess process;
    process.start(command);
    process.waitForFinished();
    QByteArray result=process.readAllStandardOutput();
    qDebug()<<result;
    QString Output=QString::fromLocal8Bit(result);
    qDebug()<<Output;
    QStringList drives = Output.split("\r\n",Qt::SkipEmptyParts);
    QStringList usbdriveletters;
    qDebug()<<"Removable drives:";
    for(const QString &drive:drives){
        qDebug()<<drive.trimmed();
        usbdriveletters.append(drive.trimmed());
    }
    return usbdriveletters;
}
void Widget::shareUsbDrive(const QString &driveLetter,const QString &shareName)
{
    QProcess process;
    QString command = QString("net share %1=%2 /GRANT:everyone,Full").arg(shareName).arg(driveLetter + "\\");
    process.start("cmd.exe", QStringList()<<"/c"<<command);
    process.waitForFinished();
    QString output = process.readAllStandardOutput();
    QString errorOutput = process.readAllStandardError();
    if(!output.isEmpty()){
        qDebug()<<"共享创建成功"<<output;
    }
    if(!errorOutput.isEmpty()){

    }
}

QByteArray Widget::QStringListToByteArray(const QStringList &list)
{
    QByteArray bytearray;
    for(const QString &str:list){
        bytearray.append(str.toUtf8());
    }
    return bytearray;
}

