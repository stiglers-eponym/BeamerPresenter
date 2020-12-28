#include "src/preferences.h"

Preferences::Preferences() :
    settings(QSettings::NativeFormat, QSettings::UserScope, "beamerpresenter-new", "beamerpresenter-new")
{
}

void Preferences::loadSettings()
{
#ifdef DEBUG_READ_CONFIGS
    qDebug() << "Loading settings:" << settings.fileName();
    qDebug() << settings.allKeys();
#endif

    // SETTINGS
    gui_config_file = settings.value("gui config", "/etc/beamerpresenter/gui.json").toString();

    // DRAWING
    history_length_visible_slides = settings.value("history length visible", 100).toUInt();
    history_length_hidden_slides = settings.value("history length hidden", 50).toUInt();

    // RENDERING
    { // page_part
        // TODO: implement!
        page_part_threshold = settings.value("page part threshold").toReal();
        const QString page_part_str = settings.value("page part").toString().toLower();
        if (page_part_str == "left" || page_part_str == "presentation left")
            page_part = LeftHalf;
        else if (page_part_str == "right" || page_part_str == "presentation right")
            page_part = RightHalf;
    }
    { // renderer
        const QString renderer_str = settings.value("renderer").toString().toLower();
#ifdef DEBUG_READ_CONFIGS
        qDebug() << renderer_str;
#endif
        if (!renderer_str.isEmpty())
        {
            if (renderer_str == "extern" || renderer_str == "external")
            {
                rendering_command = settings.value("rendering command").toString();
                rendering_arguments = settings.value("rendering arguments").toStringList();
                if (rendering_command.isEmpty() || rendering_arguments.isEmpty())
                {
                    qWarning() << "External renderer requested but no command or no arguments given. Falling back to Poppler.";
                    qInfo() << "Note that both \"rendering command\" and \"rendering arguments\" are required.";
                }
                else
                    renderer = AbstractRenderer::ExternalRenderer;
            }
            else if (renderer_str == "mupdf")
            {
#ifdef INCLUDE_MUPDF
                renderer = AbstractRenderer::MuPDF;
                pdf_backend = PdfDocument::MuPdfBackend;
#else
                qWarning() << "BeamerPresenter was compiled without MuPDF support. Falling back to Poppler.";
#endif
            }
            else if (renderer_str == "poppler")
            {
#ifdef INCLUDE_POPPLER
                renderer = AbstractRenderer::Poppler;
                pdf_backend = PdfDocument::PopplerBackend;
#else
                qWarning() << "BeamerPresenter was compiled without poppler support. Falling back to MuPDF.";
#endif
            }
            else
#ifdef INCLUDE_POPPLER
                qWarning() << "Cannot not understand renderer" << renderer_str << ". Falling back to Poppler.";
#else
                qWarning() << "Cannot not understand renderer" << renderer_str << ". Falling back to MuPDF.";
#endif
        }
    }

    { // cache
        bool ok;
        max_memory = settings.value("memory").toFloat(&ok);
        if (!ok)
            max_memory = -1.;
        max_cache_pages = settings.value("cache pages").toInt(&ok);
        if (!ok)
            max_cache_pages = -1;
    }

    // INTERACTION
    {
        // Keyboard shortcuts
        settings.beginGroup("keys");
        const QStringList allKeys = settings.allKeys();
        if (!allKeys.isEmpty())
        {
            key_actions.clear();
            for (const auto& key : allKeys)
            {
                const QKeySequence seq(key);
                if (seq.isEmpty())
                {
                    qWarning() << "Unknown key sequence in config:" << key;
                }
                else
                {
                    const quint32 seq_int = quint32(seq[0] + seq[1] + seq[2] + seq[3]);
                    for (const auto &action_str : static_cast<const QStringList>(settings.value(key).toStringList()))
                    {
                        const Action action = string_to_action_map.value(action_str.toLower(), Action::InvalidAction);
                        if (action == InvalidAction)
                            qWarning() << "Unknown action in config" << action_str << "for key" << key;
                        else
                            key_actions.insert(seq_int, action);
                    }
                }
            }
        }
        settings.endGroup();
    }
}

void Preferences::loadFromParser(const QCommandLineParser &parser)
{
    const QStringList arguments = parser.positionalArguments();
    if (arguments.isEmpty())
        file_alias["presentation"] = QFileDialog::getOpenFileName(nullptr, "Presentation file", "", "Documents (*.pdf)");
    else
        file_alias["presentation"] = arguments.first();
    // TODO
}
