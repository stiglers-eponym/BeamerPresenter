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
    history_length_hidden_slides = settings.value("history length visible", 100).toUInt();
    history_length_hidden_slides = settings.value("history length hidden", 50).toUInt();

    // RENDERING
    { // page_part
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
            if (renderer_str == "mupdf")
            {
    #ifdef INCLUDE_MUPDF
                renderer = AbstractRenderer::MuPDF;
    #else
                qWarning() << "BeamerPresenter was compiled without MuPDF support. Falling back to Poppler.";
                renderer = AbstractRenderer::Poppler;
    #endif
            }
            else if (renderer_str == "extern" || renderer_str == "external")
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
            else if (renderer_str != "poppler" && renderer_str != "default")
                qWarning() << "Cannot not understand renderer" << renderer_str << ". Falling back to Poppler.";
        }
    }
}

void Preferences::loadFromParser(const QCommandLineParser &parser)
{
    if (parser.positionalArguments().isEmpty())
        file_alias["presentation"] = QFileDialog::getOpenFileName(nullptr, "Presentation file", "", "Documents (*.pdf)");
    else
        file_alias["presentation"] = parser.positionalArguments().first();
}

void Preferences::saveSettings()
{
    if (!settings.isWritable())
    {
        qCritical() << "Cannot save settings: file not writable";
        return;
    }
}
