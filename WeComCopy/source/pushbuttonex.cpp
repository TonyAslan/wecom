#include "pushbuttonex.h"
#include <QMouseEvent>
#include <QPainter>

CPushButtonEx::CPushButtonEx(QWidget *parent) : QPushButton(parent)
{
	m_varData = 0;        // 用于存储自定义数据的变量
	m_bExArea = false;    // 标记是否启用了特殊区域点击检测
	m_fRadio = 0.0;       // 宽高比（如果设置了固定宽高比）
	m_bRadio = false;     // 标记是否启用固定宽高比

    setCursor(Qt::PointingHandCursor); //设置鼠标样式
}

CPushButtonEx::~CPushButtonEx()
{

}

void CPushButtonEx::mouseReleaseEvent( QMouseEvent *e )
{
	// 如果未启用特殊区域检测，则使用默认处理
    if (!m_bExArea)
        return QPushButton::mouseReleaseEvent(e);
	// 定义特殊区域：右下角1/4区域
    QRect rcEx = rect();
    rcEx.setTopLeft(QPoint(rcEx.bottom() / 4 * 3, rcEx.right() / 4 * 3));
	// 根据点击位置发射不同信号
    if (!rcEx.contains(e->pos()))
        emit SignalClicked();
    else
        emit SignalClickedExArea();

    return QPushButton::mouseReleaseEvent(e);
}

void CPushButtonEx::resizeEvent(QResizeEvent *event)
{
    if (m_bRadio)
    {
        setFixedHeight(width() / m_fRadio);

        this->setStyleSheet(QString("QPushButton{font: bold %1px;} QPushButton:hover{font: bold %2px;} QPushButton:pressed{font: bold %3px;}")
                           .arg(this->height() / 2).arg(this->height() / 2 + 6).arg(this->height() / 2 + 2));
    }
}
