#include "notificationpane.h"
#include <QGuiApplication>
#include <QScreen>
#include <QDebug>
#include <QPropertyAnimation>
#include <QDateTime>
#include <QPainter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDesktopWidget>
#include <QApplication>
#include "pushbuttonex.h"
#include "iconhelper.h"

//通知面板

NotificationPane::NotificationPane(QWidget *parent) : QWidget(parent)
{
    m_pTimerHide = NULL; //自动隐藏定时器
    m_btnClose = NULL;   //关闭按钮
    m_nIndex = 0;        //当前通知索引

    setAttribute(Qt::WA_StyledBackground);  // 禁止父窗口样式影响子控件样式
    setWindowFlags(windowFlags() | Qt::ToolTip | Qt::FramelessWindowHint | Qt::WindowMinMaxButtonsHint);
    setProperty("form", "iconfontpane");
    setAttribute(Qt::WA_TranslucentBackground, true);
	//初始化隐藏定时器
    m_pTimerHide = new QTimer(this);
    connect(m_pTimerHide, SIGNAL(timeout()), this, SLOT(OnTimerHideTimeOut()));
	//创建关闭按钮
    m_btnClose = new CPushButtonEx(this);
    m_btnClose->setFixedSize(40, 40);
    m_btnClose->setProperty("white_bk", "true");
	//设置按钮图标
    IconHelper::SetIcon(m_btnClose, QChar(0xe64f));
    connect(m_btnClose, SIGNAL(clicked()), this, SLOT(OnTimerHideTimeOut()));
	//构建顶部布局（关闭按钮+弹簧
    QHBoxLayout *layoutTop = new QHBoxLayout();
    layoutTop->addStretch();
    layoutTop->addWidget(m_btnClose);
    layoutTop->setSpacing(0);
    layoutTop->setContentsMargins(0, 10, 20, 0);
	// 主垂直布局
    QVBoxLayout *layoutMain = new QVBoxLayout();
    layoutMain->addLayout(layoutTop);
    layoutMain->addStretch();
    layoutMain->setSpacing(0);
    layoutMain->setMargin(0);
    setLayout(layoutMain);

    setFixedSize(400, 150);
}
// \param msec 自动关闭时间（毫秒）
void NotificationPane::Start(int msec)
{
    m_pTimerHide->stop();

    if (m_tMsgInfo.bAutoClose)
    {
        m_pTimerHide->setInterval(msec);
        m_pTimerHide->start();
    }

    update();
}

void NotificationPane::Notice(NotificationPane::TMsgInfo tMsgInfo)
{
    m_tMsgInfo = tMsgInfo;
}
// \brief 鼠标进入事件 - 停止自动关闭计时
void NotificationPane::enterEvent(QEvent *event)
{
    m_pTimerHide->stop();
}
// \brief 鼠标离开事件 - 重新启动自动关闭计时
void NotificationPane::leaveEvent(QEvent *event)
{
    m_pTimerHide->start();
}

void NotificationPane::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);         // 创建画家对象
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing, true); // 抗锯齿和使用平滑转换算法
	// 绘制白色背景矩形（带10像素边距）
    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    path.addRect(10, 10, this->width() - 20, this->height() - 20);
    painter.fillPath(path, QBrush(Qt::white));
	// 绘制多层阴影效果（模拟立体感）
    QColor color(92, 93, 95, 50);
    int arr[10] = { 150, 120, 80, 50, 40, 30, 20, 10, 5, 5};// 透明度递减序列

    for (int i = 0; i < 10; i++)
    {
         QPainterPath path;
         path.setFillRule(Qt::WindingFill);
         if(i == 5)
             path.addRect(10-i-1, 10-i-1, this->width()-(10-i)*2, this->height()-(10-i)*2);
         else
            path.addRoundedRect(10-i-1, 10-i-1, this->width()-(10-i)*2, this->height()-(10-i)*2,2,2);

         color.setAlpha(arr[i]);
         painter.setPen(color);
         painter.drawPath(path);
    }
	// 绘制标题文本
    QFont ft = painter.font();
    QRect rcClient = rect();

    painter.save();//保存painter当前的状态
    QRect rcTitle(rcClient);
    rcTitle.setTop(rcClient.top() + 17);
    rcTitle.setLeft(rcClient.left() + 25);
    rcTitle.setBottom(rcTitle.top() + 27);
    rcTitle.setRight(rcClient.right() - 25);

    QFont ftTemp = ft;
    ftTemp.setPointSize(12);
    ftTemp.setBold(false);
    painter.setFont(ftTemp);

    painter.setPen(QColor("#262626"));
    painter.drawText(rcTitle, Qt::AlignLeft | Qt::AlignVCenter, m_tMsgInfo.strTitle);
    painter.setFont(ft);
    painter.restore();//painter 恢复之前的状态

    painter.save();
    QRect rcContent(rcClient);
    rcContent.setTop(rcTitle.bottom() + 17);
    rcContent.setLeft(rcClient.left() + 25);
    rcContent.setBottom(rcClient.bottom() - 17);
    rcContent.setRight(rcClient.right() - 25);

    ftTemp.setPointSize(10);
    ftTemp.setBold(false);
    painter.setFont(ftTemp);

    painter.setPen(QColor("#262626"));
    painter.drawText(rcContent, Qt::AlignLeft | Qt::TextWordWrap, m_tMsgInfo.strContent);
    painter.setFont(ft);
    painter.restore();
}
// \brief 定时器超时处理 - 隐藏通知
void NotificationPane::OnTimerHideTimeOut()
{
    m_pTimerHide->stop();
    hide();
    emit SignalHide(m_nIndex);
}


////////////////////////////////////////////////////////////////////
//单例管理类实现

NotificationMgr* NotificationMgr::m_pNotificationMgr = NULL;
NotificationMgr::NotificationMgr(QWidget *parent)
{
	//默认单通知模式
    m_bSingleMode = true;

    m_listNotification.clear();

    // 默认创建10个
    for (int i = 0; i < MAX_NOTICE; i++)
    {
        NotificationPane *pNotice = new NotificationPane(parent);
		//初始隐藏
        pNotice->hide();
        pNotice->SetId(i);

        connect(pNotice, SIGNAL(SignalHide(int)), this, SLOT(OnItemHide(int)));
		//初始化通知项
        TNoteItem tItem;
        tItem.pNotificationPane = pNotice;
		tItem.bNew = false;
		tItem.bShow = false;
		tItem.nInterval = 3000;
        m_listNotification << tItem;
    }
}
// 创建单例实例
void NotificationMgr::Init(QWidget *parent)
{
    if (NULL == m_pNotificationMgr)
    {
        m_pNotificationMgr = new NotificationMgr(parent);
    }
}
// \brief 获取单例实例
NotificationMgr *NotificationMgr::GetInstance()
{
    return m_pNotificationMgr;
}
//销毁
void NotificationMgr::ExitInstance()
{
    if (m_pNotificationMgr != NULL)
    {
        delete m_pNotificationMgr;
        m_pNotificationMgr = NULL;
    }
}

bool NotificationMgr::CompareData(const TNoteItem &tItem1, const TNoteItem &tItem2)
{
    if (tItem1.currentTime < tItem2.currentTime)
    {
        return true;
    }

    return false;
}
// \brief 显示新通知
// \param strContent 通知内容
// \param strTitle 通知标题（可选）
// \param bAutoClose 是否自动关闭
// \param dwFlag 显示位置标志（如AlignTop|AlignRight）
void NotificationMgr::Notice(QString strContent, QString strTitle, bool bAutoClose, uint dwFlag)
{
	// 计算通知显示位置
    int nPos = 10;
    int nStartHeight = 10;
	// 查找可用通知面板
    for (int i = 0; i < m_listNotification.size(); i++)
    {
		//单通知模式
        if (m_bSingleMode) 
        {
			//重用第一个面板
			NotificationPane* pPane = m_listNotification[0].pNotificationPane;
			pPane->hide(); //先隐藏旧通知
			//更新通知项状态
            m_listNotification[0].bShow = true;
            m_listNotification[0].bNew = true;
            m_listNotification[0].pNotificationPane->hide();
            m_listNotification[0].dwFlag = dwFlag;
			//设置新消息
            NotificationPane::TMsgInfo tMsgInfo;
            tMsgInfo.bAutoClose = bAutoClose;
            tMsgInfo.strContent = strContent;
            tMsgInfo.strTitle = strTitle;
            m_listNotification[0].pNotificationPane->Notice(tMsgInfo);
            break;
        }
        else
        {
			//多通知模式
            TNoteItem tItem = m_listNotification.at(i);
            NotificationPane *pNotice = tItem.pNotificationPane;

            if (!tItem.bShow)//找到未使用的面板
            {
				//更新通知项
                m_listNotification[i].bShow = true;
                m_listNotification[i].bNew = true;
                m_listNotification[i].currentTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
                m_listNotification[i].dwFlag = dwFlag;
				//设置消息
                NotificationPane::TMsgInfo tMsgInfo;
                tMsgInfo.bAutoClose = bAutoClose;
                tMsgInfo.strContent = strContent;
                tMsgInfo.strTitle = strTitle;
                pNotice->Notice(tMsgInfo);

                break;
            }
            else //已显示面板 累加高度
            {
                QSize size = pNotice->size();
                nStartHeight += size.height();
                nStartHeight += nPos;
            }
        }
    }

    ShowAll(); //更新所有通知显示
}

void NotificationMgr::ShowAll()
{
	// 获取屏幕可用区域
    QDesktopWidget *desktopWidget = QApplication::desktop();
    QRect screenRect = desktopWidget->availableGeometry();

    int dWidth = screenRect.width();
    int dHeight = screenRect.height();

    int nPos = 10;
    int nStartHeight = 10;
	//遍历所有通知项
    for (int i = 0; i < m_listNotification.size(); i++)
    {
        TNoteItem tItem = m_listNotification.at(i);
		if (!tItem.bShow) continue;
        NotificationPane *pNotice = tItem.pNotificationPane;
        if (tItem.bShow)
        {
           pNotice->show();

           const int nSpace = 10;
           QSize size = pNotice->size();
           QRect rcStart, rcEnd;
           QPoint ptStart, ptEnd;
           if (tItem.dwFlag & AlignLeft)
           {
               ptStart.setX(-size.width());
               ptEnd.setX(nSpace);
           }
           if (tItem.dwFlag & AlignTop)
           {
               ptStart.setY(nStartHeight);
               ptEnd.setY(nStartHeight);
           }
           if (tItem.dwFlag & AlignRight)
           {
               ptStart.setX(dWidth);
               ptEnd.setX(dWidth - size.width() - nSpace);
           }
           if (tItem.dwFlag & AlignBottom)
           {
               ptStart.setY(dHeight - size.height() - nStartHeight);
               ptEnd.setY(dHeight - size.height() - nStartHeight);
           }

           rcStart = QRect(ptStart.x(), ptStart.y(), size.width(), size.height());
           rcEnd = QRect(ptEnd.x(), ptEnd.y(), size.width(), size.height());

		   //新通知显示动画
           if (tItem.bNew)
           {	// 创建一个属性动画对象，用于改变通知窗口的几何位置
				//QPropertyAnimation: Qt 的属性动画类，用于平滑改变对象的属性值
               QPropertyAnimation *animation = new QPropertyAnimation(pNotice, "geometry");
               animation->setDuration(200);
               animation->setStartValue(rcStart);
               animation->setEndValue(rcEnd);
               animation->start();

               pNotice->Start(tItem.nInterval);
               m_listNotification[i].bNew = false;
           }
           else
           {
               QSize size = pNotice->size();
               pNotice->move(dWidth - size.width() - nSpace, nStartHeight);
           }

           nStartHeight += size.height();
           nStartHeight += nPos;

           if (m_bSingleMode)
           {
               break;
           }
        }
    }
}

void NotificationMgr::OnItemHide(int nIndex)
{
    m_listNotification[nIndex].bShow = false;
    ShowAll();
}
