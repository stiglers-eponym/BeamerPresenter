#include "cacheupdatethread.h"

void CacheUpdateThread::run()
{
    if (mode==Renderer::poppler) {
        QPixmap presimg = presLabel->getPixmap(presPage);
        if (isInterruptionRequested())
            return;
        QPixmap noteimg = noteLabel->getPixmap(notePage);
        if (isInterruptionRequested())
            return;
        QPixmap smallimg = smallLabel->getPixmap(presPage);
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
        emit resultsReady(pres, note, small, presPage->index());
    }
    else if (mode==Renderer::custom) {
        int const index = presPage->index();
        qDebug() << index;
        ExternalRenderer* presRenderer  = new ExternalRenderer(index);
        if (!presLabel->cacheContains(index)) {
            QStringList presArguments = QStringList(renderArguments);
            presArguments.replaceInStrings("%file", presFileName);
            presArguments.replaceInStrings("%page", QString::number(index));
            presArguments.replaceInStrings("%width", QString::number(presLabel->width()));
            presArguments.replaceInStrings("%height", QString::number(presLabel->height()));
            qDebug() << "Calling external renderer:" << renderCommand << presArguments;
            presRenderer->start(renderCommand, presArguments);
        }

        ExternalRenderer* noteRenderer  = new ExternalRenderer(index);
        if (!noteLabel->cacheContains(index)) {
            QStringList noteArguments = QStringList(renderArguments);
            noteArguments.replaceInStrings("%file", noteFileName);
            noteArguments.replaceInStrings("%page", QString::number(index));
            noteArguments.replaceInStrings("%width", QString::number(noteLabel->width()));
            noteArguments.replaceInStrings("%height", QString::number(noteLabel->height()));
            qDebug() << "Calling external renderer:" << renderCommand << noteArguments;
            noteRenderer->start(renderCommand, noteArguments);
        }

        ExternalRenderer* smallRenderer  = new ExternalRenderer(index);
        if (!smallLabel->cacheContains(index)) {
            QStringList smallArguments = QStringList(renderArguments);
            smallArguments.replaceInStrings("%file", presFileName);
            smallArguments.replaceInStrings("%page", QString::number(index));
            smallArguments.replaceInStrings("%width", QString::number(smallLabel->width()));
            smallArguments.replaceInStrings("%height", QString::number(smallLabel->height()));
            qDebug() << "Calling external renderer:" << renderCommand << smallArguments;
            smallRenderer->start(renderCommand, smallArguments);
        }

        QByteArray const* pres = nullptr;
        QByteArray const* note = nullptr;
        QByteArray const* small = nullptr;
        if (!presLabel->cacheContains(index)) {
            if (presRenderer->waitForFinished(60000))
                pres = presRenderer->getBytes();
            else
                presRenderer->kill();
        }
        if (!noteLabel->cacheContains(index)) {
            if (noteRenderer->waitForFinished(60000))
                note = noteRenderer->getBytes();
            else
                noteRenderer->kill();
        }
        if (!smallLabel->cacheContains(index)) {
            if (smallRenderer->waitForFinished(60000))
                small = smallRenderer->getBytes();
            else
                smallRenderer->kill();
        }
        if (isInterruptionRequested()) {
            delete pres;
            delete note;
            delete small;
            return;
        }
        emit resultsReady(pres, note, small, index);
    }
}

void CacheUpdateThread::setLabels(const PageLabel *pres, const PageLabel *note, const PageLabel *small)
{
    presLabel = pres;
    noteLabel = note;
    smallLabel = small;
}

void CacheUpdateThread::setPages(const Poppler::Page *pres, const Poppler::Page *note)
{
    presPage = pres;
    notePage = note;
}

void CacheUpdateThread::setCustomRenderer(const QString &renderCommand, const QString &presFileName, const QString &noteFileName, const QStringList &renderArguments, Renderer renderer)
{
    this->renderCommand = renderCommand;
    this->presFileName = presFileName;
    this->noteFileName = noteFileName;
    this->renderArguments = renderArguments;
    mode = renderer;
}
