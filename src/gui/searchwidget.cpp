// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include <QLineEdit>
#include <QPair>
#include <QRectF>
#include <QToolButton>
#include <QHBoxLayout>
#include "src/gui/searchwidget.h"
#include "src/preferences.h"

SearchWidget::SearchWidget(QWidget *parent) :
    QWidget{parent},
    search_field{new QLineEdit(this)},
    forward_button{new QToolButton(this)},
    backward_button{new QToolButton(this)}
{
    search_field->setPlaceholderText(tr("search..."));
    search_field->setToolTip(tr("enter search text"));
    // icons
    QIcon icon = QIcon::fromTheme("go-previous");
    if (icon.isNull())
        // Sometimes name + "-symbolic" is a reasonable fallback icon.
        icon = QIcon::fromTheme("go-previous-symbolic");
    if (icon.isNull())
        backward_button->setText("<");
    else
        backward_button->setIcon(icon);
    backward_button->setToolTip(tr("go to previous matching slide"));
    icon = QIcon::fromTheme("go-next");
    if (icon.isNull())
        // Sometimes name + "-symbolic" is a reasonable fallback icon.
        icon = QIcon::fromTheme("go-next-symbolic");
    if (icon.isNull())
        forward_button->setText(">");
    else
        forward_button->setIcon(icon);
    backward_button->setToolTip(tr("go to next matching slide"));
    // layout
    setMinimumHeight(16);
    search_field->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    backward_button->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
    forward_button->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(search_field);
    layout->addWidget(backward_button);
    layout->addWidget(forward_button);
    setLayout(layout);
    // connections
    connect(search_field, &QLineEdit::returnPressed, this, &SearchWidget::searchCurrent);
    connect(forward_button, &QToolButton::clicked, this, &SearchWidget::searchForward);
    connect(backward_button, &QToolButton::clicked, this, &SearchWidget::searchBackward);
}

SearchWidget::~SearchWidget()
{
    delete search_field;
    delete forward_button;
    delete backward_button;
}

void SearchWidget::search(qint8 forward)
{
    const QString &text = search_field->text();
    emit searchPdf(text, preferences()->page + forward, forward >= 0);
}
