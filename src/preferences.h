#ifndef PREFERENCES_H
#define PREFERENCES_H

#include "src/enumerates.h"
#include "src/names.h"
#include "src/rendering/abstractrenderer.h"
#include "src/rendering/pdfdocument.h"
#include "src/drawing/drawtool.h"
#include "src/drawing/pointingtool.h"
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
class Preferences : public QObject
{
    Q_OBJECT

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

    int log_level = NoLog;


    // DRAWING
    /// Maximum number of steps in drawing history of currently visible slide.
    int history_length_visible_slides;
    /// Maximum number of steps in drawing history of hidden slide.
    int history_length_hidden_slides;
    /// Which devices should be used for pressure-sensitive input:
    int pressure_sensitive_input_devices = TabletPen | TabletEraser | TabletCursor | TabletOther;


    // RENDERING
    /// Threshold of page aspect ratio for splitting pages in notes and
    /// presentation.
    float page_part_threshold {0.};
    PagePart default_page_part = FullPage;

#ifdef INCLUDE_MUPDF
    /// PDF engine (should be same as renderer except if renderer is external)
    PdfDocument::PdfEngine pdf_engine {PdfDocument::MuPdfEngine};
    /// Renderer used to convert PDF page to image.
    AbstractRenderer::Renderer renderer {AbstractRenderer::MuPDF};
#else
    /// PDF engine (should be same as renderer except if renderer is external)
    PdfDocument::PdfEngine pdf_engine {PdfDocument::PopplerEngine};
    /// Renderer used to convert PDF page to image.
    AbstractRenderer::Renderer renderer {AbstractRenderer::Poppler};
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
    QMultiMap<quint32, Tool*> key_tools;


    /****************************/
    /* DEFINED PER PRESENTATION */
    /****************************/

    /// Map "presentation", "notes", ... to file names.
    /// This is needed to interpret GUI config.
    QMap<QString, QString> file_alias;
    const PdfDocument *document;
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


    /// Load settings from default config file location (defined by Qt).
    Preferences(QObject *parent = NULL);

    /// Load settings from given file.
    Preferences(const QString &file, QObject *parent = NULL);

    ~Preferences();

    /// Load settings from QSettings.
    void loadSettings();

    /// Load settings from command line parser.
    /// Usually called after loadSettings().
    void loadFromParser(const QCommandLineParser &parser);

    Tool *currentTool(const int device) const noexcept;
    void replaceKeyTool(const int keys, Tool *newtool);

    void addKeyAction(const quint32 sequence, const Action action);
    void removeKeyAction(const quint32 sequence, const Action action);

public slots:
    void setMemory(const QString &string);
    void setCacheSize(const QString &string);
    void setRenderer(const QString &string);

signals:
    void stopDrawing() const;
    void distributeMemory() const;
};


/// Get writable globally shared preferences object.
/// Init globally accessible preferences on first call.
Preferences *writable_preferences(Preferences *new_preferences = NULL);

/// Get read-only globally shared preferences object.
/// This is the usual way of accessing preferences.
const Preferences *preferences();


Tool *createTool(const QJsonObject &obj);


#ifdef QT_DEBUG
// Show warning if debugging is enabled
#define warn_msg qWarning()
// Show debug message if debugging is enabled for this type
#define debug_msg(msg_type) if (preferences()->log_level & (msg_type)) qDebug() << (msg_type)
// Show debug message if verbose debugging is enabled for this type
#define debug_verbose(msg_type) if ((preferences()->log_level & (msg_type|DebugVerbose)) == (msg_type|DebugVerbose)) qDebug() << (msg_type)
#else
#define debug_msg(msg_type) qDebug()
#define debug_verbose(msg_type) qDebug()
#define warn_msg qDebug()
#endif


#endif // PREFERENCES_H
