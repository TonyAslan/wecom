#include "msgqueue.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>


CMsgQueue* CMsgQueue::m_pMsgQueue = NULL;
CMsgQueue::CMsgQueue( QObject *parent ) : QThread(parent)
{

}

CMsgQueue::~CMsgQueue()
{
    Clear();
}
/**
 * @fn CMsgQueue* CMsgQueue::GetInstance()
 * @brief 获取消息队列单例
 * @return 单例对象指针
 * @note 采用懒汉式单例模式，首次调用时创建实例并启动线程
 */
CMsgQueue *CMsgQueue::GetInstance()
{
    if (NULL == m_pMsgQueue)
    {
        m_pMsgQueue = new CMsgQueue();
        m_pMsgQueue->start();
    }

    return m_pMsgQueue;
}
/**
 * @fn void CMsgQueue::ExitInstance()
 * @brief 释放单例实例
 * @note 安全停止线程并销毁实例
 */
void CMsgQueue::ExitInstance()
{
    if (m_pMsgQueue != NULL)
    {
        m_pMsgQueue->requestInterruption(); //请求中断线程
        m_pMsgQueue->wait();  //等待线程结束

        delete m_pMsgQueue;
        m_pMsgQueue = NULL;
    }
}
/**
 * @fn bool CMsgQueue::Push(TMsgItem item)
 * @brief 向消息队列添加新消息项
 * @param item 消息项（结构体，应包含strUrl和pObj等成员）
 * @return 添加结果（恒为true）
 * @note 使用互斥锁保证线程安全
 */
bool CMsgQueue::Push(TMsgItem item)
{
    QMutexLocker locker(&m_mutex);

    m_listMsg.append(item); //添加到队列尾部

    return true;
}
/**
 * @fn void CMsgQueue::run()
 * @brief 消息处理线程主函数
 * @note 循环处理消息队列中的请求，支持线程中断
 */
void CMsgQueue::run()
{
	//在没有收到中断请求的情况下不断运行
    while (!isInterruptionRequested() /*TRUE*/)
    {
        bool hasMsg = false;

        {
            QMutexLocker locker(&m_mutex);

            if (!m_listMsg.isEmpty())
            {
                hasMsg = true;
				// 创建HTTP管理器和请求对象
                QNetworkAccessManager *pHttpMgr = new QNetworkAccessManager();
                QNetworkRequest requestInfo;
                requestInfo.setUrl(QUrl(m_listMsg.at(0).strUrl));
				//创建事件循环 实现同步请求
				/*
				使用 QEventLoop 阻塞当前线程直到网络请求完成
				使用事件循环不会阻止其他事件的处理，而使用 while 循环
				可能会导致代码阻塞，进而阻止其他事件的正常处理
				*/
                QEventLoop eventLoop;
				//发起Http get请求
                QNetworkReply *reply =  pHttpMgr->get(requestInfo);
                connect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
                eventLoop.exec();       //阻塞等待请求完成

                if (reply->error() == QNetworkReply::NoError)
                {
                    qDebug() << "request protobufHttp NoError";
                }
                else
                {
                    qDebug()<<"request protobufHttp handle errors here";
					//获取HTTP状态码
                    QVariant statusCodeV = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
                    //statusCodeV是HTTP服务器的相应码，reply->error()是Qt定义的错误码，可以参考QT的文档
                    qDebug( "request protobufHttp found error ....code: %d %d\n", statusCodeV.toInt(), (int)reply->error());
                    qDebug(qPrintable(reply->errorString()));
                }

                //读取响应数据 并发送信号
                QByteArray responseByte = reply->readAll();
				//携带接收对象的指针
                emit SignalRecvMsg(responseByte, m_listMsg.at(0).pObj);
                //清理当前消息项
				m_listMsg.removeFirst();
				delete reply;
				delete pHttpMgr;
            }
        }

		//无消息时休眠降低CPU占用
        if (!hasMsg)
        {
            msleep(200);
        }
    }
}
/**
 * @fn void CMsgQueue::Clear()
 * @brief 清空消息队列
 * @note 使用互斥锁保证线程安全
 */

void CMsgQueue::Clear()
{
    QMutexLocker locker(&m_mutex);

    if (!m_listMsg.isEmpty())
    {
        m_listMsg.clear();
    }
}
