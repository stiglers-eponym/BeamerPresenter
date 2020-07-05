#ifndef PREFERENCES_H
#define PREFERENCES_H

#include "enumerates.h"

/// Class storing various preferences.
/// It should have only one instance, owned by (the only instance of)
/// ControlScreen. Instances of other classes get const pointers to the
/// instance owned by ControlScreen.
class Preferences
{
public:
    int history_length_visible_slides = 100;
    int history_length_hidden_slides = 50;

    /// Page part corresponding to the presentation.
    PagePart pagePart = FullPage;
    Preferences();
};

#endif // PREFERENCES_H
