#ifndef QDMCPCONNECTION_H
#define QDMCPCONNECTION_H

#include <QObject>

#include <QAbstractSocket>
#include <QTcpSocket>
#include <QDataStream>

#include "qdmcpwriterequest.h"
#include "qdmcpreadrequest.h"

class QDmcpConnection : public QObject
{
    Q_OBJECT
public:
    explicit QDmcpConnection(QObject *parent = nullptr);

    void connectToRMC(QString hostName, quint16 port);
    void disconnectFromRMC();

    void sendRequest(QDmcpRequest& request);

    QString socketErrorString() { return m_socket->errorString(); }
    QTcpSocket::SocketState state() { return m_socket->state(); }

    enum ResponseCode {
        Success = 0x00,
        Malformed = 0x01,
        TooLong = 0x02,
        InvalidAddress = 0x03
    };

private:
    QTcpSocket *m_socket;
    QDataStream m_dataStream;
    QList<QSharedDataPointer<QDmcpRequestData>> m_requests;

    quint16 m_currentTransactionID = 0;

private slots:
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError error);
    void onDataReceived();

signals:
    void connected();
    void disconnected();
    void socketErrorOccurred(QAbstractSocket::SocketError error);
    void readResponse(QDmcpReadRequest *request, QSharedPointer<QVector<QVariant>> values, QDmcpConnection::ResponseCode responseCode);
    void writeResponse(QDmcpWriteRequest *request, QDmcpConnection::ResponseCode responseCode);
};

#endif // QDMCPCONNECTION_H
