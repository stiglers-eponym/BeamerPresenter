/*
 * This file is part of BeamerPresenter.
 * Copyright (C) 2019  stiglers-eponym

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

#include "cacheupdatethread.h"

void CacheUpdateThread::run()
{
    if (mode==Renderer::poppler) {
        QPixmap presimg = presLabel->getPixmap(page);
        if (isInterruptionRequested())
            return;
        QPixmap noteimg = noteLabel->getPixmap(page);
        if (isInterruptionRequested())
            return;
        QPixmap smallimg = smallLabel->getPixmap(page);
        if (isInterruptionRequested())
            return;
        QByteArray* pres = new QByteArray();
        {
            QBuffer buffer(pres);
            buffer.open(QIODevice::WriteOnly);
            presimg.save(&buffer, "PNG");
        }
        QByteArray* note = new QByteArray();
        {
            QBuffer buffer(note);
            buffer.open(QIODevice::WriteOnly);
            noteimg.save(&buffer, "PNG");
        }
        QByteArray* small = new QByteArray();
        {
            QBuffer buffer(small);
            buffer.open(QIODevice::WriteOnly);
            smallimg.save(&buffer, "PNG");
        }
        if (isInterruptionRequested()) {
            delete pres;
            delete note;
            delete small;
            return;
        }
        emit resultsReady(pres, note, small, page);
    }
    else if (mode==Renderer::custom) {
        ExternalRenderer* presRenderer  = new ExternalRenderer(page);
        if (!presLabel->cacheContains(page)) {
            QStringList presArguments = QStringList(renderArguments);
            presArguments.replaceInStrings("%file", presFileName);
            presArguments.replaceInStrings("%page", QString::number(page+1));
            if (pagePart==FullPage)
                presArguments.replaceInStrings("%width", QString::number(presLabel->width()));
            else
                presArguments.replaceInStrings("%width", QString::number(2*presLabel->width()));
            presArguments.replaceInStrings("%height", QString::number(presLabel->height()));
            //qDebug() << "Calling external renderer:" << renderCommand << presArguments;
            presRenderer->start(renderCommand, presArguments);
        }

        ExternalRenderer* noteRenderer  = new ExternalRenderer(page);
        if (!noteLabel->cacheContains(page)) {
            QStringList noteArguments = QStringList(renderArguments);
            noteArguments.replaceInStrings("%file", noteFileName);
            noteArguments.replaceInStrings("%page", QString::number(page+1));
            if (pagePart==FullPage)
                noteArguments.replaceInStrings("%width", QString::number(noteLabel->width()));
            else
                noteArguments.replaceInStrings("%width", QString::number(2*noteLabel->width()));
            noteArguments.replaceInStrings("%height", QString::number(noteLabel->height()));
            //qDebug() << "Calling external renderer:" << renderCommand << noteArguments;
            noteRenderer->start(renderCommand, noteArguments);
        }

        ExternalRenderer* smallRenderer  = new ExternalRenderer(page);
        if (!smallLabel->cacheContains(page)) {
            QStringList smallArguments = QStringList(renderArguments);
            smallArguments.replaceInStrings("%file", presFileName);
            smallArguments.replaceInStrings("%page", QString::number(page+1));
            if (pagePart==FullPage)
                smallArguments.replaceInStrings("%width", QString::number(smallLabel->width()));
            else
                smallArguments.replaceInStrings("%width", QString::number(2*smallLabel->width()));
            smallArguments.replaceInStrings("%height", QString::number(smallLabel->height()));
            //qDebug() << "Calling external renderer:" << renderCommand << smallArguments;
            smallRenderer->start(renderCommand, smallArguments);
        }

        QByteArray const* pres = nullptr;
        QByteArray const* note = nullptr;
        QByteArray const* small = nullptr;
        if (!presLabel->cacheContains(page)) {
            if (presRenderer->waitForFinished(60000))
                pres = presRenderer->getBytes();
            else
                presRenderer->kill();
        }
        if (!noteLabel->cacheContains(page)) {
            if (noteRenderer->waitForFinished(60000))
                note = noteRenderer->getBytes();
            else
                noteRenderer->kill();
        }
        if (!smallLabel->cacheContains(page)) {
            if (smallRenderer->waitForFinished(60000))
                small = smallRenderer->getBytes();
            else
                smallRenderer->kill();
        }
        delete presRenderer;
        delete noteRenderer;
        delete smallRenderer;
        if (isInterruptionRequested()) {
            delete pres;
            delete note;
            delete small;
        }
        else
            emit resultsReady(pres, note, small, page);
    }
}

void CacheUpdateThread::setSlideWidgets(const PreviewSlide *pres, const PreviewSlide *note, const PreviewSlide *small)
{
    presLabel = pres;
    noteLabel = note;
    smallLabel = small;
}

void CacheUpdateThread::setCustomRenderer(const QString &renderCommand, const QString &presFileName, const QString &noteFileName, const QStringList &renderArguments, Renderer renderer)
{
    this->renderCommand = renderCommand;
    this->presFileName = presFileName;
    this->noteFileName = noteFileName;
    this->renderArguments = renderArguments;
    mode = renderer;
}
