#ifndef INPUTFILTER_H
#define INPUTFILTER_H

#include <QObject>

class InputFilter : public QObject
{
    Q_OBJECT

private:
    bool shiftHeld_;

public:
    InputFilter();

signals:
    void sendMessage();

protected:
    bool eventFilter(QObject* obj, QEvent* event);

};

#endif // INPUTFILTER_H
