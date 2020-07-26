#ifndef MASTER_H
#define MASTER_H

#include <QWidget>
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QBoxLayout>
#include <QKeyEvent>
#include <QSizePolicy>
#include "src/enumerates.h"
#include "src/names.h"

class PdfMaster;
class SlideScene;
class ContainerWidget;
class PixCache;

class Master : public QObject
{
    Q_OBJECT

    /// List of all PDF documents.
    /// Master file is the first entry in this list.
    QList<PdfMaster*> documents;
    /// Graphics scenes of this application. For each combination of PDF file
    /// and page shift one scene is created.
    /// Master scene is the first scene in the list.
    QList<SlideScene*> scenes;
    /// Map of cache hashs to cache objects.
    QMap<int, PixCache*> caches;
    /// List of all windows of the applications.
    QList<QWidget*> windows;

public:
    Master();
    ~Master();
    /// Show all windows of the application.
    void showAll() const;
    /// Read configuration file and build up GUI. Return true on success.
    bool readGuiConfig(const QString& filename);
    /// Create widgets recursively.
    QPair<QWidget*, GuiWidget*> createWidget(QJsonObject& object, ContainerWidget *parent = nullptr);
    /// Calculate total cache size (sum up cache sizes from all PixCache objects).
    qint64 getTotalCache() const;

public slots:
    /// Handle key events collected by any other object.
    /// By default all key events are redirected to this and handled here.
    void receiveKeyEvent(QKeyEvent const* event);

    /// Read memory size restriction from preferences and distribute memory to pixcaches.
    void distributeMemory();

signals:
    /// Send out action.
    void sendAction(const Action action) const;
    /// Send out nagivation signal (after updating preferences().page).
    void navigationSignal(const int page) const;
};

#endif // MASTER_H
