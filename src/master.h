#ifndef MASTER_H
#define MASTER_H

#include <QWidget>
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QBoxLayout>
#include <QKeyEvent>
#include "src/enumerates.h"
#include "src/names.h"

class PdfMaster;
class SlideScene;
class ContainerWidget;

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
    QWidget *createWidget(QJsonObject& object, ContainerWidget *parent = nullptr);

public slots:
    void receiveKeyEvent(QKeyEvent const* event);

signals:
    void sendAction(const Action action);
    void navigationSignal(const int page);
};

#endif // MASTER_H
