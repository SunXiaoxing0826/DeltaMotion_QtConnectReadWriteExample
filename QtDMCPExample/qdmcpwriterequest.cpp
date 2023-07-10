#include "qdmcpwriterequest.h"

QDmcpWriteRequest::QDmcpWriteRequest(QObject *parent) : QDmcpRequest(parent) {}

void QDmcpWriteRequest::setValues(const QVector<QVariant>& values) {
    QVector<QVariant> *valuesCopy = new QVector<QVariant>(values);
    m_data->m_values = QSharedPointer<QVector<QVariant>>(valuesCopy);
}

const void QDmcpWriteRequest::write(QDataStream& stream) {
    stream.setByteOrder(QDataStream::ByteOrder::LittleEndian);

    // packet length
    stream << static_cast<quint16>(14 + sizeof(qint32) * m_data->m_values->count());

    // static values
    stream << static_cast<quint16>(0x0200);

    // transaction ID
    stream << m_data->m_transactionID;

    // function code
    stream << static_cast<quint8>(0x15);

    // byte order (little endian)
    stream << static_cast<quint8>(0);

    // starting address (file)
    stream << m_data->m_file;

    // starting address (element)
    stream << m_data->m_element;

    // write count (less than 1024)
    stream << static_cast<quint16>(m_data->m_values->count());

    // reserved
    stream << static_cast<quint16>(0);

    // data
    QVectorIterator<QVariant> i(*(m_data->m_values));
    while (i.hasNext()) {
        QVariant v = i.next();
        if (v.type() == QMetaType::Float) {
            float value = v.value<float>();
            qDebug() << value;
            stream << value;
        } else {
            qint32 value = v.value<qint32>();
            qDebug() << value;
            stream << value;
        }
    }
}
