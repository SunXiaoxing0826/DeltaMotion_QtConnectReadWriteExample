#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "qdmcpconnection.h"
#include "qdmcpwriterequest.h"
#include "qdmcpreadrequest.h"

#include <QMainWindow>

#include <QAbstractSocket>
#include <QTcpSocket>
#include <QDataStream>
#include <QFile>
#include <QMessageBox>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void tryToConnect();
    void onConnected();
    void onDisconnected();
    void onSocketErrorOccurred(QAbstractSocket::SocketError error);

    void sendWriteRequest();
    void sendReadRequest();

    void onWriteResponse(QDmcpWriteRequest *request, QDmcpConnection::ResponseCode responseCode);
    void onReadResponse(QDmcpReadRequest *request, QSharedPointer<QVector<QVariant>> values, QDmcpConnection::ResponseCode responseCode);

private:
    Ui::MainWindow *m_ui;
    QDmcpConnection* m_connection;
};
#endif // MAINWINDOW_H
