#ifndef MAIN_H
#define MAIN_H


class Win
    : public TDEMainWindow
{
    TQ_OBJECT
    KParts::Part* p;
public:
    Win();
public slots:
    void pythonExited();
    void forked();
};

#endif
