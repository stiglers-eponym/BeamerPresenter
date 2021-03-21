#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QTabWidget>

class QTextEdit;

/// Must always run in the same thread as writable_preferences().
/// This construction might change in the future.
class SettingsWidget : public QTabWidget
{
    Q_OBJECT

    /// Manual
    QTextEdit *help;

    /// General settings
    QWidget *misc;

    /// List and modify keyboard shortcuts
    QWidget *shortcuts;

    /// Settings affecting rendering and cache
    QWidget *rendering;

public:
    explicit SettingsWidget(QWidget *parent = NULL);

    QSize sizeHint() const noexcept override
    {return {100,200};}

    bool hasHeightForWidth() const noexcept override
    {return true;}

private slots:
    void appendShortcut();

signals:

};

#endif // SETTINGSWIDGET_H
