#ifndef PREFERENCES_H
#define PREFERENCES_H

#include "src/enumerates.h"
#include "src/rendering/abstractrenderer.h"

/// Class storing various preferences.
/// It should have only one instance, owned by (the only instance of)
/// ControlScreen. Instances of other classes get const pointers to the
/// instance owned by ControlScreen.
class Preferences
{
public:
    // DRAWING
    /// Maximum number of steps in drawing history of currently visible slide.
    int history_length_visible_slides = 100;
    /// Maximum number of steps in drawing history of hidden slide.
    int history_length_hidden_slides = 50;


    // RENDERING
    /// Page part corresponding to the presentation.
    PagePart page_part = FullPage;

    /// Renderer used to convert PDF page to image.
    AbstractRenderer::Renderer renderer = AbstractRenderer::Poppler;
    /// Rendering command for external renderer.
    QString rendering_command;
    /// Arguments to rendering_command.
    QStringList rendering_arguments;


    // GUI
    QString gui_config_file = "/etc/beamerpresenter/gui.json";


    // INPUT


    Preferences();
};

Preferences &writable_preferences()
{
    static Preferences preferences;
    return preferences;
}

const Preferences &preferences()
{
    return writable_preferences();
}

#endif // PREFERENCES_H
