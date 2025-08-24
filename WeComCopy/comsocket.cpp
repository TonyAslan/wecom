#include <QJsonDocument>
#include <QNetworkDatagram>
#include "comsocket.h"
ComSocket* ComSocket::m_instance = nullptr;
ComSocket * ComSocket::getInstance()
{
	if (!m_instance)
	{
		m_instance = new ComSocket;
	}
	return m_instance;
}

void ComSocket::connectServer(const QString & host, quint16 tcpPort)
{
	m_tcpSocket = new QTcpSocket(this);
	connect(m_tcpSocket, &QTcpSocket::connected, this, &ComSocket::onTcpConnected);
	connect(m_tcpSocket, &QTcpSocket::disconnected, this, &ComSocket::onTcpDisconnected);
	connect(m_tcpSocket, &QTcpSocket::readyRead, this, &ComSocket::onTcpReadyRead);
	m_tcpSocket->connectToHost(host, tcpPort);

	m_udpSocket = new QUdpSocket(this);
	connect(m_udpSocket, &QUdpSocket::readyRead, this, &ComSocket::onUdpReadyRead);
	
}
void ComSocket::bindReceiveMsgPort(quint16 udpPort)
{
	if (!m_udpSocket->bind(QHostAddress::Any, udpPort))
	{
		qDebug() << "bind receive message port error" << endl;
	}
}
void ComSocket::onTcpConnected()
{
	qDebug() << __FUNCTION__;
}
void ComSocket::onTcpDisconnected()
{
	qDebug() << __FUNCTION__;
}
void ComSocket::onUdpReadyRead()
{
	while (m_udpSocket->hasPendingDatagrams()) {
		QNetworkDatagram datagram = m_udpSocket->receiveDatagram();
		QByteArray data = datagram.data();
		emit signalUdpReceivedData(data);
	}
}
void ComSocket::onTcpReadyRead()
{
	QByteArray data = m_tcpSocket->readAll();
	emit signalTcpReceivedData(data);
}
void ComSocket::sendJsonRequest(const QJsonObject & json)
{
	qDebug() << "ComSocket::sendJsonRequest" << endl;
	QJsonDocument doc(json);
	QByteArray data = doc.toJson();
	m_tcpSocket->write(data);
	m_tcpSocket->flush();
}

ComSocket::ComSocket(QObject * parent)
{
}

ComSocket::~ComSocket()
{
	delete m_instance;
	m_instance = NULL;
}
