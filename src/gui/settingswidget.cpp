#include "settingswidget.h"

SettingsWidget::SettingsWidget(QWidget *parent) :
    QTabWidget(parent),
    help(new QTextEdit(this)),
    misc(new QScrollArea(this)),
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
        if (manual.isReadable())
            help->setHtml(manual.readAll());
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
            action_to_string[*it] = it.key();
        const auto &key_actions = preferences()->key_actions;
        for (auto it=key_actions.cbegin(); it!=key_actions.cend(); ++it)
        {
            input_shortcut = new KeyInputLabel(it.key(), *it, shortcuts);
            layout->addRow(action_to_string.value(*it, "unknown"), input_shortcut);
        }
        QMap<int, QString> tool_to_string;
        for (auto it=string_to_tool.cbegin(); it!=string_to_tool.cend(); ++it)
            tool_to_string[*it] = it.key();
        const auto &key_tools = preferences()->key_tools;
        for (auto it=key_tools.cbegin(); it!=key_tools.cend(); ++it)
        {
            input_shortcut = new KeyInputLabel(it.key(), it.value(), shortcuts);
            layout->addRow(tool_to_string.value((*it)->tool(), "unknown"), input_shortcut);
        }
        QPushButton *add_shortcut_button = new QPushButton("Add new shortcut", shortcuts);
        connect(add_shortcut_button, &QPushButton::clicked, this, &SettingsWidget::appendShortcut);
        layout->addRow(add_shortcut_button);
        shortcuts->setLayout(layout);
    }

    // Rendering
    {
        QFormLayout *layout = new QFormLayout();
        QLabel *explanation_label = new QLabel("Set general settings for rendering. Many settings only work after restarting the program.", rendering);
        explanation_label->setWordWrap(true);
        layout->addRow(explanation_label);

        QDoubleSpinBox *memory_box = new QDoubleSpinBox(rendering);
        memory_box->setMinimum(-1.);
        memory_box->setMaximum(4096.);
        memory_box->setValue(preferences()->max_memory/1048596);
        connect(memory_box, QOverload<double>::of(&QDoubleSpinBox::valueChanged), writable_preferences(), &Preferences::setMemory);
        layout->addRow("cache memory (MiB)", memory_box);

        QSpinBox *spin_box = new QSpinBox(rendering);
        spin_box->setMinimum(-1);
        spin_box->setMaximum(10000);
        spin_box->setValue(preferences()->max_cache_pages);
        connect(spin_box, QOverload<int>::of(&QSpinBox::valueChanged), writable_preferences(), &Preferences::setCacheSize);
        layout->addRow("max. slides in cache", spin_box);

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

        QDoubleSpinBox *page_part_box = new QDoubleSpinBox(rendering);
        page_part_box->setMinimum(1.);
        page_part_box->setMaximum(20.);
        page_part_box->setValue(preferences()->page_part_threshold);
        connect(page_part_box, QOverload<double>::of(&QDoubleSpinBox::valueChanged), writable_preferences(), &Preferences::setPagePartThreshold);
        layout->addRow("Page part threshold", page_part_box);

        QLineEdit *line_edit = new QLineEdit(rendering);
        line_edit->setText(preferences()->rendering_command);
        connect(line_edit, &QLineEdit::textChanged, writable_preferences(), &Preferences::setRenderingCommand);
        layout->addRow("rendering command", line_edit);

        line_edit = new QLineEdit(rendering);
        line_edit->setText(preferences()->rendering_arguments.join(","));
        connect(line_edit, &QLineEdit::textChanged, writable_preferences(), &Preferences::setRenderingArguments);
        layout->addRow("rendering arguments", line_edit);

        rendering->setLayout(layout);
    }

    // MISC
    {
        QFormLayout *layout = new QFormLayout();
        QLabel *explanation_label = new QLabel("Various settings.", misc);
        explanation_label->setWordWrap(true);
        layout->addRow(explanation_label);

        QSpinBox *spin_box = new QSpinBox(misc);
        spin_box->setValue(preferences()->history_length_visible_slides);
        connect(spin_box, QOverload<int>::of(&QSpinBox::valueChanged), writable_preferences(), &Preferences::setHistoryVisibleSlide);
        spin_box->setMinimum(0);
        spin_box->setMaximum(1000);
        layout->addRow("History length visible slides", spin_box);

        spin_box = new QSpinBox(misc);
        spin_box->setValue(preferences()->history_length_hidden_slides);
        connect(spin_box, QOverload<int>::of(&QSpinBox::valueChanged), writable_preferences(), &Preferences::setHistoryHiddenSlide);
        spin_box->setMinimum(0);
        spin_box->setMaximum(1000);
        layout->addRow("History length hidden slides", spin_box);

        QCheckBox *box = new QCheckBox("log slide changes to standard output", misc);
        box->setChecked(preferences()->global_flags & Preferences::LogSlideChanges);
        connect(box, QOverload<bool>::of(&QCheckBox::clicked), writable_preferences(), &Preferences::setLogSlideChanges);
        layout->addRow(box);

        box = new QCheckBox("automatic slide changes", misc);
        box->setChecked(preferences()->global_flags & Preferences::ShowAnimations);
        connect(box, QOverload<bool>::of(&QCheckBox::clicked), writable_preferences(), &Preferences::setShowAnimations);
        layout->addRow(box);

        QComboBox *combo_box = new QComboBox(misc);
        for (auto it=string_to_overlay_mode.cbegin(); it!=string_to_overlay_mode.cend(); ++it)
            combo_box->addItem(it.key());
        combo_box->setCurrentText(string_to_overlay_mode.key(preferences()->overlay_mode));
        connect(combo_box, &QComboBox::currentTextChanged, writable_preferences(), &Preferences::setOverlayMode);
        layout->addRow("drawing mode", combo_box);

        misc->setLayout(layout);
    }

    addTab(help, "help");
    addTab(misc, "misc");
    addTab(rendering, "rendering");
    addTab(shortcuts, "shortcuts");
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
