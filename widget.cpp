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
    this->setFixedSize(600, 400);
    this->setWindowTitle("Remote Apps");
    setUpFileView();
    ui->fileListView->setRootIndex(fileModel->index("C:\\Test"));
    setAcceptDrops(true);
    server=new QTcpServer;
    socket=new QTcpSocket;
    server->listen(QHostAddress::AnyIPv4,4567);
    connect(server,&QTcpServer::newConnection,this,&Widget::NewConnectionHandler);
}

Widget::~Widget()
{
    delete ui;
}
void Widget::NewConnectionHandler()
{
    qDebug()<<"something connected";
    QTcpSocket *s=(QTcpSocket*)server->nextPendingConnection();
    connect(s,&QTcpSocket::readyRead,this,&Widget::Reader);
    qDebug()<<"reader activated";
    qDebug()<<s->peerAddress();
    socket->connectToHost(s->peerAddress(),4567);
    qDebug()<<"socket connected";
}


void Widget::dragEnterEvent(QDragEnterEvent *event)
{
    if(event->mimeData()->hasUrls())
    {
        event->acceptProposedAction();
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
            QString filename = urlList[0].toLocalFile();
            qDebug() << filename << Qt::endl;

            if (!filename.isEmpty())
            {
                QFileInfo fileInfo(filename);
                QString extension = fileInfo.suffix().toLower();
                QString baseName = fileInfo.baseName();  // 获取不带扩展名的文件名
                qDebug() << "Base name:" << baseName;

                if (extension == "lnk")
                {
                    // 处理 .lnk 文件的逻辑
                    qDebug() << "It's a .lnk file.";
                    // 例如：复制到目标位置
                    QFile file(filename);
                    if (file.copy("C:/Test/" + baseName + ".lnk"))
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
                    QString shortcutPath = shortcutDirectory + "/" + QFileInfo(filePath).baseName() + ".lnk";
                    if (createWindowsShortcut(filePath, shortcutPath)) {
                        QMessageBox::information(this, tr("Shortcut Created"), tr("Shortcut created in C:/Test."));
                    } else {
                        QMessageBox::warning(this, tr("Shortcut Creation Failed"), tr("Failed to create the shortcut."));
                    }
                }
                else
                {
                    QMessageBox::warning(this,"Warning","Unsupported file ");
                }

                // 发送文件路径到 socket
                /*QByteArray qba = filename.toUtf8();
                socket->write(qba);
                socket->flush();
                ui->lineEdit->setText("sent " + qba);*/
            }
        }
    }
}

void Widget::onConnected()
{
    QString filePath = QFileDialog::getOpenFileName(nullptr, "Select File to Send");
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
    socket->write(fileData);
    socket->flush();
    file.close();
    socket->disconnectFromHost();
    qDebug() << "File sent successfully";
}

void Widget::Reader()
{

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

