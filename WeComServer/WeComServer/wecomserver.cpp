#include "wecomserver.h"
#include <QSqlRecord>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QDebug>
#include <QSqlDatabase>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QMessageBox>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QFileDialog>
#include <QTimer>

#include "TcpServer.h"
const int gtcpPort = 6666;

WeComServer::WeComServer(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
	if (!connectMySql())
	{
		QMessageBox::warning(NULL,
			QString::fromLocal8Bit("提示"),
			QString::fromLocal8Bit("连接数据库失败！"));
		close();
		return;
	}
	initTcpSocket();
	initUdpSocket();

	//做一些界面上的初始化
	m_queryInfoModel.setQuery("SELECT * FROM user_table");
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

	updateTableData();

	//定时刷新数据
	m_timer = new QTimer(this);
	m_timer->setInterval(200);
	m_timer->start();
	
	connect(m_timer, &QTimer::timeout, this, &WeComServer::onRefresh);

}

void WeComServer::updateTableData(int employeeId)
{
	ui.tableWidget->clear();
	if (employeeId != 0) //精确查找
	{
		m_queryInfoModel.setQuery(QString("SELECT * FROM user_table WHERE user_id = %1").arg(employeeId));
	}
	else //查找所有
	{
		m_queryInfoModel.setQuery(QString("SELECT * FROM user_table"));
	}

	int nRows = m_queryInfoModel.rowCount();
	int nColumns = m_queryInfoModel.columnCount();

	QModelIndex index; //模型索引

	//设置表格的行数、列数
	ui.tableWidget->setRowCount(nRows);
	ui.tableWidget->setColumnCount(nColumns);

	QStringList headerList;
	headerList << QStringLiteral("员工ID")
		<< QStringLiteral("员工姓名")
		<< QStringLiteral("员工部门")
		<< QStringLiteral("员工邮箱")
		<< QStringLiteral("头像路径")
		<< QStringLiteral("员工签名");
	ui.tableWidget->setHorizontalHeaderLabels(headerList);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	for (int i = 0; i < nRows; i++)
	{
		for (int j = 0; j < nColumns; j++)
		{
			index = m_queryInfoModel.index(i, j);
			QString strData = m_queryInfoModel.data(index).toString();
			//获取字段名称
			//当前行的记录
			QSqlRecord record = m_queryInfoModel.record(i);
			//行中 列的名字
			QString strRecordName = record.fieldName(j);//列
			ui.tableWidget->setItem(i, j, new QTableWidgetItem(strData));	
		}
	}

}

void WeComServer::on_queryIDBtn_clicked()
{
	//检测ID是否输入
	if (!ui.queryIDLineEdit->text().length())
	{
		QMessageBox::information(this,
			QStringLiteral("提示"),
			QStringLiteral("请输入员工ID"));
		ui.queryIDLineEdit->setFocus();
		m_queryID = 0;
		return;
	}

	//获取用户输入的员工ID号
	int nUserID = ui.queryIDLineEdit->text().toInt();
	//检测输入合法性
	QSqlQuery queryInfo(QString("SELECT * FROM user_table WHERE user_id = %1").arg(nUserID));
	queryInfo.exec();
	if (!queryInfo.next())
	{
		QMessageBox::information(this,
			QStringLiteral("提示"),
			QStringLiteral("请输入正确的员工ID！"));
		ui.queryIDLineEdit->setFocus();
		return;
	}
	else
	{
		m_queryID = nUserID;
	}

}

void WeComServer::onRefresh()
{
	updateTableData(m_queryID);
}

void WeComServer::on_selectPictureBtn_clicked()
{
	//获取选择的头像路径 
	m_pixPath = QFileDialog::getOpenFileName(
		this,
		QString::fromLocal8Bit("选择头像"),
		".",
		"*.png;;*.jpg"
	);

	if (!m_pixPath.size())
	{
		return;
	}

	//将头像显示到标签 
	QPixmap pixmap;
	pixmap.load(m_pixPath);

	qreal widthRatio = (qreal)ui.headLabel->width() / (qreal)pixmap.width();
	qreal heightRatio = (qreal)ui.headLabel->height() / (qreal)pixmap.height();

	QSize size(pixmap.width() * widthRatio, pixmap.height() * heightRatio);
	ui.headLabel->setPixmap(pixmap.scaled(size));
}

void WeComServer::on_addNewUser_Btn_clicked()
{
	QString strName = ui.nameLineEdit->text();
	QString strPart = ui.departmentLineEdit->text();
	QString strEmail = ui.emailLineEdit->text();
	QString strPassword = ui.passwordLineEdit->text();
	if (!strName.size() || !strPart.size() || !strEmail.size() || !strPassword.size())
	{
		QMessageBox::information(this,
			QString::fromLocal8Bit("提示"),
			QString::fromLocal8Bit("请先输入员工基本信息！"));
		
		return;
	}

	//检测员工选择头像
	if (!m_pixPath.size())
	{
		QMessageBox::information(this,
			QString::fromLocal8Bit("提示"),
			QString::fromLocal8Bit("请选择员工头像路径！"));
		return;
	}

	//数据库插入新item
	QSqlQuery maxUserID("SELECT MAX(user_id) FROM user_table");
	maxUserID.exec();
	maxUserID.next();

	int nUserID = maxUserID.value(0).toInt() + 1;

	//m_pixPath.replace("/", "\\\\");

	QString strInsertUserSql = QString("INSERT INTO user_table(user_id,user_name,user_part,user_email,user_img) VALUE(%1,\"%2\",\"%3\",\"%4\",\"%5\")")
		.arg(nUserID)
		.arg(strName)
		.arg(strPart)
		.arg(strEmail)
		.arg(m_pixPath);
	qDebug() << strInsertUserSql << endl;
	QSqlQuery insertUserSql(strInsertUserSql);
	insertUserSql.exec();

	QSqlQuery insertLoginSql(QString("INSERT INTO login_table(id,pwd) VALUE(%1,%2)")
									.arg(nUserID)
									.arg(strPassword));
	insertLoginSql.exec();

	QMessageBox::information(this,
		QString::fromLocal8Bit("提示"),
		QString::fromLocal8Bit("新增员工成功！")
	);
	m_pixPath = "";
	
	ui.nameLineEdit->clear();
	ui.departmentLineEdit->clear();
	ui.emailLineEdit->clear();
	ui.passwordLineEdit->clear();

	ui.headLabel->setText(QStringLiteral(" 员工头像 "));
}

void WeComServer::on_addFriendship_Btn_clicked()
{
	int nUserId = ui.userId_lineEdit->text().toInt();
	int nFriendId = ui.friendId_lineEdit->text().toInt();
	QString strInsertSql = QString("INSERT INTO friends_table(user_id,friend_id) VALUE(%1,%2)")
		.arg(nUserId)
		.arg(nFriendId);
	QSqlQuery insertSql(strInsertSql);
	insertSql.exec();
	QMessageBox::information(this,
		QStringLiteral("提示"),
		QStringLiteral("添加好友关系成功！")
		);
	ui.userId_lineEdit->clear();
	ui.friendId_lineEdit->clear();
}

void WeComServer::initTcpSocket()
{
	m_tcpServer = new TcpServer(gtcpPort);
	m_tcpServer->run();

	//收到tcp客户端发来的信息后进行解析
	connect(m_tcpServer, &TcpServer::signalTcpMsgComes,
		this, &WeComServer::onDealReceiveMsgs);
	connect(m_tcpServer, &TcpServer::signalSocketDisconnected,
		this, &WeComServer::onRemoveSocket);
}

void WeComServer::initUdpSocket()
{
	m_udpSender = new QUdpSocket(this);
}

bool WeComServer::connectMySql()
{
	QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
	db.setDatabaseName("test");	//数据库名称
	db.setHostName("localhost");//主机名
	db.setUserName("root");		//用户名
	db.setPassword("123456");	//密码
	db.setPort(3306);			//端口

	if (db.open())
	{
		qDebug() << "connect sql success" << endl;
		return true;
	}
	else
	{
		qDebug() << "connect sql fail" << endl;
		return false;
	}
}

bool WeComServer::authenticateUser(const QString & account, const QString & password, USERINFO & userInfo, QVector<FRIENDSIMPLE>& friends, int & friendCount)
{
	//查询密码
	qDebug() << "account = " << account << endl;
	QSqlQuery queryPassword(QString("SELECT pwd FROM login_table WHERE id = %1").arg(account));
	queryPassword.exec();
	//指向结果集第一条
	if (!queryPassword.first())
	{
		qDebug() << "authenticate fail" << endl;
		return false;
	}

	QString strPwd = queryPassword.value("pwd").toString();
	qDebug() << "query pwd = " << strPwd << endl;
	if (strPwd != password)
	{
		qDebug() << "password error" << endl;
		return false;
	}
	//查用户信息
	QSqlQuery queryUserInfo(QString("SELECT user_name,user_part,user_email,user_img FROM user_table WHERE user_id = %1").arg(account));
	queryUserInfo.exec();
	if (queryUserInfo.next())
	{
		// 填充用户信息
		userInfo.userId = account.toInt();
		userInfo.userName = queryUserInfo.value(0).toString();
		userInfo.userPart = queryUserInfo.value(1).toString();
		userInfo.userEmail = queryUserInfo.value(2).toString();
		userInfo.userImg = queryUserInfo.value(3).toString();
	}
	else
	{
		qDebug() << "query userinfo fail" << endl;
		return false;
	}
	
	//查好友列表
	//QString strSql = QString("SELECT CASE WHEN user_id = %1 THEN friend_id ELSE user_id END AS friend_id FROM friends_table WHERE user_id = %2 OR friend_id = %3").arg(account, account, account);
	//qDebug() << "query Sql = " << strSql << endl;
	QSqlQuery queryFrienList(QString("SELECT CASE WHEN user_id = %1 THEN friend_id ELSE user_id END AS friend_id FROM friends_table WHERE user_id = %2 OR friend_id = %3").arg(account,account,account));
	queryFrienList.exec();
	if (queryFrienList.first())
	{
		int nFriendCount = 0;
		do
		{
			int friendId = queryFrienList.value("friend_id").toInt();
			qDebug() << "friendId = " << friendId << " ";
			QSqlQuery queryFriendName(QString("SELECT user_name FROM user_table WHERE user_id = %1").arg(friendId));
			queryFriendName.exec();
			if (queryFriendName.next())
			{
				QString friendName = queryFriendName.value(0).toString();
				friends.append({ friendId,friendName });
			}

			nFriendCount++;
		} while (queryFrienList.next());
		friendCount = nFriendCount;
	}
	else 
	{
		qDebug() << "query friendlist fail" << endl;
		return false;
	}
	return true;
}

bool WeComServer::getFriendDetails(const QVector<int>& friendsId, QVector<FRIENDDETAIL>& friendDetails)
{
	if (friendsId.isEmpty())
	{
		qDebug() << "friendsid empty" << endl;
		return true;
	}
	QStringList idList;
	for (int nId: friendsId)
	{
		idList.append(QString::number(nId));
	}
	QSqlQuery queryFriendsDetail;
	queryFriendsDetail.prepare(QString("SELECT user_id,user_name,user_part,user_email,user_img,user_sign FROM user_table WHERE user_id IN(%1)").arg(idList.join(",")));
	queryFriendsDetail.exec();
	if (queryFriendsDetail.first())
	{
		friendDetails.clear();
		do
		{
			friendDetails.append({
				queryFriendsDetail.value(0).toInt(),
				queryFriendsDetail.value(1).toString(),
				queryFriendsDetail.value(2).toString(),
				queryFriendsDetail.value(3).toString(),
				queryFriendsDetail.value(4).toString(),
				queryFriendsDetail.value(5).toString()
				});
		} while (queryFriendsDetail.next());
	}
	else
	{
		qDebug() << "query friends detail fail" << endl;
		return false;
	}
	return true;
}

void WeComServer::handleLoginRequest(QTcpSocket * clientSocket, const QJsonObject & request)
{
	QJsonObject loginObj = request["login"].toObject();
	QString strAccount = loginObj["account"].toString();
	QString strPassword = loginObj["password"].toString();
	USERINFO userInfo;
	QVector<FRIENDSIMPLE> vFriends;
	int nFriendCount = 0;
	if (!authenticateUser(strAccount, strPassword, userInfo, vFriends, nFriendCount))
	{
		sendErrorResponse(clientSocket, 3000, "NO", "authenticate fail");
		return;
	}
	//成功验证的话 构建精简好友JSON数组
	QJsonArray friendArray;
	for (const FRIENDSIMPLE & friendInfo : vFriends)
	{
		friendArray.append
		(QJsonObject
		{
			{"id",friendInfo.id},
			{"name",friendInfo.name}
		}
		);
	}

	//构建登录响应
	QJsonObject response
	{
		{"type", "0"},
		{"data", QJsonObject{
		{"userId", userInfo.userId},
		{"userName", userInfo.userName},
		{"userPart", userInfo.userPart},
		{"userEmail", userInfo.userEmail},
		{"userImg", userInfo.userImg},
		{"list", friendArray},
		{"friendCount", nFriendCount}
		}},
		{"status", 1000},
		{"desc", "OK"}
	};

	//成功登录之后 记录一下socket和对应用户id
	int nId = userInfo.userId;
	m_mapIdSocket.insert(clientSocket, nId);
	sendJsonResponse(clientSocket, response);
}

void WeComServer::handleFriendDetailRequest(QTcpSocket * clientSocket, const QJsonObject & request)
{
	QJsonArray friendIdArray = request["friendIds"].toArray();
	QVector<int> vFriendIds;
	for (const QJsonValue &value : friendIdArray)
	{
		vFriendIds.append(value.toInt());
	}
	if (vFriendIds.isEmpty())
	{
		sendErrorResponse(clientSocket, 4003, "Invalid request", "No friends Id");
		return;
	}

	QVector<FRIENDDETAIL> friendDetails;

	if (!getFriendDetails(vFriendIds, friendDetails))
	{
		sendErrorResponse(clientSocket, 2000, "database error", "fail to get friend details");
		return;
	}

	qDebug() << "friendDetails.size = " << friendDetails.size();

	//构建好友详情的JSON数组
	QJsonArray friendDetailsArray;
	for (const FRIENDDETAIL& detail : friendDetails)
	{
		friendDetailsArray.append
		(QJsonObject{
			{"id",detail.id},
			{"name",detail.name},
			{"part",detail.part},
			{"email",detail.email},
			{"img",detail.img},
			{"sign",detail.sign}
		}
		);
	}
	//构建详情相应
	QJsonObject response
	{
		{"type","2"},
		{"data",QJsonObject{
			{"friendDetails",friendDetailsArray}
		}},
		{"status",1000},
		{"desc","OK"}
	};

	sendJsonResponse(clientSocket, response);
}

void WeComServer::handleChatRequest(QTcpSocket * clientSocket, const QJsonObject & request)
{
	if (!m_mapIdSocket.contains(clientSocket))
	{
		qDebug() << "unauthrized chat request" << endl;
		return;
	}
	int nSenderId = m_mapIdSocket[clientSocket];
	QJsonObject data = request["data"].toObject();
	int nReceiverId = data["recv"].toInt();
	QJsonArray msgArray = data["msg"].toArray();

	//分发聊天消息
	forwardChatMessage(nSenderId, nReceiverId, msgArray);

	//发送成功响应
	QJsonObject response
	{
		{"type","1"},
		{"status",1000},
		{"desc","OK"}
	};
	sendJsonResponse(clientSocket, response);
}

void WeComServer::forwardChatMessage(int senderId, int receiverId, const QJsonArray & messages)
{
	//广播分发
	QJsonObject chatObj
	{
		{"type","1"},
		{"data", QJsonObject{
			{"send",senderId},
			{"recv",receiverId},
			{"what","msg"},
			{"msg",messages}
		}}
	};

	QJsonDocument doc(chatObj);
	QByteArray data = doc.toJson(QJsonDocument::Compact);

	qDebug() << "boardcast data = " << data << endl;

	//多端口广播
	for (quint16 port = 10000; port < 10000 + 200; ++port)
	{
		m_udpSender->writeDatagram(data, data.size(), QHostAddress::Broadcast, port);
	}
}

void WeComServer::sendJsonResponse(QTcpSocket * clientSocket, const QJsonObject & response)
{
	QJsonDocument doc(response);
	//QJsonDocument::Compact:指定输出为紧凑格式
	QByteArray data = doc.toJson(QJsonDocument::Compact);
	clientSocket->write(data);
	clientSocket->flush();
}

void WeComServer::sendErrorResponse(QTcpSocket * clientSocket, int status, const QString & desc, const QString & message)
{
	QJsonObject response
	{
		{"status",status},
		{"desc",desc},
		{"message",message},
	};
	qDebug() << "response error " << status << desc << message << endl;
	sendJsonResponse(clientSocket, response);
}

void WeComServer::onRemoveSocket(QTcpSocket *item)
{
	if(item)
		m_mapIdSocket.remove(item);
}

void WeComServer::onDealReceiveMsgs(QTcpSocket* clientSocket, QByteArray& data)
{
	qDebug() << " receive : "<< data << endl;
	//QTcpSocket* clientSocket = qobject_cast<QTcpSocket*>(sender());
	if (!clientSocket)
	{
		return;
	}
	QJsonDocument doc = QJsonDocument::fromJson(data);
	if (doc.isNull() || !doc.isObject())
	{
		sendErrorResponse(clientSocket, 4000, "Invalid Request", "Invalid JSON format");
		qDebug() << "invalid json request" << endl;
		return;
	}

	QJsonObject request = doc.object();
	QString type = request["type"].toString();

	if (type == "0") {  //登录请求
		handleLoginRequest(clientSocket, request);
	}
	else if (type == "1") //聊天请求
	{
		handleChatRequest(clientSocket, request);
	}
	else if (type == "2") { //查询好友详情请求
		handleFriendDetailRequest(clientSocket, request);
	}
	else {
		sendErrorResponse(clientSocket, 4001, "Unsupported Type", "Unsupported request type");
	}

}