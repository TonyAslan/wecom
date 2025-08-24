#include "incofontpane.h"
#include <QVBoxLayout>
#include "pushbuttonex.h"
#include "iconhelper.h"
#include <QFile>
#include <QDebug>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonArray>
#include <QApplication>
#include <QClipboard>
#include "public.h"
#include "tipwidget.h"
#include "notificationpane.h"


CIncoFontPane::CIncoFontPane(QWidget *parent) : QWidget(parent)
{
   m_pScrollArea = NULL;
   m_widgetContent = NULL;

   CreateAllChildWnd();
   InitCtrl();
   InitSolts();
   Relayout();

   ParseJsonFile();
}
//字体符号页按钮点击槽
void CIncoFontPane::OnBtnClicked()
{
	//获取信号发出的对象
    CPushButtonEx *btnSender = (CPushButtonEx *)sender();
	//QClipboard 是 Qt 框架中用于访问系统剪贴板的类，它提供了在不同应用程序之间或同一应用程序内复制和粘贴数据的标准机制
    QClipboard *clipboard = QApplication::clipboard();   //获取系统剪贴板指针

    clipboard->setText(QString("0x%1").arg(QString::number(btnSender->Data().toInt(), 16)));
	//显示提示窗口
    AUTOTIP->SetMesseage(QString("0x%1 复制成功").arg(QString::number(btnSender->Data().toInt(), 16)));
}
//创建所有的子窗口
void CIncoFontPane::CreateAllChildWnd()
{
#define NEW_OBJECT(pObj, TYPE) \
    if (NULL == pObj) { pObj = new TYPE(this); }

    NEW_OBJECT(m_pScrollArea, QScrollArea);
    NEW_OBJECT(m_widgetContent, QWidget);
}

void CIncoFontPane::InitCtrl()
{
    m_pScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);//隐藏横向滚动条
    m_pScrollArea->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    m_pScrollArea->setWidgetResizable(true);
    m_pScrollArea->setWidget(m_widgetContent);
    m_pScrollArea->setStyleSheet(".QScrollArea{border-style:none;}");

    m_widgetContent->setAttribute(Qt::WA_StyledBackground);
    m_widgetContent->setProperty("form", "iconfontpane");
}

void CIncoFontPane::InitSolts()
{

}

void CIncoFontPane::Relayout()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_pScrollArea);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    setLayout(mainLayout);
}

//解析json文件 添加图标按钮
void CIncoFontPane::ParseJsonFile()
{
    int nRow = 0;
    int nCol = 0;
	//widget里创建一个网格布局
    QGridLayout *layoutMain = new QGridLayout(m_widgetContent);

    QFile file(":/qss/res/WeComCopy.json");
    if (file.open(QFile::ReadOnly))
    {
        QByteArray json = file.readAll();
        file.close();

        QJsonParseError jsonError;
		// 将字节数组解析为JSON文档
        QJsonDocument doucment = QJsonDocument::fromJson(json, &jsonError);
        if (!doucment.isNull() && (jsonError.error == QJsonParseError::NoError))
        {
			// 验证文档根元素是否为JSON对象
            if (doucment.isObject())
            {
                QJsonObject object = doucment.object();
				// 检查根对象是否包含"glyphs"键
                if(object.contains("glyphs"))
                {
					// 获取"glyphs"对应的值
                    QJsonValue glyphs = object.value("glyphs");
					// 确认该值是数组类型
                    if (glyphs.isArray())
                    {
						// 将值转换为JSON数组
                        QJsonArray arrIcon = glyphs.toArray();
                        for (int i = 0; i < arrIcon.size(); i++)
                        {
							// 获取当前数组元素
                            QJsonValue jsonIcon = arrIcon.at(i);
							// 确认元素是对象类型
                            if (jsonIcon.isObject())
                            {
								// 转换为JSON对象
                                QJsonObject objectIcon = jsonIcon.toObject();
								//获取键值并移除键
                                QChar cUnicode = objectIcon.take("unicode_decimal").toInt();
                                QString strUnicodeHex = objectIcon.take("unicode").toString();

                                CPushButtonEx *btn = new CPushButtonEx(m_widgetContent);
                                btn->SetAspectRatio(2.2);
                                IconHelper::SetIcon(btn, cUnicode, 30);
                                btn->setProperty("IconfontBtn", "true");   // iconfont 单独为一类别
                                btn->SetData(cUnicode);
                                btn->setToolTip(QString("0x%1").arg(strUnicodeHex));
                                layoutMain->addWidget(btn, nRow, nCol);
                                connect(btn, SIGNAL(clicked()), this, SLOT(OnBtnClicked()));

                                nCol++;

                                if (nCol > 4)
                                {
                                    nRow++;
                                    nCol = 0;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
