#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QSettings>
#include <QCommandLineParser>
#include <QDebug>
#include <QPen>
#include <QJsonObject>
#include "src/config.h"
#include "src/enumerates.h"
#include "src/pdfmaster.h"
#include "src/drawing/pointingtool.h"
#include "src/rendering/abstractrenderer.h"

class DrawTool;

/// Debug flags: combinable debug flags
#ifdef QT_DEBUG
enum DebugFlags
{
    NoLog = 0,
    DebugRendering = 1 << 0,
    DebugCache = 1 << 1,
    DebugDrawing = 1 << 2,
    DebugMedia = 1 << 3,
    DebugKeyInput = 1 << 4,
    DebugOtherInput = 1 << 5,
    DebugSettings = 1 << 6,
    DebugTransitions = 1 << 7,
    DebugPageChange = 1 << 8,
    DebugLayout = 1 << 9,
    DebugWidgets = 1 << 10,
    DebugAll = 0x7fff,
    DebugVerbose = 1 << 15,
};

/// Convert strings to DebugFlag components.
static const QMap<QString, DebugFlags> string_to_debug_flags
{
    {"", NoLog},
    {"none", NoLog},
    {"debug rendering", DebugRendering},
    {"debug cache", DebugCache},
    {"debug drawing", DebugDrawing},
    {"debug media", DebugMedia},
    {"debug key-input", DebugKeyInput},
    {"debug other-input", DebugOtherInput},
    {"debug settings", DebugSettings},
    {"debug transitions", DebugTransitions},
    {"debug page-change", DebugPageChange},
    {"debug layout", DebugLayout},
    {"debug widgets", DebugWidgets},
    {"debug all", DebugAll},
    {"debug verbose", DebugVerbose},
};
#endif

/// Class storing various preferences.
/// It should have only one instance, which is available globally through
/// the functions writable_preferences() and preferences().
/// Values get initialized by calling loadSettings().
class Preferences : public QObject
{
    Q_OBJECT

    /// Interface for configuration file. Only local configuration file is used.
    /// The system-wide configuration file is copied to a user space file, if
    /// that one does not exist.
    QSettings settings;

public:
    enum
    {
        /// Automatically change slides if the pdf defines a duration for the slide.
        AutoSlideChanges = 1 << 0,
        /// Log slide changes to standard output.
        LogSlideChanges = 1 << 1,
        /// Mute all multimedia content by default.
        MuteApplication = 1 << 2,
        /// Allow opening external links.
        OpenExternalLinks = 1 << 3,
        /// Finalize drawing paths
        FinalizeDrawnPaths = 1 << 4,
    };

    /************************/
    /* GLOBAL CONFIGURATION */
    /************************/

    // SETTINGS
    /// Path to GUI configuration file.
    QString gui_config_file;

    /// Path to manual (html formatted), required for help tab in settings.
    QString manual_file;

    /// Directory for icons
    QString icon_path;

#ifdef QT_DEBUG
    /// Flags for log and debug level.
    quint16 debug_level = NoLog;
#endif

    /// Other flags.
    quint8 global_flags = AutoSlideChanges;


    // DRAWING
    /// Maximum number of steps in drawing history of currently visible slide.
    int history_length_visible_slides;
    /// Maximum number of steps in drawing history of hidden slide.
    int history_length_hidden_slides;
    /// Define how should drawings be assigned to overlays.
    PdfMaster::OverlayDrawingMode overlay_mode = PdfMaster::Cumulative;
    /// Duration of a slide in an animation, in ms.
    int slide_duration_animation = 50;

    // SHAPE RECOGNITION
    /// Parameter for sensitivity of line detectoin.
    /// Larger numbers mean that more strokes are recognized as lines.
    qreal line_sensitivity = 0.005;
    /// Sensitivity for detecting ellipsis.
    qreal ellipse_sensitivity = 0.05;
    /// Ellipses with relative deviation of the radii smaller than this number
    /// will be considered as circles.
    qreal ellipse_to_circle_snapping = 0.025;
    /// Slope (angle in radiants with small angle approximation) for snapping
    /// to horizontal or vertical direction for recognized shapes.
    qreal snap_angle = 0.05;
    /// Tolerance in angles of different sides of a rectangle (radiants).
    qreal rect_angle_tolerance = 0.3;
    /// Allowed distance between first and last point in rectangle relative to variance of point coordinates.
    qreal rect_closing_tolerance = 0.1;

    // SELECTION
    qreal path_min_selectable_width = 3.;
    QPen selection_rect_pen {QPen(QColor(128,128,144,128), 0.5, Qt::DotLine)};
    QBrush selection_rect_brush {QBrush(QColor(128,128,144,32))};

    // RENDERING
    /// Threshold of page aspect ratio for splitting pages in notes and
    /// presentation.
    float page_part_threshold {2.5};
    /// PagePart which represents the presentation.
    PagePart default_page_part = FullPage;

#ifdef USE_MUPDF
    /// PDF engine (should be same as renderer except if renderer is external)
    PdfDocument::PdfEngine pdf_engine {PdfDocument::MuPdfEngine};
    /// Renderer used to convert PDF page to image.
    AbstractRenderer::Renderer renderer {AbstractRenderer::MuPDF};
#elif defined(USE_POPPLER)
    /// PDF engine (should be same as renderer except if renderer is external)
    PdfDocument::PdfEngine pdf_engine {PdfDocument::PopplerEngine};
    /// Renderer used to convert PDF page to image.
    AbstractRenderer::Renderer renderer {AbstractRenderer::Poppler};
#elif defined(USE_QTPDF)
    /// PDF engine (should be same as renderer except if renderer is external)
    PdfDocument::PdfEngine pdf_engine {PdfDocument::QtPDFEngine};
    /// Renderer used to convert PDF page to image.
    AbstractRenderer::Renderer renderer {AbstractRenderer::QtPDF};
#endif
#ifdef USE_EXTERNAL_RENDERER
    /// Rendering command for external renderer.
    QString rendering_command;
    /// Arguments to rendering_command.
    QStringList rendering_arguments;
#endif

    /// Maximally allowed memory size in bytes.
    /// Negative numbers are interpreted as infinity.
    float max_memory {-1.};
    /// Maximally allowed number of pages in cache.
    /// Negative numbers are interpreted as infinity.
    int max_cache_pages {-1};


    // INTERACTION
    /// Map key combinations to actions for global keyboard shortcuts.
    QMultiMap<QKeySequence, Action> key_actions
    {
        {Qt::Key_PageDown, Action::NextPage},
        {Qt::Key_PageUp, Action::PreviousPage},
        {Qt::Key_Space, Action::Update},
    };
    /// Map key combinations to tools. These tools are not owned by this.
    /// Only when the program ends and preferences() is the last object to
    /// be deleted, the remaining key_tools will be deleted in the destructor
    /// of preferences().
    /// Objects owning a tool which is listed here should always call
    /// writable_preferences().removeKeyTool before deleting this tool.
    QMultiMap<QKeySequence, Tool*> key_tools;

    /// Map gestures to actions
    QMultiMap<Gesture, Action> gesture_actions
    {
        {SwipeLeft, Action::NextPage},
        {SwipeRight, Action::PreviousPage},
        {SwipeUp, Action::NextPage},
        {SwipeDown, Action::PreviousPage},
    };

    /****************************/
    /* DEFINED PER PRESENTATION */
    /****************************/

    /// Map "presentation", "notes", ... to file names.
    /// This is needed to interpret GUI config.
    QMap<QString, QString> file_alias;
    /// Main document
    const PdfDocument *document = NULL;
    /// Number of pages in main document
    int number_of_pages = 0;


    /*********************************/
    /* VARIABLES WITHOUT FIXED VALUE */
    /*********************************/

    /// Current page number in reference presentation view.
    int page = 0;
    /// Previous page, required during slide changes.
    int previous_page = 0;

    /// Target time of presentation. Set to zero if timer is not running.
    QDateTime target_time;
    /// Passed time. When timer is running, this is set to UINT_LEAST32_MAX.
    quint32 msecs_passed = 0;
    /// Total time for presentation.
    quint32 msecs_total = 0;

    /// Tool used for other input device, owned by this.
    /// The keys are taken from InputDevice.
    QList<Tool*> current_tools
    {
        new PointingTool(Tool::Eraser, 10., QColor(128,128,128,192), Tool::TabletEraser|Tool::MouseRightButton, 0.5),
    };


    /*******************/
    /*    FUNCTIONS    */
    /*******************/

    /// Load settings from default config file location (defined by Qt).
    Preferences(QObject *parent = NULL);

    /// Load settings from given file.
    Preferences(const QString &file, QObject *parent = NULL);

    /// Destructor. Deletes current_tools and key_tools.
    ~Preferences();

    /// Load settings from QSettings.
    void loadSettings();

    /// Load settings from command line parser.
    /// Usually called after loadSettings(), since it should overwrite
    /// settings from the configuration file.
    void loadFromParser(const QCommandLineParser &parser);
#ifdef QT_DEBUG
    /// Set debug flags depending on command line option.
    void loadDebugFromParser(const QCommandLineParser &parser);
#endif

    /// Convert given file identifier to URL. If a relative path is provided,
    /// first check relative to the directory of the presentation PDF.
    /// Remote URLs are only returned if external links are enabled.
    /// The returned URL may be invalid.
    QUrl resolvePath(const QString &identifier) const noexcept;

    /// Get the current tool for a given input device or NULL if there is no
    /// tool for this device. The tool remains owned by preferences().
    Tool *currentTool(const int device) const noexcept;
    /// Remove (but don't delete) all occurences of tool in key_tools.
    /// This should be called by an object owning tool before it deletes tool.
    void removeKeyTool(const Tool *tool, const bool remove_from_settings);
    /// Change key sequence associated with tool from oldkeys to newkeys.
    /// oldkeys or newkeys may be left empty to add a new tool or remove
    /// an existing tool.
    void replaceKeyToolShortcut(const QKeySequence oldkeys, const QKeySequence newkeys, Tool *tool);

    /// Connect an action to a key code. This does not remove existing
    /// actions connected to the same key code.
    void addKeyAction(const QKeySequence sequence, const Action action);
    /// Remove an action from a key sequence.
    void removeKeyAction(const QKeySequence sequence, const Action action);

    /// Interpret input as definition of tools and actions.
    /// Append these tools and actions to the given lists.
    /// This function is used for reading tools/actions for
    /// key shortcuts.
    static void parseActionsTools(const QVariant &input, QList<Action> &actions, QList<Tool*> &tools, const int default_device = 0);

    /// Show error message in dialog in front of main window.
    void showErrorMessage(const QString &title, const QString &text) const;

    /// Set new GUI configuration file.
    bool setGuiConfigFile(const QString &file);

public slots:
    /// Set maximum memory for cache. This function uses double instead of
    /// qreal because it is connected to a QDoubleSpinBox.
    void setMemory(const double new_memory);
    /// Set maximal number of slides in cache.
    void setCacheSize(const int new_size);
    /// Set renderer. Allowed values are "poppler", "mupdf",
    /// "poppler + external" and "mupdf + external".
    void setRenderer(const QString &string);
    /// Set page part threshold. This function uses double instead of qreal
    /// because it is connected to a QDoubleSpinBox.
    void setPagePartThreshold(const double threshold);
    /// Set number of drawing history steps for visible slides.
    void setHistoryVisibleSlide(const int length);
    /// Set number of drawing history steps for hidden slides.
    void setHistoryHiddenSlide(const int length);
    /// Enable/disable logging of slide changes.
    void setLogSlideChanges(const bool log);
#ifdef USE_EXTERNAL_RENDERER
    /// Set rendering command. It is not checked whether this command is valid.
    void setRenderingCommand(const QString &string);
    /// Set rendering arguments. It is not checked whether these are valid.
    void setRenderingArguments(const QString &string);
#endif
    /// Set overlay mode. Allowed values are defined in string_to_overlay_mode:
    /// "per page", "per label" and "cumulative"
    void setOverlayMode(const QString &string);
    /// Enable or disable automatic slide changes.
    void setAutoSlideChanges(const bool show);
    /// Enable or disable external links.
    void setExternalLinks(const bool enable);
    /// Enable or disable finalizing drawn paths.
    void setFinalizePaths(const bool finalize);

signals:
    /// Send error message to master (main window), which will show a message box.
    void sendErrorMessage(const QString &title, const QString &text) const;
    /// Interrupt drawing to avoid problems when changing or deleting tools.
    void stopDrawing() const;
    /// Tell master to redistribute cache memory.
    void distributeMemory() const;
};


/// Get writable globally shared preferences object.
/// Init globally accessible preferences on first call.
Preferences *writable_preferences(Preferences *new_preferences = NULL);

/// Get read-only globally shared preferences object.
/// This is the usual way of accessing preferences.
const Preferences *preferences();


/// Create tool from JSON formatted input.
Tool *createTool(const QJsonObject &obj, const int default_device = 0);

/// Write tool properties to JSON object.
void toolToJson(const Tool *tool, QJsonObject &obj);


// Define macros for debugging.

#ifdef QT_DEBUG
// Show warning if debugging is enabled
#define warn_msg(msg) qWarning() << msg
// Show debug message if debugging is enabled for this type
#define debug_msg(msg_type, msg) if (preferences()->debug_level & (msg_type)) qDebug() << (msg_type) << msg;
// Show debug message if verbose debugging is enabled for this type
#define debug_verbose(msg_type, msg) if ((preferences()->debug_level & (msg_type|DebugVerbose)) == (msg_type|DebugVerbose)) qDebug() << (msg_type) << msg
#else
#define debug_msg(msg_type, msg)
#define debug_verbose(msg_type, msg)
#define warn_msg(msg) qWarning() << msg
#endif


#endif // PREFERENCES_H
