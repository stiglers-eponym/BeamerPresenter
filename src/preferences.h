#ifndef PREFERENCES_H
#define PREFERENCES_H

#include "src/enumerates.h"
#include "src/rendering/abstractrenderer.h"
#include <QSettings>
#include <QCommandLineParser>
#include <QFileDialog>
#include <QDebug>

/// Class storing various preferences.
/// It should have only one instance, which is available globally through
/// the functions writable_preferences() and preferences().
/// Values get initialized by calling loadSettings().
class Preferences
{
    QSettings settings;

public:
    /************************/
    /* GLOBAL CONFIGURATION */
    /************************/

    // SETTINGS
    /// Path to GUI configuration file.
    QString gui_config_file;


    // DRAWING
    /// Maximum number of steps in drawing history of currently visible slide.
    int history_length_visible_slides;
    /// Maximum number of steps in drawing history of hidden slide.
    int history_length_hidden_slides;


    // RENDERING
    /// Page part corresponding to the presentation.
    PagePart page_part = FullPage;

    /// Renderer used to convert PDF page to image.
    AbstractRenderer::Renderer renderer = AbstractRenderer::Poppler;
    /// Rendering command for external renderer.
    QString rendering_command;
    /// Arguments to rendering_command.
    QStringList rendering_arguments;


    // INTERACTION
    QMultiMap<quint32, Action> key_actions =
    {
        {Qt::Key_PageDown, Action::NextPage},
        {Qt::Key_PageUp, Action::PreviousPage},
        {Qt::Key_Space, Action::Update},
    };


    /****************************/
    /* DEFINED PER PRESENTATION */
    /****************************/

    /// Map "presentation", "notes", ... to file names.
    QMap<QString, QString> file_alias;


    /*********************************/
    /* VARIABLES WITHOUT FIXED VALUE */
    /*********************************/

    int page = 0;


    Preferences();
    void loadSettings();
    void saveSettings();
    void loadFromParser(const QCommandLineParser &parser);
};


Preferences &writable_preferences();
const Preferences &preferences();

#endif // PREFERENCES_H
