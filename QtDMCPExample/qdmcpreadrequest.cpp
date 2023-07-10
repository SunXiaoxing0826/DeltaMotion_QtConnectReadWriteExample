#include "qdmcpreadrequest.h"

QDmcpReadRequest::QDmcpReadRequest(QObject *parent) : QDmcpRequest(parent) {}

void QDmcpReadRequest::setReadTypes(QVector<QMetaType::Type> *readTypes) {
    QVector<QMetaType::Type> *readTypesCopy = new QVector<QMetaType::Type>(*readTypes);
    m_data->m_readTypes = QSharedPointer<QVector<QMetaType::Type>>(readTypesCopy);
}

const void QDmcpReadRequest::write(QDataStream& stream) {
    stream.setByteOrder(QDataStream::ByteOrder::LittleEndian);

    // packet length
    stream << static_cast<quint16>(0x000C);

    // static values
    stream << static_cast<quint16>(0x0200);

    // transaction ID
    stream << m_data->m_transactionID;

    // function code
    stream << static_cast<quint8>(0x14);

    // byte order (little endian)
    stream << static_cast<quint8>(0);

    // starting address (file)
    stream << m_data->m_file;

    // starting address (element)
    stream << m_data->m_element;

    // read count
    stream << m_data->m_readCount;
}
