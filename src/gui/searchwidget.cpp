// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include <QLineEdit>
#include <QPair>
#include <QRectF>
#include <QPushButton>
#include <QHBoxLayout>
#include "src/gui/searchwidget.h"
#include "src/preferences.h"
#include "src/log.h"

SearchWidget::SearchWidget(QWidget *parent) :
    QWidget{parent},
    search_field{new QLineEdit("search...", this)},
    forward_button{new QPushButton("next", this)},
    backward_button{new QPushButton("prev", this)}
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(search_field, 6);
    layout->addWidget(forward_button, 1);
    layout->addWidget(backward_button, 1);
    setLayout(layout);
    connect(search_field, &QLineEdit::returnPressed, this, &SearchWidget::searchCurrent);
    connect(forward_button, &QPushButton::clicked, this, &SearchWidget::searchForward);
    connect(backward_button, &QPushButton::clicked, this, &SearchWidget::searchBackward);
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
