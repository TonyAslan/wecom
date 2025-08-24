#include "msgpane.h"
#include "friendslist.h"
#include <QVariant>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QKeyEvent>
#include <QWebEngineSettings>
#include <QWebEngineProfile>
#include "msgqueue.h"
#include "chattoptoolbar.h"
#include "pushbuttonex.h"
#include "comsocket.h"
#include <QVariantList>
#include <QJsonValue>
extern int gCurrentLoginId;
extern QString gCurrentLoginName;
extern QString gCurrentLoginImg;

CMsgPane::CMsgPane(QWidget *parent) : QWidget(parent)
{
    m_pFriendsList = NULL;
    m_pTopToolbar = NULL;
    m_pViewChat = NULL;
    m_textEdit = NULL;
    m_btnSend = NULL;
    m_labSeparatorLine = NULL;

    CreateAllChildWnd();
    InitCtrl();
    InitSolts();
    Relayout();
}


void CMsgPane::setFriendsList(const QVector<FRIENDINFO>& vFriendsInfo)
{
	m_pFriendsList->setFriendList(vFriendsInfo);
}

void CMsgPane::CreateAllChildWnd()
{
#define NEW_OBJECT(pObj, TYPE) \
    if (NULL == pObj) { pObj = new TYPE(this); }

    NEW_OBJECT(m_pFriendsList, CFriendsList);
    NEW_OBJECT(m_pTopToolbar, CChatTopToolbar);
    NEW_OBJECT(m_pViewChat, QWebEngineView);
    NEW_OBJECT(m_textEdit, QTextEdit);
    NEW_OBJECT(m_btnSend, CPushButtonEx);
    NEW_OBJECT(m_labSeparatorLine, QLabel);
}
//消息面板初始化
void CMsgPane::InitCtrl()
{
    setAttribute(Qt::WA_StyledBackground);  // 禁止父窗口样式影响子控件样式
    setProperty("form", "msgpane");
	//好友列表
    m_pFriendsList->setFixedWidth(250);
    m_textEdit->setFixedHeight(110);
    m_textEdit->setStyleSheet("font: 14px; color:#000000;");

    m_btnSend->setFixedSize(80, 40);
    m_btnSend->setText(tr("发送(S)"));
    m_btnSend->setProperty("sendbtn", "true");
	//分隔线
    m_labSeparatorLine->setFixedHeight(1);
    m_labSeparatorLine->setStyleSheet("background-color:#4A7ABA");
    m_labSeparatorLine->hide();
	//QWebEngineView
	//禁用网页视图的的右键上下文菜单
    m_pViewChat->setContextMenuPolicy(Qt::NoContextMenu);
	//加载嵌入资源中的Html聊天界面
    m_pViewChat->load(QUrl("qrc:/html/html/index1.html"));
	//注册拦截器
	m_pViewChat->page()->profile()->setUrlRequestInterceptor(new AvatarRequestInterceptor);
	//显示网页视图
    m_pViewChat->show();
	//为输入框安装事件过滤器
    m_textEdit->installEventFilter(this);
    m_textEdit->setFixedHeight(200);
}

void CMsgPane::InitSolts()
{
	//好友列表点击新项时发出信号 即与新的好友对话
	//更新用户信息栏
    connect(m_pFriendsList, SIGNAL(SignalFriendChange(TUserInfo)), m_pTopToolbar, SLOT(OnFriendChange(TUserInfo)));
	//刷新聊天页面
	connect(m_pFriendsList, SIGNAL(SignalFriendChange(TUserInfo)), this, SLOT(OnFriendChange(TUserInfo)));
	//点击发送按钮
	connect(m_btnSend, SIGNAL(clicked()), this, SLOT(OnBtnSendClicked()));
	//接受信息
    //connect(MSGQUEUE, SIGNAL(SignalRecvMsg(QByteArray,QObject *)), this, SLOT(OnRecvMsg(QByteArray, QObject *)));
	connect(ComSocket::getInstance(), &ComSocket::signalUdpReceivedData, this, &CMsgPane::onReceivedData);
}

//ui布局 左侧 ： 联系人列表
//右侧：
//顶部用户信息栏
//聊天页面
//分隔线
//输入框
//发送按钮
void CMsgPane::Relayout()
{
	//水平布局加个弹簧
    QHBoxLayout *layoutSend = new QHBoxLayout();
    layoutSend->addStretch();
    layoutSend->addWidget(m_btnSend);
	//垂直布局
    QVBoxLayout *layoutRMain = new QVBoxLayout();
    layoutRMain->addWidget(m_pTopToolbar);//顶部用户信息栏
    layoutRMain->addWidget(m_pViewChat);  //聊天界面
    layoutRMain->addWidget(m_labSeparatorLine); //分割线
    layoutRMain->addWidget(m_textEdit);//输入框
    layoutRMain->addLayout(layoutSend);//发送按钮
	//水平布局
    QHBoxLayout *layoutMain = new QHBoxLayout();
    layoutMain->addWidget(m_pFriendsList);//好友列表
    layoutMain->addLayout(layoutRMain);
    layoutMain->setSpacing(0);
    layoutMain->setMargin(0);

    setLayout(layoutMain);
}

bool CMsgPane::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == m_textEdit)   //当前部件是输入框 
    {
        if (e->type() == QEvent::KeyPress)
        {
            QKeyEvent *event = static_cast<QKeyEvent*>(e);
			//按下回车键时发送消息
            if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
            {
                OnBtnSendClicked(); //发送消息的槽
                return true;
            }
        }
    }

    return QWidget::eventFilter(obj, e);
}

void CMsgPane::handleChatMessage(const QJsonObject & message)
{
	QJsonObject data = message["data"].toObject();
	int nSenderId = data["send"].toInt();
	int nReceiverId = data["recv"].toInt();
	QJsonArray msgArray = data["msg"].toArray();
	if (nReceiverId != gCurrentLoginId) //不是发给自己的不要
	{
		return;
	}
	if (!msgArray.isEmpty())
	{
		QJsonArray innerArray = msgArray[0].toArray();
		if (!innerArray.isEmpty())
		{
			QJsonObject msgObj = innerArray[0].toObject();
			if (msgObj.contains("txt"))
			{
				QString userAvatar = QString("{\"name\":\"%1\",\"avatar\":\"%2\"}").arg(m_strUserName).arg(m_strUserImg);
				QString strMsg = msgObj["txt"].toString();
				QString jsStr = QString("app.newPush('%1', %2);").arg(strMsg).arg(userAvatar);
				m_pViewChat->page()->runJavaScript(jsStr);
			}

		}
	}
}

//将获取响应消息操作添加到消息队列 并将输入框中的消息添加到聊天页面
void CMsgPane::OnBtnSendClicked()
{
	/*
	//创建消息项
    TMsgItem item;
	// 构造API请求URL：使用青云客智能聊天API，将输入框内容作为消息参数
    item.strUrl = QString("http://api.qingyunke.com/api.php?key=free&appid=0&msg=%1").arg(m_textEdit->toPlainText());
	// 将当前对象指针保存到消息项中（用于后续回调）
	item.pObj = this;
	//消息项推入消息队列
    MSGQUEUE->Push(item);
	*/
	int nReceiverId = m_nCurrentChatId;
	int nSenderId = gCurrentLoginId;
	QString message = m_textEdit->toPlainText();

	QJsonArray msgArray;
	QJsonArray innerArray;
	
	innerArray.append(QJsonObject{ {"txt",message} });
	msgArray.append(innerArray);

	QJsonObject data
	{
		{"send",nSenderId},
		{"recv",nReceiverId},
		{"what","msg"},
		{"msg",msgArray}
	};

	QJsonObject request
	{
		{"type","1"},
		{"data",data}
	};

	ComSocket::getInstance()->sendJsonRequest(request);

	// 构建JavaScript执行字符串：调用网页中的addMsg函数显示发送的消息
	// 注意：这里直接将原始文本插入JS字符串，存在安全风险（应转义特殊字符
	QString userAvatar = QString("{\"name\":\"%1\",\"avatar\":\"%2\"}").arg(gCurrentLoginName).arg(gCurrentLoginImg);
	QString jsStr = QString("app.newSend('%1', %2);").arg(message).arg(userAvatar);
    m_pViewChat->page()->runJavaScript(jsStr);
    m_textEdit->clear();
}
//接收到消息 青云客 api 返回结果：{"result":0,"content":"消息内容"}
void CMsgPane::OnRecvMsg(QByteArray strMsg, QObject *obj)
{
    if (obj != this)
        return;

    QJsonParseError jsonError;
    QJsonDocument doucment = QJsonDocument::fromJson(strMsg, &jsonError);
    if (!doucment.isNull() && (jsonError.error == QJsonParseError::NoError))
    {
        if (doucment.isObject())
        {
            QJsonObject object = doucment.object();
            if (object.contains("content"))
            {
                QJsonValue value = object.value("content");
                if (value.isString())
                {
                    QString strName = value.toString();

                    QString jsStr = QString(QString("addRecvMsg(\"%1\")").arg(strName));
                    m_pViewChat->page()->runJavaScript(jsStr);
                }
            }
        }
    }
}
//刷新聊天界面
void CMsgPane::OnFriendChange(TUserInfo tUserInfo)
{
    QString jsStr = QString(QString("clear()"));
    m_pViewChat->page()->runJavaScript(jsStr);
	//切换当前聊天者id
	m_nCurrentChatId = tUserInfo.nId;
	m_strUserName = tUserInfo.strName;
	m_strUserImg = tUserInfo.strAvatar;
}

void CMsgPane::onReceivedData(const QByteArray & data)
{
	qDebug() << "onReceivedData" << data << endl;
	QJsonDocument doc = QJsonDocument::fromJson(data);
	if (doc.isNull() || !doc.isObject())
	{
		qDebug() << "illeagel json" << endl;
		return;
	}
	QJsonObject msg = doc.object();
	QString strType = msg["type"].toString();

	qDebug() << "type = " << strType << endl;
	if (strType == "1")
	{
		handleChatMessage(msg);
	}
	else
	{
		qDebug() << "type error" << endl;
	}
}
