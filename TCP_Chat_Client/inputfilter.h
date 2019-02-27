#ifndef INPUTFILTER_H
#define INPUTFILTER_H

#include <QObject>

class InputFilter : public QObject
{
    Q_OBJECT

public:
    InputFilter();

signals:
    void sendMessage();

protected:
    bool eventFilter(QObject* obj, QEvent* event);

private:
     bool shiftHeld_;
};

#endif // INPUTFILTER_H
