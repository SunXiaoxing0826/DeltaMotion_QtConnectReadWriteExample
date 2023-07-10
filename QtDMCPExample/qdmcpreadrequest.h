#ifndef QDMCPREADREQUEST_H
#define QDMCPREADREQUEST_H

#include "qdmcprequest.h"

class QDmcpReadRequest : public QDmcpRequest
{
public:
    explicit QDmcpReadRequest(QObject *parent = nullptr);

    quint16 readCount() { return m_data->m_readCount; }
    void setReadCount(quint16 count) { m_data->m_readCount = count; }

    QVector<QMetaType::Type> readTypes() { return *m_data->m_readTypes; }
    void setReadTypes(QVector<QMetaType::Type> *readTypes);

    const void write(QDataStream& stream) override;

};

#endif // QDMCPREADREQUEST_H
