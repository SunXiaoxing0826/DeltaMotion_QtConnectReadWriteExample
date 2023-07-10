#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <stdio.h>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_ui(new Ui::MainWindow),
      m_connection(new QDmcpConnection(this))

{
    m_ui->setupUi(this);

    // Set up UI signals
    connect(m_ui->connectButton, &QPushButton::clicked, this, &MainWindow::tryToConnect);
    connect(m_ui->sendDataButton, &QPushButton::clicked, this, &MainWindow::sendWriteRequest);
    connect(m_ui->receiveDataButton, &QPushButton::clicked, this, &MainWindow::sendReadRequest);

    // Set up RMC signals
    connect(m_connection, &QDmcpConnection::connected, this, &MainWindow::onConnected);
    connect(m_connection, &QDmcpConnection::disconnected, this, &MainWindow::onDisconnected);
    connect(m_connection, &QDmcpConnection::socketErrorOccurred, this, &MainWindow::onSocketErrorOccurred);
    connect(m_connection, &QDmcpConnection::writeResponse, this, &MainWindow::onWriteResponse);
    connect(m_connection, &QDmcpConnection::readResponse, this, &MainWindow::onReadResponse);

    // Initialize program to the "disconnected" state
    this->onDisconnected();
}

void MainWindow::tryToConnect() {
    if (m_connection->state() != QTcpSocket::ConnectedState) {
        m_ui->ipAddressField->setEnabled(false);
        m_ui->connectButton->setEnabled(false);
        m_ui->connectButton->setText("Connecting...");
        m_connection->connectToRMC(m_ui->ipAddressField->text(), 1324);

    } else {
        m_connection->disconnectFromRMC();
    }
}

void MainWindow::onConnected() {
    /// <summary>
    /// Run when the application has successfully connected to the RMC.
    /// </summary>
    qDebug() << "Connected to RMC";

    m_ui->ipAddressField->setEnabled(false);
    m_ui->connectButton->setText(tr("Disconnect"));
    m_ui->connectButton->setEnabled(true);

    m_ui->addressFileField->setEnabled(true);
    m_ui->addressElementField->setEnabled(true);

    m_ui->sendDataField->setEnabled(true);
    m_ui->sendDataButton->setEnabled(true);

    m_ui->receivedDataField->setEnabled(true);
    m_ui->receiveDataButton->setEnabled(true);
}

void MainWindow::onDisconnected() {
    /// <summary>
    /// Run when the application has disconnected from the RMC.
    /// This is also run at startup, as well as when an error occurs.
    /// </summary>
    qDebug() << "Disconnected from RMC";

    m_ui->ipAddressField->setEnabled(true);
    m_ui->connectButton->setText(tr("Connect"));
    m_ui->connectButton->setEnabled(true);

    m_ui->addressFileField->setEnabled(false);
    m_ui->addressElementField->setEnabled(false);

    m_ui->sendDataField->setEnabled(false);
    m_ui->sendDataButton->setEnabled(false);

    m_ui->receivedDataField->setEnabled(false);
    m_ui->receiveDataButton->setEnabled(false);
}

void MainWindow::onSocketErrorOccurred(QAbstractSocket::SocketError) {
    /// <summary>
    /// Run when an error occurs while attempting to connect to the RMC.
    /// </summary>
    /// <param name="error">The socket error that occurred.</param>

    // Create a message box to display the error.
    QMessageBox errorBox = QMessageBox(
        QMessageBox::Icon::Critical,
        QString("Couldn't connect to RMC"),
        QString("Error: ") + m_connection->socketErrorString()
    );

    // Display the message box.
    errorBox.exec();

    // Return the UI to the "disconnected" state.
    onDisconnected();
}

void MainWindow::sendWriteRequest() {
    /// <summary>
    /// Requests the RMC to set the data at a given address.
    /// </summary>

    // Create a new write request.
    QDmcpWriteRequest request;

    // Set the starting address for this write request
    int file = m_ui->addressFileField->value();
    int element = m_ui->addressElementField->value();
    request.setStartingAddress(file, element);

    // Create a QVector that contains one element: the value we want to
    // write at the address specified above. If we wanted to write to multiple
    // registers in sequence following the starting address, we'd just append
    // more values to this QVector.
    float valueToWrite = static_cast<float>(m_ui->sendDataField->value());

    // Put the value into a QVariant.
    QVariant wrappedValue = QVariant(valueToWrite);
    QVector<QVariant> valueVector = QVector<QVariant> { wrappedValue };
    request.setValues(valueVector);

    // Send the write request to the RMC. If the RMC responds, we'll hear about it in
    // onWriteResponse().
    m_connection->sendRequest(request);
}

void MainWindow::sendReadRequest() {
    /// <summary>
    /// Requests the RMC to send the data at a given address.
    /// </summary>

    // Create a new read request on the heap.
    QDmcpReadRequest request;

    // Set the starting address for this read request.
    int file = m_ui->addressFileField->value();
    int element = m_ui->addressElementField->value();
    request.setStartingAddress(file, element);

    // Set the number of registers, starting with the register at the address above, that we
    // want to read from.
    QVector<QMetaType::Type> types = QVector<QMetaType::Type> { QMetaType::Type::Float };
    request.setReadCount(1);
    request.setReadTypes(&types);

    // Send the read request to the RMC. If the RMC responds, we'll handle it in onReadResponse().
    m_connection->sendRequest(request);
}

void MainWindow::onWriteResponse(QDmcpWriteRequest *, QDmcpConnection::ResponseCode responseCode) {
    /// <summary>
    /// Run when the RMC has responded to a write request.
    /// </summary>
    /// <param name="request">The write request that was responded to, as determined by the transaction ID.</param>
    /// <param name="responseCode">The response code returned by the RMC.</param>

    // Create a message box so that we can display the response to the user.
    QMessageBox box;

    // Three out of four response codes mean there was an error, so it's more efficient to set the
    // box's icon to indicate an error occurred, then change it to an "OK" symbol when an error was
    // not encountered.
    box.setIcon(QMessageBox::Icon::Critical);

    // Set the contents of the message box depending on the response code sent by the RMC.
    switch (responseCode) {
    case QDmcpConnection::ResponseCode::Success:
        box.setIcon(QMessageBox::Icon::Information);
        box.setText("The write was successful!");
        break;
    case QDmcpConnection::ResponseCode::Malformed:
        box.setText("Error: Request was malformed.");
        break;
    case QDmcpConnection::ResponseCode::TooLong:
        box.setText("Error: Request was too long.");
        break;
    case QDmcpConnection::ResponseCode::InvalidAddress:
        box.setText("Error: Address was invalid.");
        break;
    }

    // Display the message box.
    box.exec();
}

void MainWindow::onReadResponse(QDmcpReadRequest *, QSharedPointer<QVector<QVariant>> values, QDmcpConnection::ResponseCode responseCode) {
    /// <summary>
    /// Run when the RMC has responded to a read request.
    /// </summary>
    /// <param name="request">The read request that was responded to, as determined by the transaction ID.</param>
    /// <param name="values">The values retrieved from the RMC.</param>
    /// <param name="responseCode">The response code returned by the RMC.</param>

    // Create a message box so that we can display an error message to the user if necessary.
    QMessageBox box = QMessageBox();
    box.setIcon(QMessageBox::Icon::Critical);

    // Handle the various error response codes from the RMC.
    if (responseCode != QDmcpConnection::ResponseCode::Success) {
        switch (responseCode) {
        case QDmcpConnection::ResponseCode::Malformed:
            box.setText("Error: Request was malformed.");
            break;
        case QDmcpConnection::ResponseCode::TooLong:
            box.setText("Error: Request was too long.");
            break;
        case QDmcpConnection::ResponseCode::InvalidAddress:
            box.setText("Error: Address was invalid.");
            break;
        default:
            break;
        }

        box.exec();
    }

    // If there is not a single value in the values QVector, display an error message indicating
    // as such. (This shouldn't ever happen.)
    else if (values->count() < 1) {
        box.setText("No values were retrieved.");
        box.exec();
    }

    // If no errors occurred, get the first value from the values QVector and display it in the UI.
    else {
        QVariant value = values->at(0);

        if (value.userType() == QMetaType::Float) {
            m_ui->receivedDataField->setValue(value.value<float>());
        } else {
            m_ui->receivedDataField->setValue(value.value<qint32>());
        }
    }
}

MainWindow::~MainWindow()
{
    delete m_ui;
}

