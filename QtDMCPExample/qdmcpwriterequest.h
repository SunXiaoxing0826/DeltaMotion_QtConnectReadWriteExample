#ifndef QDMCPWRITEREQUEST_H
#define QDMCPWRITEREQUEST_H

#include "qdmcprequest.h"
#include <QVector>

class QDmcpWriteRequest : public QDmcpRequest
{
public:
    explicit QDmcpWriteRequest(QObject *parent = nullptr);

//    QVector<qint32> *values() { return QPointer<QVector<qint32>>(new QVector(*data->m_values)); }
    void setValues(const QVector<QVariant>& values);

    const void write(QDataStream& stream) override;

};

#endif // QDMCPWRITEREQUEST_H
