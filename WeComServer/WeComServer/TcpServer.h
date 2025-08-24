#pragma once

#include <QTcpServer>

class TcpServer : public QTcpServer
{
	Q_OBJECT

public:
	TcpServer(int port);
	~TcpServer();

public:
	bool run(); //监听

protected:
	//客户端有新连接时
	void incomingConnection(qintptr socketDescription);

signals:
	void signalTcpMsgComes(QTcpSocket*,QByteArray&);
	void signalSocketDisconnected(QTcpSocket*);

private slots:
	//处理数据 
	void onSocketDataProcessing(QByteArray& sendData, int descriptor);
	//断开连接处理
	void onSocketDisconnected(int descriptor);

private:
	int m_port;
	QList<QTcpSocket*> m_tcpSocketConnectList;
	
};

