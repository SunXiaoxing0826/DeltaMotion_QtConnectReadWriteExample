#ifndef QDMCPREQUEST_H
#define QDMCPREQUEST_H

#include <QObject>
#include <QDataStream>
#include <QVariant>
#include <QSharedData>
#include <QVector>
#include <QDebug>
#include <QPointer>

class QDmcpRequestData : public QSharedData
{
public:
    QDmcpRequestData() {}
    QDmcpRequestData(const QDmcpRequestData &other) :
        QSharedData(other),
        m_transactionID(other.m_transactionID),
        m_file(other.m_file),
        m_element(other.m_element),
        m_readCount(other.m_readCount),
        m_readTypes(other.m_readTypes),
        m_values(other.m_values),
        m_associatedData(other.m_associatedData) {}
    ~QDmcpRequestData() {}

    quint16 m_transactionID;
    quint16 m_file;
    quint16 m_element;

    // for read requests
    quint16 m_readCount;
    QSharedPointer<QVector<QMetaType::Type>> m_readTypes;

    // for write requests
    QSharedPointer<QVector<QVariant>> m_values;

    QVariant m_associatedData;
};

class QDmcpRequest : public QObject
{
    Q_OBJECT
    friend class QDmcpConnection;

public:
    explicit QDmcpRequest(QObject *parent = nullptr) : QObject(parent) { m_data = new QDmcpRequestData; }
    ~QDmcpRequest() {}

    quint16 startingAddressFile() { return m_data->m_file; }
    void setStartingAddressFile(quint16 file) { m_data->m_file = file; }

    quint16 startingAddressElement() { return m_data->m_element; }
    void setStartingAddressElement(quint16 element) { m_data->m_element = element; }

    void setStartingAddress(quint16 file, quint16 element) { m_data->m_file = file; m_data->m_element = element; }

    QVariant associatedData() { return m_data->m_associatedData; }
    void setAssociatedData(QVariant associatedData) { m_data->m_associatedData = associatedData; }

    const virtual void write(QDataStream&) {}

protected:
    QSharedDataPointer<QDmcpRequestData> m_data;

private:
    void setData(QDmcpRequestData *data) { this->m_data = data; }
};

#endif // QDMCPREQUEST_H
