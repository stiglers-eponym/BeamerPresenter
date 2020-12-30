#include "flexlayout.h"

FlexLayout::~FlexLayout()
{
    for (const auto item : qAsConst(items))
        delete item;
    items.clear();
}

QSize FlexLayout::sizeHint() const
{
    // Only aspect ratios are relevant.
    QSize hint;
    switch (direction)
    {
    case QBoxLayout::LeftToRight:
    case QBoxLayout::RightToLeft:
    {
        int width = 0;
        for (const auto child : qAsConst(items))
        {
            hint = child->widget()->sizeHint();
            if (child->widget()->hasHeightForWidth())
                width += hint.height() ? 1024 * hint.width() / hint.height() : 0;
            else
                width += hint.width();
        }
        qDebug() << "FlexLayout size hint:" << width << 1024;
        return QSize(width, 1024);
    }
    case QBoxLayout::TopToBottom:
    case QBoxLayout::BottomToTop:
    {
        int height = 0;
        for (const auto child : qAsConst(items))
        {
            hint = child->widget()->sizeHint();
            if (child->widget()->hasHeightForWidth())
                height += hint.width() ? 1024 * hint.height() / hint.width() : 0;
            else
                height += hint.height();
        }
        qDebug() << "FlexLayout size hint:" << 1024 << height;
        return QSize(1024, height);
    }
    }
}

void FlexLayout::setGeometry(const QRect &rect)
{
    qDebug() << "FlexLayout setGeometry" << rect;
    QLayout::setGeometry(rect);
    QVector<qreal> aspects(items.size());
    QVector<int> minsizes(items.size());
    QSize hint;
    switch (direction)
    {
    case QBoxLayout::LeftToRight:
    case QBoxLayout::RightToLeft:
    {
        int width = 0;
        qreal totalwidth = 0.;
        for (int i=0; i<items.size(); i++)
        {
            hint = items[i]->widget()->sizeHint();
            if (items[i]->widget()->hasHeightForWidth())
            {
                aspects[i] = hint.width() / qreal(hint.height());
                totalwidth += aspects[i];
            }
            else
            {
                minsizes[i] = hint.width();
                width += hint.width();
            }
        }
        if (rect.width() - width > rect.height() * totalwidth)
        {
            // rect is wider than needed.
            const int margin = (rect.width() - width - rect.height() * totalwidth) / items.size();
            width = margin/2;
            for (int i=0; i<items.size(); i++)
            {
                if (aspects[i] > 0.)
                {
                    items[i]->setGeometry(QRect(width, 0, aspects[i]*rect.height()+.5, rect.height()));
                    width += aspects[i]*rect.height() + .5 + margin;
                }
                else
                {
                    items[i]->setGeometry(QRect(width, 0, minsizes[i], rect.height()));
                    width += minsizes[i] + margin;
                }
            }
        }
        else
        {
            // rect is higher than needed.
            width = 0;
            const qreal scale = (rect.width() - width) / (rect.height() * totalwidth);
            const int shift_y = (1. - scale) * rect.height() / 2;
            for (int i=0; i<items.size(); i++)
            {
                if (aspects[i] > 0.)
                {
                    items[i]->setGeometry(QRect(width, shift_y, scale*aspects[i]*rect.height()+.5, scale*rect.height()));
                    width += scale*aspects[i]*rect.height() + .5;
                }
                else
                {
                    items[i]->setGeometry(QRect(width, 0, minsizes[i], rect.height()));
                    width += minsizes[i];
                }
            }
        }
        break;
    }
    case QBoxLayout::TopToBottom:
    case QBoxLayout::BottomToTop:
    {
        int height = 0;
        qreal totalheight = 0.;
        for (int i=0; i<items.size(); i++)
        {
            hint = items[i]->widget()->sizeHint();
            if (items[i]->widget()->hasHeightForWidth())
            {
                aspects[i] = hint.height() / qreal(hint.width());
                totalheight += aspects[i];
            }
            else
            {
                minsizes[i] = hint.height();
                height += hint.height();
            }
        }
        if (rect.height() - height > rect.width() * totalheight)
        {
            const int margin = (rect.height() - height - rect.width() * totalheight) / items.size();
            height = margin/2;
            // rect is heigher than needed.
            for (int i=0; i<items.size(); i++)
            {
                if (aspects[i] > 0.)
                {
                    items[i]->setGeometry(QRect(0, height, rect.width(), aspects[i]*rect.width()+.5));
                    height += aspects[i]*rect.width() + .5 + margin;
                }
                else
                {
                    items[i]->setGeometry(QRect(0, height, rect.width(), minsizes[i]));
                    height += minsizes[i] + margin;
                }
            }
        }
        else
        {
            // rect is wider than needed.
            height = 0;
            const qreal scale = (rect.height() - height) / (rect.width() * totalheight);
            const int shift_x = (1-scale)/2 * rect.width();
            for (int i=0; i<items.size(); i++)
            {
                if (aspects[i] > 0.)
                {
                    items[i]->setGeometry(QRect(shift_x, height, scale*rect.width(), scale*aspects[i]*rect.width()+.5));
                    height += scale*aspects[i]*rect.width() + .5;
                }
                else
                {
                    items[i]->setGeometry(QRect(0, height, rect.width(), minsizes[i]));
                    height += minsizes[i];
                }
            }
        }
        break;
    }
    }
}
