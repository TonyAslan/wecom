#include "appinit.h"
#include "qmutex.h"
#include "qapplication.h"
#include "qevent.h"
#include "qwidget.h"

AppInit *AppInit::self = 0;
AppInit *AppInit::Instance()
{
    if (!self) {
        QMutex mutex;
        QMutexLocker locker(&mutex);
        if (!self) {
            self = new AppInit;
        }
    }

    return self;
}

AppInit::AppInit(QObject *parent) : QObject(parent)
{
}
//通过安装全局事件过滤器来实现应用程序所有窗体的移动
//基本逻辑是根据窗体的 canMove 属性(WeComWnd 窗体)，判断是否允许鼠标拖
//动来移动窗体
bool AppInit::eventFilter(QObject *obj, QEvent *evt)
{
    QWidget *w = (QWidget *)obj;
    if (!w->property("canMove").toBool()) {
        return QObject::eventFilter(obj, evt);
    }

    static QPoint mousePoint;
    static bool mousePressed = false;
	/*为了避免非鼠标事件导致崩溃或行为异常，应该在处理之前做类型检查
		如果对象不是目标类型，dynamic_cast 转换结果会返回 nullptr*/
    QMouseEvent *event = static_cast<QMouseEvent *>(evt);
	if (!event)
	{
		return QObject::eventFilter(obj, evt);
	}

	if (event->type() == QEvent::MouseButtonPress) {
        if (event->button() == Qt::LeftButton) {
            mousePressed = true;
            mousePoint = event->globalPos() - w->pos();
            return true;
        }
    } else if (event->type() == QEvent::MouseButtonRelease) {
        mousePressed = false;
        return true;
    } else if (event->type() == QEvent::MouseMove) {
        if (mousePressed && (event->buttons() && Qt::LeftButton)) {
            w->move(event->globalPos() - mousePoint);
            return true;
        }
    }

    return QObject::eventFilter(obj, evt);
}

void AppInit::start()
{
    qApp->installEventFilter(this);
}
