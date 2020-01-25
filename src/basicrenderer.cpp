/*
 * This file is part of BeamerPresenter.
 * Copyright (C) 2020  stiglers-eponym

 * BeamerPresenter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * BeamerPresenter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with BeamerPresenter. If not, see <https://www.gnu.org/licenses/>.
 */

#include "basicrenderer.h"

BasicRenderer::BasicRenderer(PdfDoc const* doc, PagePart const part, QObject* parent)
    : QObject(parent),
      pdf(doc),
      pagePart(part),
      cacheThread(new CacheThread(this, this))
{
    connect(cacheThread, &CacheThread::finished, this, &BasicRenderer::receiveBytes);
}

QPixmap const BasicRenderer::renderPixmap(int const page) const
{
    // This should only be called from within CacheThread, BasicRenderer and CacheMap!
    Poppler::Page const* cachePage = pdf->getPage(page);
    QImage image = cachePage->renderToImage(72*resolution, 72*resolution);
    if (pagePart == FullPage)
        return QPixmap::fromImage(image);
    else if (pagePart == LeftHalf)
        return QPixmap::fromImage(image.copy(0, 0, image.width()/2, image.height()));
    else
        return QPixmap::fromImage(image.copy(image.width()/2, 0, image.width()/2, image.height()));
}

QString const BasicRenderer::getRenderCommand(int const page) const
{
    if (renderCommand.isEmpty())
        return renderCommand;
    QString command = renderCommand;
    command.replace("%file", pdf->getPath());
    command.replace("%page", QString::number(page+1));
    if (pagePart==FullPage)
        command.replace("%width", QString::number(int(resolution*pdf->getPageSize(page).width()+0.5)));
    else
        command.replace("%width", QString::number(int(2*resolution*pdf->getPageSize(page).width()+0.5)));
    command.replace("%height", QString::number(int(resolution*pdf->getPageSize(page).height()+0.5)));
    return command;
}
