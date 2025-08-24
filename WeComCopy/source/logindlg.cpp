#include "logindlg.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include "iconhelper.h"
#include "msgqueue.h"
#include <QTimer>
#include <QDesktopWidget>
#include "wecomwnd.h"
#include <QLabel>
#include <QTcpSocket>
#include <QLineEdit>
#include <QJsonDocument>
#include <QJsonObject>
#include "comsocket.h"
int gCurrentLoginId;
QString gCurrentLoginName;
QString gCurrentLoginImg;

CLoginDlg::CLoginDlg(QWidget *parent) : CBaseDlg(parent)
{
    m_labCompany = NULL;
    m_labUserIcon = NULL;
    m_labUserName = NULL;
    m_labTip = NULL;
    m_btnCfg = NULL;
    m_labQRcode = NULL;

	m_labUserId = NULL;
	m_labUserPassword = NULL;
	m_userIdEdit = NULL;
	m_passwordEdit = NULL;
	m_loginBtn = NULL;

    CreateAllChildWnd();
    InitCtrl(); //show 输入用户id密码 其他隐藏
    InitSolts();
    Relayout();
	//连接服务器
	ComSocket::getInstance()->connectServer("127.0.0.1", 6666);
    //连接接受数据的信号槽
	connect(ComSocket::getInstance(), &ComSocket::signalTcpReceivedData, this, &CLoginDlg::onReadyRead);
	//QTimer::singleShot(2000, this, SLOT(OnLoging()));
	//应该是按钮点击然后开始登录 
	//向服务器发送一个查询的json指令
	connect(m_loginBtn, SIGNAL(clicked()), this, SLOT(onSendLoginRequest()));
}

void CLoginDlg::CreateAllChildWnd()
{
#define NEW_OBJECT(pObj, TYPE) \
    if (NULL == pObj) { pObj = new TYPE(this); }

    NEW_OBJECT(m_labCompany, QLabel);
    NEW_OBJECT(m_labUserIcon, QLabel);
    NEW_OBJECT(m_labUserName, QLabel);
    NEW_OBJECT(m_labTip, QLabel);
    NEW_OBJECT(m_btnCfg, CPushButtonEx);
    NEW_OBJECT(m_labQRcode, QLabel);

	NEW_OBJECT(m_labUserId, QLabel);
	NEW_OBJECT(m_labUserPassword, QLabel);
	NEW_OBJECT(m_userIdEdit, QLineEdit);
	NEW_OBJECT(m_passwordEdit, QLineEdit);
	NEW_OBJECT(m_loginBtn, QPushButton);

	//NEW_OBJECT(m_socket, QTcpSocket);
}

void CLoginDlg::InitCtrl()
{
    setFixedSize(300, 425);
	//允许拖动
    EnableMoveWindow(true);
	//获取桌面对象
    QDesktopWidget *desktopWidget = QApplication::desktop();
	//获取屏幕尺寸
    QRect screenRect = desktopWidget->screenGeometry();
	//将窗口移动到屏幕中央
    move((screenRect.width() - this->width())/2, (screenRect.height() - this->height()) / 2);

	//设置用户头像标签 - 使用图片作为背景
    m_labUserIcon->setStyleSheet("border-image: url(:/qss/res/usricon.jpeg);");
    m_labUserIcon->setFixedSize(110, 110);
	//设置二维码标签 - 使用图片作为背景
    m_labQRcode->setStyleSheet("border-image: url(:/qss/res/QRcode.png);");
    m_labQRcode->setFixedSize(200, 200);

    m_btnCfg->setFixedSize(40, 40);
    m_labCompany->setText(tr("造化天宫"));
    m_labUserName->setText(tr("逍遥子"));
    m_labTip->setText(tr("登录"));
    m_labCompany->setAlignment(Qt::AlignCenter);
    m_labUserName->setAlignment(Qt::AlignCenter);
    m_labTip->setAlignment(Qt::AlignCenter);

    IconHelper::SetIcon(m_btnCfg, QChar(0xe642));
    m_btnCfg->setProperty("white_bk", "true");

    m_labCompany->setStyleSheet("QLabel{font: bold 13px; color:#36608F;}");
    m_labUserName->setStyleSheet("QLabel{font: bold 16px; color:#000000;}");
    m_labTip->setStyleSheet("QLabel{font: 14px; color:#000000;}");

    //m_labQRcode->show();
	m_labQRcode->hide();
    m_labCompany->hide();
    m_labUserIcon->hide();
    m_labUserName->hide();

	m_labUserId->setText("用户ID: ");
	m_labUserPassword->setText("密码: ");
	m_labUserId->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	m_labUserPassword->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	m_labUserId->setStyleSheet("QLabel{font: bold 16px; color:#000000;}");
	m_labUserPassword->setStyleSheet("QLabel{font: bold 16px; color:#000000;}");
	m_userIdEdit->setFixedSize(150,30);
	m_labUserId->setFixedSize(80, 30);
	m_labUserPassword->setFixedSize(80, 30);
	m_passwordEdit->setFixedSize(150, 30);
	m_passwordEdit->setEchoMode(QLineEdit::Password); // 设置为密码模式
	m_loginBtn->setFixedSize(80, 30);
	m_loginBtn->setText("登录");

}

void CLoginDlg::InitSolts()
{

}

void CLoginDlg::Relayout()
{
    QHBoxLayout *layoutIcon = new QHBoxLayout();
    layoutIcon->addStretch();
    layoutIcon->addWidget(m_labQRcode);
    layoutIcon->addWidget(m_labUserIcon);
    layoutIcon->addStretch();

    QHBoxLayout *layoutCfg = new QHBoxLayout();
    layoutCfg->addStretch();
    layoutCfg->addWidget(m_btnCfg);
    layoutCfg->setContentsMargins(0, 0, 10, 0);

    QVBoxLayout *layoutUser = new QVBoxLayout();
    layoutUser->addLayout(layoutIcon);
    layoutUser->addWidget(m_labUserName);
    layoutUser->setSpacing(0);
    layoutUser->setMargin(0);

	QHBoxLayout *layoutInputId = new QHBoxLayout();
	layoutInputId->addStretch();
	layoutInputId->addWidget(m_labUserId);
	layoutInputId->addWidget(m_userIdEdit);
	layoutInputId->addStretch();
	layoutInputId->setSpacing(10);

	QHBoxLayout *layoutInputPassword = new QHBoxLayout();
	layoutInputPassword->addStretch();
	layoutInputPassword->addWidget(m_labUserPassword);
	layoutInputPassword->addWidget(m_passwordEdit);
	layoutInputPassword->addStretch();
	layoutInputPassword->setSpacing(10);

	QHBoxLayout *layoutLoginBtn = new QHBoxLayout();
	layoutLoginBtn->addStretch();
	layoutLoginBtn->addWidget(m_loginBtn);
	layoutLoginBtn->addStretch();
	layoutLoginBtn->setSpacing(20);

    QVBoxLayout *layoutVMain = new QVBoxLayout();
    layoutVMain->addWidget(m_labCompany);
    layoutVMain->addLayout(layoutUser);
    layoutVMain->addWidget(m_labTip);
    layoutVMain->addStretch();

	layoutVMain->addLayout(layoutInputId);
	layoutVMain->addLayout(layoutInputPassword);
	layoutVMain->addLayout(layoutLoginBtn);

    layoutVMain->addLayout(layoutCfg);
    layoutVMain->setSpacing(40);
    layoutVMain->setContentsMargins(0, 40, 0, 10);
	//centralWidget() 返回基类内定义的中间部件
    centralWidget()->setLayout(layoutVMain);
}

void CLoginDlg::OnLoginFinish()
{
	//当前登录者的id
	gCurrentLoginId = m_userId;
	gCurrentLoginName = m_userName;
	gCurrentLoginImg = m_userImg;
	//绑定消息通信的端口 用id做端口号
	ComSocket::getInstance()->bindReceiveMsgPort((quint16)m_userId);
	m_weComWnd->show();
    hide();
    //emit SignalLoginFinish();
}

void CLoginDlg::OnLoging()
{
    m_labTip->setText(tr("正在登录..."));
    m_labQRcode->hide();
	
	m_labUserId->hide();
	m_labUserPassword->hide();
	m_userIdEdit->hide();
	m_passwordEdit->hide();
	m_loginBtn->hide();
	m_labCompany->setText(m_userPart);
    m_labCompany->show();
	m_labUserIcon->setStyleSheet(QString("border-image: url(%1);").arg(m_userImg));
    m_labUserIcon->show();
	m_labUserName->setText(m_userName);
    m_labUserName->show();

	//登录中查好友详情
	if (!m_vFriendsId.isEmpty())
	{
		//根据需求查登入后界面可视部分
		QVector<int> vRequestId;
		//假定只显示5个
		for (int i = 0; i < qMin(5, m_vFriendsId.size()); ++i)
		{
			vRequestId.append(m_vFriendsId[i]);
			qDebug() << "vRequestId = " << m_vFriendsId[i] << " ";
		}
		//发送查详情请求
		sendFriendsDetailRequest(vRequestId);
	}
	//在显示主窗口之前，先设置号好友信息


    //QTimer::singleShot(3000, this, SLOT(OnLoginFinish()));
}

void CLoginDlg::OnBtnCloseClicked()
{
    qApp->quit();
    reject();
}

void CLoginDlg::onSendLoginRequest()
{
	//发送登录请求
	qDebug() << "send login request" << endl;
	/*if (m_socket->state() != QAbstractSocket::ConnectedState) {
		qDebug() << "connection error";
		return;
	}*/
	QJsonObject loginObj;

	loginObj["account"] = m_userIdEdit->text();
	loginObj["password"] = m_passwordEdit->text();

	QJsonObject request;
	request["type"] = QString("0");
	request["login"] = loginObj;

	ComSocket::getInstance()->sendJsonRequest(request);
}


void CLoginDlg::onReadyRead(const QByteArray& data)
{
	//QByteArray data = m_socket->readAll();
	QJsonDocument doc = QJsonDocument::fromJson(data);
	if (doc.isNull() || !doc.isObject())
	{
		qDebug() << "received invalid JSON response";
		return;
	}

	QJsonObject response = doc.object();
	QString type = response["type"].toString();
	int status = response["status"].toInt();
	QString desc = response["desc"].toString();

	qDebug() << "Received response type:" << type << "status:" << status << desc;

	if (type == "0") { // 登录响应
		handleLoginResponse(response);
	}
	else if (type == "2")
	{
		//查好友详情响应
		handleFriendsDetailResponse(response);
	}
	else {
		qDebug() << "Unsupported response type:" << type;
	}

}
//处理登录回复
void CLoginDlg::handleLoginResponse(const QJsonObject& response)
{
	//解析用户信息
	QJsonObject dataObj = response["data"].toObject();
	m_userId = dataObj["userId"].toInt();
	m_userName = dataObj["userName"].toString();
	m_userPart = dataObj["userPart"].toString();
	m_userEmail = dataObj["userEmail"].toString();
	m_userImg = dataObj["userImg"].toString();
	int nFriendCount = dataObj["friendCount"].toInt();

	qDebug() << "User info:"
		<< "\nID:" << m_userId
		<< "\nName:" << m_userName
		<< "\nDepartment:" << m_userPart
		<< "\nEmail:" << m_userEmail
		<< "\nAvatar:" << m_userImg
		<< "\nFriends count:" << nFriendCount;

	QJsonArray friendList = dataObj["list"].toArray();

	for (const QJsonValue& value : friendList)
	{
		QJsonObject friendObj = value.toObject();
		int friendId = friendObj["id"].toInt();
		QString friendName = friendObj["name"].toString();
		m_vFriendsId.append(friendId);
		m_mapFriends[friendId] = { friendId,friendName,"","","" };
	}

	//拿到一些基本信息 设置登录界面
	QTimer::singleShot(2000, this, SLOT(OnLoging()));
}

//处理详情回复
void CLoginDlg::handleFriendsDetailResponse(const QJsonObject & response)
{
	QJsonObject dataObj = response["data"].toObject();
	//解析好友详情
	QJsonArray friendDetailArray = dataObj["friendDetails"].toArray();
	QVector<FRIENDINFO> vFriendInfo;
	vFriendInfo.clear();
	for (const QJsonValue& value : friendDetailArray)
	{
		QJsonObject detail = value.toObject();
		int nId = detail["id"].toInt();
		if (m_mapFriends.contains(nId))
		{
			auto& friendInfo = m_mapFriends[nId];
			friendInfo.id = nId;
			friendInfo.part = detail["part"].toString();
			friendInfo.email = detail["email"].toString();
			friendInfo.img = detail["img"].toString();
			friendInfo.sign = detail["sign"].toString();
			qDebug() << "friend details" << nId << friendInfo.name << endl;
			vFriendInfo.append(friendInfo);
		}
		
	}
	//拿到好友详情后 拿信息构建主窗口
	m_weComWnd = new WeComWnd();
	//设置个人信息
	m_weComWnd->setUserDetail(m_userName, m_userImg, m_userEmail, m_userPart);
	//设置好友列表
	m_weComWnd->setFriendList(vFriendInfo);
	//设置好后 再登录完成 显示页面
	QTimer::singleShot(1000, this, SLOT(OnLoginFinish()));
	
}

void CLoginDlg::sendFriendsDetailRequest(const QVector<int>& vFriendIds)
{
	QJsonArray friendsIdArray;
	for (int nId : vFriendIds)
	{
		friendsIdArray.append(nId);
	}

	QJsonObject request;
	request["type"] = "2";
	request["friendIds"] = friendsIdArray;

	ComSocket::getInstance()->sendJsonRequest(request);
}



