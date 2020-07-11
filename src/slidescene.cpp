#include "src/slidescene.h"

SlideScene::SlideScene(const PdfMaster *master, QObject *parent) :
    QGraphicsScene(parent),
    master(master)
{

}

void SlideScene::stopDrawing()
{
    if (currentPath.isEmpty())
        return;
    // TODO: add path to other paths.
}

unsigned int SlideScene::identifier() const
{
    return qHash(QPair<int, const void*>(shift,master));
}
