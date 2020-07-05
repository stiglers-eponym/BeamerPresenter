#ifndef ABSTRACTRENDERER_H
#define ABSTRACTRENDERER_H

#include "src/enumerates.h"
#include <QPixmap>
#include <QByteArray>

class AbstractRenderer
{
public:
    AbstractRenderer();
    /// Render page to a QPixmap.
    virtual const QPixmap renderPixmap(const int page, const qreal resolution) const;
    /// Render page to PNG image in a QByteArray.
    virtual const QByteArray * renderPng(const int page, const qreal resolution) const;
};

#endif // ABSTRACTRENDERER_H
