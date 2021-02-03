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
        QFile manual(preferences()->manual_file);
        manual.open(QFile::ReadOnly | QFile::Text);
        // TODO: html-formatted help, which can also be read if markdown is
        // not supported.
        if (manual.isReadable())
#ifdef QT_FEATURE_textmarkdownreader
            help->setMarkdown(manual.readAll());
#else
            help->setPlainText(manual.readAll());
#endif
    }

    // Shortcuts
    {
        QFormLayout *layout = new QFormLayout();
        QLabel *explanation_label = new QLabel("Change shortcuts by clicking on them and typing the new shortcut. Remove with delete key.", shortcuts);
        explanation_label->setWordWrap(true);
        layout->addRow(explanation_label);
        KeyInputLabel *input_shortcut;
        QMap<int, QString> action_to_string;
        for (auto it=string_to_action_map.cbegin(); it!=string_to_action_map.cend(); ++it)
            action_to_string[it.value()] = it.key();
        auto &key_actions = writable_preferences()->key_actions;
        for (auto it=key_actions.cbegin(); it!=key_actions.cend(); ++it)
        {
            input_shortcut = new KeyInputLabel(it.key(), it.value(), shortcuts);
            layout->addRow(action_to_string.value(it.value(), "unknown"), input_shortcut);
        }
        QPushButton *add_shortcut_button = new QPushButton("Add new shortcut", shortcuts);
        connect(add_shortcut_button, &QPushButton::clicked, this, &SettingsWidget::appendShortcut);
        layout->addRow(add_shortcut_button);
        shortcuts->setLayout(layout);
    }

    // Rendering
    {
        QFormLayout *layout = new QFormLayout();
        QLineEdit *lineedit = new QLineEdit(rendering);
        lineedit->setText(QString::number(preferences()->max_memory/1048596));
        connect(lineedit, &QLineEdit::textChanged, writable_preferences(), &Preferences::setMemory);
        layout->addRow("cache memory (MiB)", lineedit);

        lineedit = new QLineEdit(rendering);
        lineedit->setText(QString::number(preferences()->max_cache_pages));
        connect(lineedit, &QLineEdit::textChanged, writable_preferences(), &Preferences::setCacheSize);
        layout->addRow("max. slides in cache", lineedit);

        QComboBox *select_renderer = new QComboBox(rendering);
#ifdef INCLUDE_MUPDF
        select_renderer->addItem("MuPDF", PdfDocument::MuPdfEngine);
#endif
#ifdef INCLUDE_POPPLER
        select_renderer->addItem("poppler", PdfDocument::PopplerEngine);
#endif
#ifdef INCLUDE_MUPDF
        select_renderer->addItem("MuPDF + external", PdfDocument::MuPdfEngine);
#endif
#ifdef INCLUDE_POPPLER
        select_renderer->addItem("poppler + external", PdfDocument::PopplerEngine);
#endif
        connect(select_renderer, &QComboBox::currentTextChanged, writable_preferences(), &Preferences::setRenderer);
        layout->addRow("Renderer (requires restart)", select_renderer);

        rendering->setLayout(layout);
    }

    addTab(help, "help");
    addTab(general, "general");
    addTab(shortcuts, "shortcuts");
    addTab(rendering, "rendering");
}

void SettingsWidget::appendShortcut()
{
    QComboBox *select_menu = new QComboBox(shortcuts);
    select_menu->addItem("tool...");
    for (auto it=string_to_action_map.cbegin(); it!=string_to_action_map.cend(); ++it)
        select_menu->addItem(it.key());
    select_menu->setCurrentText("update");
    KeyInputLabel *input_shortcut = new KeyInputLabel(0, Action::InvalidAction, shortcuts);
    QFormLayout *layout = static_cast<QFormLayout*>(shortcuts->layout());
    connect(select_menu, &QComboBox::currentTextChanged, input_shortcut, &KeyInputLabel::changeAction);
    layout->addRow(select_menu, input_shortcut);
}
