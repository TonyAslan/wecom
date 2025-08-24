#pragma once

#include <QTcpServer>

class TcpServer : public QTcpServer
{
	Q_OBJECT

public:
	TcpServer(int port);
	~TcpServer();

public:
	bool run(); //����

protected:
	//�ͻ�����������ʱ
	void incomingConnection(qintptr socketDescription);

signals:
	void signalTcpMsgComes(QTcpSocket*,QByteArray&);
	void signalSocketDisconnected(QTcpSocket*);

private slots:
	//�������� 
	void onSocketDataProcessing(QByteArray& sendData, int descriptor);
	//�Ͽ����Ӵ���
	void onSocketDisconnected(int descriptor);

private:
	int m_port;
	QList<QTcpSocket*> m_tcpSocketConnectList;
	
};

