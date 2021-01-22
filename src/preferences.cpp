#include "src/preferences.h"

Preferences::Preferences() :
    settings(QSettings::NativeFormat, QSettings::UserScope, "beamerpresenter-new", "beamerpresenter-new")
{
    settings.setIniCodec("UTF-8");
}

Preferences::Preferences(const QString &file) :
    settings(file, QSettings::NativeFormat)
{
    settings.setIniCodec("UTF-8");
}

Preferences::~Preferences()
{
    qDeleteAll(current_tools);
    current_tools.clear();
    qDeleteAll(key_tools);
    key_tools.clear();
}

void Preferences::loadSettings()
{
#ifdef DEBUG_READ_CONFIGS
    qDebug() << "Loading settings:" << settings.fileName();
    qDebug() << settings.allKeys();
#endif

    // GENERAL SETTINGS
    gui_config_file = settings.value("gui config", "/etc/beamerpresenter/gui.json").toString();
    manual_file = settings.value("manual", "/usr/share/doc/beamerpresenter/manual.md").toString();

    // DRAWING
    settings.beginGroup("drawing");
    history_length_visible_slides = settings.value("history length visible", 100).toUInt();
    history_length_hidden_slides = settings.value("history length hidden", 50).toUInt();
    settings.endGroup();

    // RENDERING
    settings.beginGroup("rendering");
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
    settings.endGroup();

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
            key_tools.clear();
            for (const auto& key : allKeys)
            {
                const QKeySequence seq(key);
                if (seq.isEmpty())
                    qWarning() << "Unknown key sequence in config:" << key;
                else
                {
                    const quint32 key_code = quint32(seq[0] + seq[1] + seq[2] + seq[3]);

                    // First try to interpret sequence as json object.
                    // Here the way how Qt changes the string is not really optimal.
                    // First check if the value is already a json object.
                    //QJsonObject object;
                    QJsonArray array;
                    if (settings.value(key).canConvert(QMetaType::Type::QJsonArray))
                        array = settings.value(key).toJsonArray();
                    else if (settings.value(key).canConvert(QMetaType::Type::QJsonObject))
                        array.append(settings.value(key).toJsonObject());
                    else
                    {
                        // Try to parse a human readable input string as json object.
                        // In the input quotation marks must be escaped. For
                        // convenience it is allowed to use single quotation marks
                        // instead.
                        QJsonParseError error;
                        const QJsonDocument doc = QJsonDocument::fromJson(settings.value(key).toStringList().join(",").replace("'", "\"").toUtf8(), &error);
                        if (error.error == QJsonParseError::NoError)
                        {
                            if (doc.isArray())
                                array = doc.array();
                            else if (doc.isObject())
                                array.append(doc.object());
                        }
                    }
                    if (!array.isEmpty())
                    {
                        for (const auto &value : qAsConst(array))
                        {
                            if (!value.isObject())
                                continue;
                            const QJsonObject object = value.toObject();
                            int device = AnyDevice;
                            const QJsonValue json_device = object.value("device");
                            if (json_device.isString())
                                device = string_to_input_device.value(json_device.toString(), AnyDevice);
                            else if (json_device.isArray())
                            {
                                device = 0;
                                for (const auto &dev_string : static_cast<const QJsonArray>(json_device.toArray()))
                                    device |= string_to_input_device.value(dev_string.toString(), 0);
                                if (device == 0)
                                    device = AnyDevice;
                            }
                            Tool *tool = nullptr;
                            const BasicTool base_tool = string_to_tool.value(object.value("tool").toString());
                            switch (base_tool)
                            {
                            case Pen:
                            {
                                const QColor color(object.value("color").toString("black"));
                                const float width = object.value("width").toDouble(2.);
                                const Qt::PenStyle style = string_to_pen_style.value(object.value("style").toString(), Qt::SolidLine);
                                tool = new DrawTool(Pen, device, QPen(color, width, style, Qt::RoundCap));
                                break;
                            }
                            case Highlighter:
                            {
                                const QColor color(object.value("color").toString("yellow"));
                                const float width = object.value("width").toDouble(20.);
                                const Qt::PenStyle style = string_to_pen_style.value(object.value("style").toString(), Qt::SolidLine);
                                tool = new DrawTool(Highlighter, device, QPen(color, width, style, Qt::RoundCap), QPainter::CompositionMode_Darken);
                                break;
                            }
                            case InvalidTool:
                                break;
                            default:
                                tool = new Tool(base_tool, device);
                                break;
                            }
                            if (tool)
                            {
                                qDebug() << "Adding tool" << tool << tool->tool() << device;
                                key_tools.insert(key_code, tool);
                            }
                        }
                    }
                    else
                    {
                        for (const auto &action_str : static_cast<const QStringList>(settings.value(key).toStringList()))
                        {
                            const Action action = string_to_action_map.value(action_str.toLower(), Action::InvalidAction);
                            if (action == InvalidAction)
                                qWarning() << "Unknown action in config" << action_str << "for key" << key;
                            else
                                key_actions.insert(key_code, action);
                        }
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
    {
        file_alias["presentation"] = arguments.first();
        if (arguments.length() > 1 && !file_alias.contains("notes"))
            file_alias["notes"] = arguments[1];
    }

    if (parser.isSet("t"))
        msecs_total = 60000 * parser.value("t").toDouble();
    if (parser.isSet("renderer"))
    {
        QString const &renderer_str = parser.value("renderer").toLower();
        if (renderer != AbstractRenderer::ExternalRenderer && (renderer_str == "extern" || renderer_str == "external"))
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
#ifdef INCLUDE_MUPDF
        else if (renderer_str == "mupdf")
        {
            renderer = AbstractRenderer::MuPDF;
            pdf_backend = PdfDocument::MuPdfBackend;
        }
#endif
#ifdef INCLUDE_POPPLER
        else if (renderer_str == "poppler")
        {
            renderer = AbstractRenderer::Poppler;
            pdf_backend = PdfDocument::PopplerBackend;
        }
#endif
    }
#if defined(INCLUDE_MUPDF) and defined(INCLUDE_POPPLER)
    if (parser.isSet("engine"))
    {
        if (parser.value("engine").compare("mupdf", Qt::CaseInsensitive) == 0)
        {
            pdf_backend = PdfDocument::MuPdfBackend;
            if (renderer == AbstractRenderer::Poppler)
                renderer = AbstractRenderer::MuPDF;
        }
        else if (parser.value("engine").compare("poppler", Qt::CaseInsensitive) == 0)
        {
            pdf_backend = PdfDocument::PopplerBackend;
            if (renderer == AbstractRenderer::MuPDF)
                renderer = AbstractRenderer::Poppler;
        }
    }
#endif
}
