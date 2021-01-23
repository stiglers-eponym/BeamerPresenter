#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QTabWidget>
#include <QTextEdit>
#include <QFormLayout>
#include <QScrollArea>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include "src/preferences.h"
#include "src/gui/keyinputlabel.h"

/// Must always run in the same thread as writable_preferences().
/// This construction might change in the future.
class SettingsWidget : public QTabWidget
{
    Q_OBJECT

    /// Manual
    QTextEdit *help;

    /// General settings: save settings, ...
    QScrollArea *general;

    /// List and modify keyboard shortcuts
    QScrollArea *shortcuts;

    /// Settings affecting rendering and cache
    QScrollArea *rendering;

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
