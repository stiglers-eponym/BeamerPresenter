#ifndef PREFERENCES_H
#define PREFERENCES_H

#include "src/enumerates.h"
#include "src/names.h"
#include "src/rendering/abstractrenderer.h"
#include "src/rendering/pdfdocument.h"
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
    PagePart page_part {FullPage};

    /// PDF backend (should be same as renderer except if renderer is external)
#ifdef INCLUDE_POPPLER
    PdfDocument::PdfBackend pdf_backend {PdfDocument::PopplerBackend};
#else
    PdfDocument::PdfBackend pdf_backend {PdfDocument::MuPdfBackend};
#endif
    /// Renderer used to convert PDF page to image.
#ifdef INCLUDE_POPPLER
    AbstractRenderer::Renderer renderer {AbstractRenderer::Poppler};
#else
    AbstractRenderer::Renderer renderer {AbstractRenderer::MuPDF};
#endif
    /// Rendering command for external renderer.
    QString rendering_command;
    /// Arguments to rendering_command.
    QStringList rendering_arguments;

    /// Maximally allowed memory size in bytes.
    float max_memory = -1;


    // INTERACTION
    QMultiMap<quint32, Action> key_actions
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
    void loadFromParser(const QCommandLineParser &parser);
};


Preferences &writable_preferences();
const Preferences &preferences();

#endif // PREFERENCES_H
