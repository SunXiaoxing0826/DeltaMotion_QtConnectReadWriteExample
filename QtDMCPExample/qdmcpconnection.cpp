#include "qdmcpconnection.h"
#include <QFile>

QDmcpConnection::QDmcpConnection(QObject *parent) : QObject(parent), m_socket(new QTcpSocket(this))
{
    // Set up data stream
    m_dataStream.setDevice(m_socket);
    m_dataStream.setByteOrder(QDataStream::ByteOrder::LittleEndian);
    m_dataStream.setFloatingPointPrecision(QDataStream::FloatingPointPrecision::SinglePrecision);

    // Set up socket signals
    connect(m_socket, &QTcpSocket::connected, this, &QDmcpConnection::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &QDmcpConnection::onDisconnected);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &QDmcpConnection::onError);
    connect(m_socket, &QTcpSocket::readyRead, this, &QDmcpConnection::onDataReceived);
}

void QDmcpConnection::connectToRMC(QString hostName, quint16 port)
{
    /// <summary>
    /// Run when the user attempts to connect to the RMC. Errors connecting are reported via
    /// the `socketErrorOccurred` signal.
    /// </summary>
    /// <param name="hostName">The hostname (such as IP address) of the RMC to connect to.</param>
    /// <param name="port">The port (usually 1324) of the RMC to connect to.</param>
    m_socket->connectToHost(hostName, port, QIODevice::ReadWrite, QAbstractSocket::IPv4Protocol);
}

void QDmcpConnection::disconnectFromRMC()
{
    /// <summary>
    /// Begins the process of disconnecting the user from the RMC. Once the disconnection is complete,
    /// the `disconnected` signal will be fired.
    /// </summary>
    m_socket->disconnectFromHost();
}

void QDmcpConnection::sendRequest(QDmcpRequest& request)
{
    /// <summary>
    /// Sends a read or write request to the RMC. The response to a read request will be sent to the
    /// `readResponse` signal, and the response to a write request will be sent to the `writeResponse` signal.
    /// </summary>
    /// <param name="request">The request to send to the RMC.</param>
    request.m_data->m_transactionID = m_currentTransactionID++;
    m_requests.append(request.m_data);
    request.write(m_dataStream);
}

void QDmcpConnection::onConnected()
{
    /// <summary>
    /// Run when the socket connection with the RMC is established.
    /// </summary>
    emit connected();
}

void QDmcpConnection::onDisconnected()
{
    /// <summary>
    /// Run when the socket connection with the RMC is closed.
    /// </summary>
    emit disconnected();
}

void QDmcpConnection::onError(QAbstractSocket::SocketError error)
{
    /// <summary>
    /// Run when an error occurs from the socket.
    /// </summary>
    emit socketErrorOccurred(error);
}

void QDmcpConnection::onDataReceived()
{
    /// <summary>
    /// Run when data is received from the socket.
    /// This function determines which request the response is for, and emits a signal for the
    /// response of the corresponding type.
    /// </summary>

    quint16 packetLength, staticValues, transactionID;
    quint8 functionCode, responseCode;

    // Read in raw data from the data stream to the header variables.
    m_dataStream >> packetLength;
    m_dataStream >> staticValues;
    m_dataStream >> transactionID;
    m_dataStream >> functionCode;
    m_dataStream >> responseCode;

    // Locate the QDmcpRequest item in the list of active requests
    for (int i = 0; i < m_requests.count(); i++) {
        QSharedDataPointer<QDmcpRequestData> thisRequestData = m_requests.at(i);

        if (thisRequestData->m_transactionID == transactionID) {
            // Remove this request from the list of active requests.
            m_requests.removeAt(i);

            // If this was a write request, emit the writeResponse signal
            // with the original write request object as well as the response code.
            if (functionCode == 0x95 || functionCode == 0x55) {
                QSharedPointer<QDmcpWriteRequest> newRequest(new QDmcpWriteRequest);
                newRequest->setData(thisRequestData);
                emit writeResponse(newRequest.data(), static_cast<QDmcpConnection::ResponseCode>(responseCode));
            }

            // If this was not a write request, it must have been a read request.
            else {
                // Pull as many integers from the data stream as the header told us there were,
                // and add them to a new QVector so the user can easily access them.
                int numberOfValues = (packetLength - 6) / sizeof(qint32);
                QSharedPointer<QVector<QVariant>> values = QSharedPointer<QVector<QVariant>>(new QVector<QVariant>());
                qint32 intValue;
                float floatValue;
                for (int i = 0; i < numberOfValues; i++) {
                    // Check what this value is supposed to be
                    if (thisRequestData->m_readTypes->at(i) == QMetaType::Float) {
                        m_dataStream >> floatValue;
                        values->append(QVariant(floatValue));
                    } else {
                        m_dataStream >> intValue;
                        values->append(intValue);
                    }
                }

                // Emit the readResponse signal with the original read request object, the QVector of values
                // the request retrieved, and the response code.
                QSharedPointer<QDmcpReadRequest> newRequest(new QDmcpReadRequest);
                newRequest->setData(thisRequestData);
                emit readResponse(newRequest.data(), values, static_cast<QDmcpConnection::ResponseCode>(responseCode));
            }
            break;
        }
    }
}
