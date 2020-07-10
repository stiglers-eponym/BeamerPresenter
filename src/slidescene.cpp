#include "src/slidescene.h"

SlideScene::SlideScene(QObject *parent) : QGraphicsScene(parent)
{

}

void SlideScene::stopDrawing()
{
    if (currentPath.isEmpty())
        return;
    // TODO: add path to other paths.
}
