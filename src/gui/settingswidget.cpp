#include "settingswidget.h"

SettingsWidget::SettingsWidget(QWidget *parent) :
    QTabWidget(parent),
    help(new QTextEdit(this)),
    general(new QScrollArea(this)),
    shortcuts(new QScrollArea(this)),
    rendering(new QScrollArea(this))
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumSize(30, 20);

    // Help
    {
        help->setReadOnly(true);
        QFile manual(preferences().manual_file);
        manual.open(QFile::ReadOnly | QFile::Text);
        if (manual.isReadable())
            help->setMarkdown(manual.readAll());
    }

    // Shortcuts
    {
        QFormLayout *layout = new QFormLayout();
        QLabel *explanation_label = new QLabel("Change shortcuts by clicking on them and typing the new shortcut. Remove with delete key.", shortcuts);
        explanation_label->setWordWrap(true);
        layout->addRow(explanation_label);
        KeyInputLabel *input_shortcut;
        QMap<Action, QString> action_to_string;
        for (auto it=string_to_action_map.cbegin(); it!=string_to_action_map.cend(); ++it)
            action_to_string[it.value()] = it.key();
        auto &key_actions = writable_preferences().key_actions;
        for (auto it=key_actions.cbegin(); it!=key_actions.cend(); ++it)
        {
            input_shortcut = new KeyInputLabel(it.key(), it.value(), shortcuts);
            layout->addRow(action_to_string.value(it.value(), "unknown"), input_shortcut);
        }
        shortcuts->setLayout(layout);
        QPushButton *add_shortcut_button = new QPushButton("Add new shortcut", shortcuts);
        connect(add_shortcut_button, &QPushButton::clicked, this, &SettingsWidget::appendShortcut);
        layout->addRow(add_shortcut_button);
    }

    addTab(help, "help");
    addTab(general, "general");
    addTab(shortcuts, "shortcuts");
    addTab(rendering, "rendering");
}

void SettingsWidget::appendShortcut()
{
    QComboBox *select_menu = new QComboBox(shortcuts);
    for (auto it=string_to_action_map.cbegin(); it!=string_to_action_map.cend(); ++it)
        select_menu->addItem(it.key(), it.value());
    KeyInputLabel *input_shortcut = new KeyInputLabel(0, Action::InvalidAction, shortcuts);
    QFormLayout *layout = static_cast<QFormLayout*>(shortcuts->layout());
    connect(select_menu, &QComboBox::currentTextChanged, input_shortcut, &KeyInputLabel::changeAction);
    layout->addRow(select_menu, input_shortcut);
}
