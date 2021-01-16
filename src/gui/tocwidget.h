#ifndef TOCWIDGET_H
#define TOCWIDGET_H

#include <QDebug>
#include <QWidget>
#include <QBoxLayout>
#include <QLabel>
#include "src/rendering/pdfdocument.h"
#include "src/preferences.h"
#include "src/gui/tocbutton.h"

class TOCwidget : public QWidget
{
    Q_OBJECT

    QVector<TOCbutton*> buttons;

public:
    explicit TOCwidget(QWidget *parent = nullptr) : QWidget(parent) {}

    ~TOCwidget()
    {qDeleteAll(buttons);}

    void generateTOC(const PdfDocument *document = nullptr);

    /// Actually this is nonsense, but currently the layout only works with
    /// this option set.
    bool hasHeightForWidth() const override
    {return true;}

public slots:
    bool event(QEvent *event) override;
    void expandTo(const int page);

signals:
    void sendNavigationSignal(const int page);

};

#endif // TOCWIDGET_H
