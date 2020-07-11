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

    QList<PdfMaster*> documents;
    QList<SlideScene*> scenes;
    QList<QWidget*> windows;
    /// Map "presentation", "notes", ... to file names.
    QMap<QString, QString> files;

public:
    Master();
    ~Master();
    void showAll() const;
    void addFile(const QString& name, const QString& path);
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
