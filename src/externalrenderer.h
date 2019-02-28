#ifndef EXTERNALRENDERER_H
#define EXTERNALRENDERER_H

#include <QtDebug>
#include <QWidget>
#include <QProcess>

class ExternalRenderer : public QProcess
{
    Q_OBJECT

public:
    ExternalRenderer(int const page, QObject* parent = nullptr);
    QByteArray const* getBytes() {return bytes;}

private:
    int page;
    QByteArray const* bytes = nullptr;

public slots:
    void returnImage(int const exitCode, QProcess::ExitStatus exitStatus);

signals:
    void sendImage(QByteArray const* bytes, int const page);
};

#endif // EXTERNALRENDERER_H
