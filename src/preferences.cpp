#include "src/preferences.h"


Tool *createTool(const QJsonObject &obj)
{
    const BasicTool base_tool = string_to_tool.value(obj.value("tool").toString());
    Tool *tool;
    switch (base_tool)
    {
    case Pen:
    {
        const QColor color(obj.value("color").toString("black"));
        const float width = obj.value("width").toDouble(2.);
        const Qt::PenStyle style = string_to_pen_style.value(obj.value("style").toString(), Qt::SolidLine);
        tool = new DrawTool(Pen, AnyDevice, QPen(color, width, style, Qt::RoundCap, Qt::RoundJoin));
        break;
    }
    case Highlighter:
    {
        const QColor color(obj.value("color").toString("yellow"));
        const float width = obj.value("width").toDouble(20.);
        const Qt::PenStyle style = string_to_pen_style.value(obj.value("style").toString(), Qt::SolidLine);
        tool = new DrawTool(Highlighter, AnyDevice, QPen(color, width, style, Qt::RoundCap, Qt::RoundJoin), QPainter::CompositionMode_Darken);
        break;
    }
    case Pointer:
    {
        const QColor color(obj.value("color").toString("red"));
        const float size = obj.value("size").toDouble(5.);
        tool = new PointingTool(Pointer, size, color, AnyDevice);
        break;
    }
    case Torch:
    {
        const QColor color(obj.value("color").toString("#80000000"));
        const float size = obj.value("size").toDouble(80.);
        tool = new PointingTool(Torch, size, color, AnyDevice);
        break;
    }
    case Magnifier:
    {
        const QColor color(obj.value("color").toString("black"));
        const float size = obj.value("size").toDouble(120.);
        PointingTool *pointing_tool = new PointingTool(Magnifier, size, color, AnyDevice);
        if (obj.contains("scale"))
            pointing_tool->setScale(obj.value("scale").toDouble(2.));
        tool = pointing_tool;
        break;
    }
    case InvalidTool:
        return NULL;
    default:
        tool = new Tool(base_tool, AnyDevice);
    }
    int device = 0;
    const QJsonValue dev_obj = obj.value("device");
    if (dev_obj.isString())
        device |= string_to_input_device.value(dev_obj.toString());
    else if (dev_obj.isArray())
        for (const auto &dev : static_cast<const QJsonArray>(obj.value("devices").toArray()))
            device |= string_to_input_device.value(dev.toString());
    tool->setDevice(device);
    return tool;
}


Preferences::Preferences(QObject *parent) :
    QObject(parent),
    settings(QSettings::NativeFormat, QSettings::UserScope, "beamerpresenter", "beamerpresenter")
{
    settings.setIniCodec("UTF-8");
}

Preferences::Preferences(const QString &file, QObject *parent) :
    QObject(parent),
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
    debug_msg(DebugSettings) << "Loading settings:" << settings.fileName();
    debug_msg(DebugSettings) << settings.allKeys();

    // GENERAL SETTINGS
    {
        gui_config_file = settings.value("gui config", "/etc/beamerpresenter/gui.json").toString();
        manual_file = settings.value("manual", "/usr/share/doc/beamerpresenter/manual.md").toString();
        const QStringList log_flags = settings.value("log").toStringList();
        log_level = NoLog;
        for (const auto &flag : log_flags)
            log_level |= string_to_log_level.value(flag, NoLog);
    }

    // DRAWING
    {
        settings.beginGroup("drawing");
        history_length_visible_slides = settings.value("history length visible", 100).toUInt();
        history_length_hidden_slides = settings.value("history length hidden", 50).toUInt();
        settings.endGroup();
    }

    // RENDERING
    settings.beginGroup("rendering");
    { // page_part threshold
        page_part_threshold = settings.value("page part threshold").toReal();
    }
    { // renderer
        const QString renderer_str = settings.value("renderer").toString().toLower();
        debug_msg(DebugSettings) << renderer_str;
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
                pdf_engine = PdfDocument::MuPdfEngine;
#else
                qWarning() << "BeamerPresenter was compiled without MuPDF support. Falling back to Poppler.";
#endif
            }
            else if (renderer_str == "poppler")
            {
#ifdef INCLUDE_POPPLER
                renderer = AbstractRenderer::Poppler;
                pdf_engine = PdfDocument::PopplerEngine;
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
                            Tool *tool = createTool(object);
                            if (tool)
                            {
                                debug_msg(DebugSettings|DebugDrawing) << "Adding tool" << tool << tool->tool() << device;
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
    // presentation file from positional arguments
    const QStringList arguments = parser.positionalArguments();
    if (!arguments.isEmpty())
    {
        file_alias["presentation"] = arguments.first();
        if (arguments.length() > 1 && !file_alias.contains("notes"))
            file_alias["notes"] = arguments[1];
    }

    // timer total time
    if (parser.isSet("t"))
        msecs_total = 60000 * parser.value("t").toDouble();

#ifdef QT_DEBUG
    // Debug legel
    if (parser.isSet("debug"))
    {
        log_level = 0;
        for (const auto &flag : static_cast<const QStringList>(parser.value("debug").split(",")))
            log_level |= string_to_log_level.value("debug " + flag, NoLog);
    }
#endif

    // Log slide changes
    if (parser.isSet("log"))
        log_level |= LogSlideChanges;

    // renderer and pdf engine
    if (parser.isSet("renderer"))
    {
        QString const &renderer_str = parser.value("renderer");
        debug_msg(DebugSettings) << "renderer" << renderer_str;
#ifdef INCLUDE_MUPDF
        if (renderer_str.count("mupdf", Qt::CaseInsensitive) > 0)
        {
            renderer = AbstractRenderer::MuPDF;
            pdf_engine = PdfDocument::MuPdfEngine;
        }
#endif
#ifdef INCLUDE_POPPLER
        if (renderer_str.count("poppler", Qt::CaseInsensitive) > 0)
        {
            renderer = AbstractRenderer::Poppler;
            pdf_engine = PdfDocument::PopplerEngine;
        }
#endif
        if (renderer_str.count("extern", Qt::CaseInsensitive) > 0)
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
    }
}

void Preferences::addKeyAction(const quint32 sequence, const Action action)
{
    if (!key_actions.contains(sequence) || !key_actions.values(sequence).contains(action))
        key_actions.insert(sequence, action);
    const QString keycode = QKeySequence(sequence).toString();
    if (!keycode.isEmpty())
    {
        settings.beginGroup("keys");
        QStringList list = settings.value(keycode).toStringList();
        const QString &string = string_to_action_map.key(action);
        if (!list.contains(string))
        {
            list.append(string);
            settings.setValue(keycode, list);
        }
        settings.endGroup();
    }
}

void Preferences::removeKeyAction(const quint32 sequence, const Action action)
{
    key_actions.remove(sequence, action);
    settings.beginGroup("keys");
    const QString keycode = QKeySequence(sequence).toString();
    if (!keycode.isEmpty() && settings.contains(keycode))
    {
        QStringList list = settings.value(keycode).toStringList();
        list.removeAll(string_to_action_map.key(action));
        if (list.isEmpty())
            settings.remove(keycode);
        else
            settings.setValue(keycode, list);
    }
    settings.endGroup();
}

void Preferences::setMemory(const QString &string)
{
    bool ok;
    const float new_memory = string.toFloat(&ok);
    if (ok)
    {
        max_memory = 1048596*new_memory;
        settings.setValue("memory", QString::number(max_memory));
    }
    emit distributeMemory();
}

void Preferences::setCacheSize(const QString &string)
{
    bool ok;
    const int new_size = string.toInt(&ok);
    if (ok)
    {
        max_cache_pages = new_size;
        settings.setValue("cache pages", QString::number(max_cache_pages));
    }
    emit distributeMemory();
}

void Preferences::setRenderer(const QString &string)
{
    const QString &new_renderer = string.toLower();
#ifdef INCLUDE_MUPDF
    if (new_renderer == "mupdf")
    {
        settings.setValue("engine", "mupdf");
        settings.setValue("renderer", "mupdf");
    }
    else
#endif
#ifdef INCLUDE_POPPLER
    if (new_renderer == "poppler")
    {
        settings.setValue("engine", "poppler");
        settings.setValue("renderer", "poppler");
    }
    else
#endif
#ifdef INCLUDE_MUPDF
    if (new_renderer == "mupdf + external")
    {
        settings.setValue("engine", "mupdf");
        settings.setValue("renderer", "external");
    }
#endif
#ifdef INCLUDE_POPPLER
    else if (new_renderer == "poppler + external")
    {
        settings.setValue("engine", "poppler");
        settings.setValue("renderer", "externaler");
    }
#endif
}

Tool *Preferences::currentTool(const int device) const noexcept
{
    for (const auto tool : preferences()->current_tools)
    {
        if (tool && (tool->device() & device))
            return tool;
    }
    return NULL;
}
