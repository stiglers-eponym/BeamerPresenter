#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QTabWidget>

class QTextEdit;

/**
 * @class SettingsWidget
 * @brief Graphical intereface to preferences()
 *
 * This must always run in the same thread as preferences(). It contains
 * 4 tabs: a manual, general settings, shortcuts and rendering settings.
 */
class SettingsWidget : public QTabWidget
{
    Q_OBJECT

    /// Manual
    QTextEdit *manual;
    /// General settings
    QWidget *misc;
    /// List and modify keyboard shortcuts
    QWidget *shortcuts;
    /// Settings affecting rendering and cache
    QWidget *rendering;

    /// Initialize manual tab.
    void initManual();
    /// Initialize misc tab.
    void initMisc();
    /// Initialize shortcuts tab.
    void initShortcuts();
    /// Initialize rendering tab.
    void initRendering();

public:
    /// Construct all 4 tabs.
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
