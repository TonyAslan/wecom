#pragma once

#include <QtWidgets/QWidget>
#include <QJsonObject>
#include <QTcpSocket>
#include <QUdpSocket>
#include "ui_wecomserver.h"
#include <QSqlQueryModel>
typedef struct USERINFO {
	int userId;
	QString userName;
	QString userPart;
	QString userEmail;
	QString userImg;
}UserInfo;

typedef struct FRIENDSIMPLE {
	int id;
	QString name;
}FriendSimple;

typedef struct FRIENDDETAIL {
	int id;
	QString name;
	QString part;
	QString email;
	QString img;
	QString sign;
}FriendDetail;

class TcpServer;

class WeComServer : public QWidget
{
    Q_OBJECT

public:
    WeComServer(QWidget *parent = Q_NULLPTR);

private:
	void updateTableData(int employeeId = 0);
	void initTcpSocket();
	void initUdpSocket();
	bool connectMySql();
	//用户认证和基本信息获取
	bool authenticateUser(const QString& account,
		const QString& password,
		USERINFO& userInfo,
		QVector<FRIENDSIMPLE>& friends,
		int& friendCount);
	bool getFriendDetails(const QVector<int>& friends,
		QVector<FRIENDDETAIL>& friendDetails);

	void handleLoginRequest(QTcpSocket* clientSocket, const QJsonObject& request);
	void handleFriendDetailRequest(QTcpSocket* clientSocket, const QJsonObject& request);
	void handleChatRequest(QTcpSocket* clientSocket, const QJsonObject& request);
	void forwardChatMessage(int senderId, int receiverId, const QJsonArray &messages);
	void sendJsonResponse(QTcpSocket* clientSocket, const QJsonObject& response);
	void sendErrorResponse(QTcpSocket* clientSocket, int status, const QString& desc, const QString& message);

private slots:
	void onDealReceiveMsgs(QTcpSocket*, QByteArray&);
	void onRemoveSocket(QTcpSocket*);
	void on_queryIDBtn_clicked();
	void onRefresh();
	void on_selectPictureBtn_clicked();
	void on_addNewUser_Btn_clicked();
	void on_addFriendship_Btn_clicked();
private:
	QTimer* m_timer;  //定时刷新数据
	QString m_pixPath; //头像路径
	QSqlQueryModel m_queryInfoModel; //查询所有员工的信息模型
	int m_queryID;
	TcpServer* m_tcpServer; //tcp服务端
	QUdpSocket* m_udpSender; //udp广播
	//用户ID和连接的Socket的映射
	QMap<QTcpSocket*, int> m_mapIdSocket;
private:
    Ui::WeComServerClass ui;
};
