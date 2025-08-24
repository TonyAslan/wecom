#ifndef CMSGPANE_H
#define CMSGPANE_H

#include <QWidget>
#include <QWebEngineView>
#include <QTextEdit>
#include <QLabel>
#include "public.h"
#include "logindlg.h"
#include <QWebEngineUrlRequestInterceptor>
#include <QDir>

class CFriendsList;
class CChatTopToolbar;
class CPushButtonEx;

class AvatarRequestInterceptor : public QWebEngineUrlRequestInterceptor {
public:
	void interceptRequest(QWebEngineUrlRequestInfo& info) override {
		if (info.requestUrl().toString().contains("qrc:/html/html/img/avatar/"))
		{ // 检查是否为头像文件请求
			QString relativePath = info.requestUrl().toString();
			//qrc:/html/html/img/avatar/usricon.jpeg
			//QString prefix = QDir::currentPath();
			QString absolutePath =/* prefix + */relativePath.replace("qrc:/html/html/img/avatar/",
				QString());
			absolutePath += ".png";
			info.redirect(QUrl::fromLocalFile(absolutePath));
			qDebug() << "img absolutePath = " << absolutePath << endl;
		}
	}
};
class CMsgPane : public QWidget
{
    Q_OBJECT
public:
    explicit CMsgPane(QWidget *parent = 0);
	void setFriendsList(const QVector<FRIENDINFO>& vFriendsInfo);

private:
    void CreateAllChildWnd();
    void InitCtrl();
    void InitSolts();
    void Relayout();
    void SendGetRequest(QString strMsg);

    bool eventFilter(QObject *obj, QEvent *e);

	void handleChatMessage(const QJsonObject& message);

public slots:
    void OnBtnSendClicked();
    void OnRecvMsg(QByteArray strMsg, QObject *obj);
    void OnFriendChange(TUserInfo tUserInfo);
	void onReceivedData(const QByteArray& data);

private:
    CFriendsList *m_pFriendsList;
    CChatTopToolbar *m_pTopToolbar;
    QWebEngineView *m_pViewChat;
    QTextEdit *m_textEdit;
    CPushButtonEx *m_btnSend;
    QLabel *m_labSeparatorLine;
	int m_nCurrentChatId = -1;
	QString m_strUserImg = "";
	QString m_strUserName = "";
};

#endif // CMSGPANE_H
