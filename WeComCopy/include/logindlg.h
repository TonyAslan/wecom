#ifndef CLOGINDLG_H
#define CLOGINDLG_H

#include <QWidget>
#include "basedlg.h"

typedef struct FRIENDINFO
{
	int id;
	QString name;
	QString part;
	QString email;
	QString img;
	QString sign;
}friendInfo;

class QJsonObject;
class QTcpSocket;
class QLineEdit;
class WeComWnd;
class CLoginDlg : public CBaseDlg
{
    Q_OBJECT
public:
    explicit CLoginDlg(QWidget *parent = 0);

private:
    void CreateAllChildWnd();
    void InitCtrl();
    void InitSolts();
    void Relayout();
	void handleLoginResponse(const QJsonObject& response);
	void handleFriendsDetailResponse(const QJsonObject& response);
	void sendFriendsDetailRequest(const QVector<int>& vFriendIds);

signals:
    //void SignalLoginFinish();

public slots:
    void OnLoginFinish();
    void OnLoging();
    void OnBtnCloseClicked();

private slots:
	void onSendLoginRequest();
	void onReadyRead(const QByteArray& data);

private:
	
	//登录用户的基本信息
	int m_userId = -1;
	QString m_userName;
	QString m_userPart;
	QString m_userEmail;
	QString m_userImg;
	//好友Id
	QVector<int> m_vFriendsId;
	//Id，详情Map
	QMap<int, friendInfo> m_mapFriends;

    QLabel *m_labCompany;
    QLabel *m_labUserIcon;
    QLabel *m_labUserName;
    QLabel *m_labTip;
    CPushButtonEx *m_btnCfg;
    QLabel *m_labQRcode;

	QLabel *m_labUserId;
	QLabel *m_labUserPassword;
	QLineEdit *m_userIdEdit;
	QLineEdit *m_passwordEdit;
	QPushButton *m_loginBtn;

	//QTcpSocket* m_socket; //通信的端口
	//主窗口
	WeComWnd* m_weComWnd;
};

#endif // CLOGINDLG_H
