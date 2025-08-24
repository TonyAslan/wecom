#pragma once
#include <QObject>
#include <QTcpSocket>
#include <QUdpSocket>
//该类只发送Json请求和转发数据
class ComSocket : public QObject
{
	Q_OBJECT
public:
	static ComSocket* getInstance();
	void connectServer(const QString& host, quint16 tcpPort);
	void bindReceiveMsgPort(quint16 udpPort);
	//使用Tcp发送json对象
	void sendJsonRequest(const QJsonObject& json);


private:
	explicit ComSocket(QObject* parent = nullptr);
	~ComSocket();

private slots:
	void onTcpConnected();
	void onTcpReadyRead();
	void onTcpDisconnected();
	void onUdpReadyRead();
private:

	static ComSocket* m_instance;

	QTcpSocket* m_tcpSocket;
	QUdpSocket* m_udpSocket;

signals:
	void signalTcpReceivedData(const QByteArray& data);
	void signalUdpReceivedData(const QByteArray& data);
};

