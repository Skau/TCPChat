#include "inputfilter.h"
#include <QEvent>
#include <QKeyEvent>

InputFilter::InputFilter() : shiftHeld_(false)
{

}

bool InputFilter::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() == QEvent::KeyPress)
    {
        auto e = static_cast<QKeyEvent*>(event);

        if(e->key() == Qt::Key_Shift)
        {
            shiftHeld_ = true;
            return true;

        }
        else if(e->key() == Qt::Key_Return)
        {
            if(shiftHeld_)
            {
                return QObject::eventFilter(obj, event);
            }

            emit sendMessage();
            return true;
        }
    }
    else if(event->type() == QEvent::KeyRelease)
    {
        auto e = static_cast<QKeyEvent*>(event);

        if(e->key() == Qt::Key_Shift)
        {
            shiftHeld_ = false;

            return true;
        }
    }

    return QObject::eventFilter(obj, event);
}
