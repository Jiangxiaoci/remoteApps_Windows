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
protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void NewConnectionHandler();
    void onConnected();
    void Reader();
    void openFolderDialog();
    void setUpFileView();
    QTcpSocket *socket;
    QTcpServer *server;

private slots:
    void on_pushButton_clicked();
    void on_openFileButton_clicked();
    void on_OpenDesktop_clicked();
};
#endif // WIDGET_H
