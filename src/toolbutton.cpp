/*
 * This file is part of BeamerPresenter.
 * Copyright (C) 2019  stiglers-eponym

 * BeamerPresenter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * BeamerPresenter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with BeamerPresenter. If not, see <https://www.gnu.org/licenses/>.
 */

#include "toolbutton.h"

static const QMap<KeyAction, QString> actionNames = {
    {Previous, "prev"},
    {Next, "next"},
    {PreviousNotes, "prev (notes)"},
    {NextNotes, "next (notes)"},
    {PreviousSkippingOverlays, "prev slide"},
    {NextSkippingOverlays, "next slide"},
    {PreviousNotesSkippingOverlays, "prev slide (notes)"},
    {NextNotesSkippingOverlays, "ntext slide (notes)"},
    {GoToPage, "goto"},
    {LastPage, "last"},
    {FirstPage, "first"},
    {SyncFromControlScreen, "sync presentation"},
    {SyncFromPresentationScreen, "sync control"},
    {Update, "update"},
    {UpdateCache, "update cache"},

#ifdef EMBEDDED_APPLICATIONS_ENABLED
    {StartEmbeddedCurrentSlide, "embedded"},
    {StartAllEmbedded, "all embedded"},
    {CloseEmbeddedCurrentSlide, "close embedded"},
    {CloseAllEmbedded, "close all embedded"},
#endif
    {PlayMultimedia, "play media"},
    {PauseMultimedia, "pause media"},
    {PlayPauseMultimedia, "play/pause"},

    {PlayPauseTimer, "timer"},
    {ContinueTimer, "continue timer"},
    {PauseTimer, "pause timer"},
    {ResetTimer, "reset timer"},
    {ToggleTOC, "TOC"},
    {ShowTOC, "TOC"},
    {HideTOC, "hide TOC"},
    {ToggleOverview, "overview"},
    {ShowOverview, "overview"},
    {HideOverview, "hide overview"},
    {HideDrawSlide, "end drawing"},
    {ToggleCursor, "cursor"},
    {ShowCursor, "show cursor"},
    {HideCursor, "hide cursor"},
    {FullScreen, "full screen"},
    {Reload, "reload"},
    {Quit, "quit"},

    {ClearAnnotations, "clear"},
    {DrawNone, "Hand"},
    {ToggleDrawMode, "Draw"},
    {DrawMode, "Draw"},
    {ToggleDrawMode, "Draw"},
    {DrawEraser, "Eraser"},
    {DrawPen, "Pen"},
    {DrawHighlighter, "Highlight"},
    {DrawPointer, "Pointer"},
    {DrawMagnifier, "Magnifier"},
    {DrawTorch, "Torch"},
};

static const QMap<KeyAction, QString> actionIconNames = {
    {PlayMultimedia, "media-playback-start"},
    {PauseMultimedia, "media-playback-stop"},
    {PlayPauseMultimedia, "media-playback-pause"},
    {Next, "go-next"},
    {Previous, "go-previous"},
    {FirstPage, "go-first"},
    {LastPage, "go-last"},
    {GoToPage, "go-jump"},
    {PreviousSkippingOverlays, "go-up"},
    {NextSkippingOverlays, "go-down"},
    {Quit, "application-exit"},
    {FullScreen, "view-fullscreen"},
    {Update, "view-refresh"},
    {ClearAnnotations, "edit-clear"},
    {PauseTimer, "media-playback-pause"},
#ifdef EMBEDDED_APPLICATIONS_ENABLED
    {StartEmbeddedCurrentSlide, "application-x-executable"},
#endif
    // TODO: more and better icons
};

ToolButton::ToolButton(QList<KeyAction> const actions, QColor const color, QWidget* parent) : QPushButton(parent)
{
    QPalette palette;
    this->actions = actions;
    this->color = color;
    if (actions.size() == 0) {
        this->actions = {NoAction};
        return;
    }
    connect(this, &QPushButton::clicked, this, &ToolButton::onClicked);
    if (actions.size() == 1 && actions[0] == DrawPen)
        palette.setColor(QPalette::ButtonText, color);
    else if (color.lightness() > color.alpha()/2)
        palette.setColor(QPalette::Button, color);
    setPalette(palette);
    QString const iconname = actionIconNames.value(actions[0], "");
    if (iconname.isEmpty())
        setText(actionNames.value(actions[0], QString::number(actions[0], 16)));
    else {
        QIcon const icon = QIcon::fromTheme(iconname);
        if (icon.isNull())
            setText(actionNames.value(actions[0], QString::number(actions[0], 16)));
        else
            setIcon(icon);
    }
    if (actions.size() == 1 && color != QColor(0,0,0,0))
        tool = actionToToolMap.value(actions.first(), {None, color}).tool;
}

void ToolButton::onClicked()
{
    if (tool == None) {
        for (auto action : actions)
            emit sendAction(action);
    }
    else {
        emit sendTool({tool, color});
        if (actions.size() > 1) {
            for (QList<KeyAction>::const_iterator it=actions.cbegin()+1; it!=actions.cend(); it++)
                emit sendAction(*it);
        }
    }
}
