#ifndef PREFERENCES_H
#define PREFERENCES_H

#include "src/enumerates.h"
#include "src/names.h"
#include "src/rendering/abstractrenderer.h"
#include "src/rendering/pdfdocument.h"
#include "src/drawing/drawtool.h"
#include <QSettings>
#include <QCommandLineParser>
#include <QFileDialog>
#include <QPen>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

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

    /// Path to manual (markdown formatted), required for help tab in settings.
    QString manual_file;


    // DRAWING
    /// Maximum number of steps in drawing history of currently visible slide.
    int history_length_visible_slides;
    /// Maximum number of steps in drawing history of hidden slide.
    int history_length_hidden_slides;
    /// Which devices should be used for pressure-sensitive input:
    int pressure_sensitive_input_devices = TabletPen | TabletEraser | TabletCursor | TabletOther;


    // RENDERING
    /// Page part corresponding to the presentation.
    PagePart page_part {FullPage};
    float page_part_threshold {0.};

#ifdef INCLUDE_POPPLER
    /// PDF backend (should be same as renderer except if renderer is external)
    PdfDocument::PdfBackend pdf_backend {PdfDocument::PopplerBackend};
    /// Renderer used to convert PDF page to image.
    AbstractRenderer::Renderer renderer {AbstractRenderer::Poppler};
#else
    /// PDF backend (should be same as renderer except if renderer is external)
    PdfDocument::PdfBackend pdf_backend {PdfDocument::MuPdfBackend};
    /// Renderer used to convert PDF page to image.
    AbstractRenderer::Renderer renderer {AbstractRenderer::MuPDF};
#endif
    /// Rendering command for external renderer.
    QString rendering_command;
    /// Arguments to rendering_command.
    QStringList rendering_arguments;

    /// Maximally allowed memory size in bytes.
    /// Negative numbers are interpreted as infinity.
    float max_memory {-1.};
    /// Maximally allowed number of pages in cache.
    /// Negative numbers are interpreted as infinity.
    int max_cache_pages {-1};


    // INTERACTION
    /// Map key combinations to actions for global keyboard shortcuts.
    QMultiMap<quint32, Action> key_actions
    {
        {Qt::Key_PageDown, Action::NextPage},
        {Qt::Key_PageUp, Action::PreviousPage},
        {Qt::Key_Space, Action::Update},
    };
    /// Map key combinations to tools.
    QMultiMap<quint32, const Tool*> key_tools;


    /****************************/
    /* DEFINED PER PRESENTATION */
    /****************************/

    /// Map "presentation", "notes", ... to file names.
    /// This is needed to interpret GUI config.
    QMap<QString, QString> file_alias;
    int number_of_pages = 0;


    /*********************************/
    /* VARIABLES WITHOUT FIXED VALUE */
    /*********************************/

    /// Current page number in reference presentation view.
    int page = 0;

    /// Target time of presentation. Set to zero if timer is not running.
    QDateTime target_time;
    /// Passed time. When timer is running, this is set to UINT_LEAST32_MAX.
    quint32 msecs_passed = 0;
    /// Total time for presentation.
    quint32 msecs_total = 0;

    /// Tool used for other input device, owned by this.
    /// The keys are taken from InputDevice.
    QSet<Tool*> current_tools
    {
        new Tool(Eraser, TabletEraser),
        new Tool(Eraser, MouseRightButton),
    };


    Preferences();
    ~Preferences();
    void loadSettings();
    void loadFromParser(const QCommandLineParser &parser);
};


/// Get writable globally shared preferences object.
Preferences &writable_preferences();
/// Get read-only globally shared preferences object.
/// This is the usual way of accessing preferences.
const Preferences &preferences();

#endif // PREFERENCES_H