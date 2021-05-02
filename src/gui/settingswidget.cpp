#include <QCheckBox>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QTextEdit>
#include <QScrollArea>
#include <QFormLayout>
#include <QScroller>
#include "src/gui/settingswidget.h"
#include "src/names.h"
#include "src/preferences.h"
#include "src/gui/keyinputlabel.h"

SettingsWidget::SettingsWidget(QWidget *parent) :
    QTabWidget(parent),
    manual(new QTextEdit(this)),
    misc(new QWidget(this)),
    shortcuts(new QWidget(this)),
    rendering(new QWidget(this))
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumSize(30, 20);

    initManual();
    initMisc();
    initShortcuts();
    initRendering();

    addTab(manual, "help");
    QScroller::grabGesture(manual);
    QScrollArea *scroll_area = new QScrollArea(this);
    scroll_area->setWidget(misc);
    addTab(scroll_area, "misc");
    QScroller::grabGesture(scroll_area);
    scroll_area = new QScrollArea(this);
    scroll_area->setWidget(rendering);
    addTab(scroll_area, "rendering");
    QScroller::grabGesture(scroll_area);
    scroll_area = new QScrollArea(this);
    scroll_area->setWidget(shortcuts);
    addTab(scroll_area, "shortcuts");
    QScroller::grabGesture(scroll_area);
}

void SettingsWidget::initManual()
{
    manual->setReadOnly(true);
    QFile file(preferences()->manual_file);
    file.open(QFile::ReadOnly | QFile::Text);
    if (file.isReadable())
        manual->setHtml(file.readAll());
    else
        manual->setPlainText("Manual file not found or not readable:\n" + preferences()->manual_file);
}

void SettingsWidget::initShortcuts()
{
    QFormLayout *layout = new QFormLayout();
    QLabel *explanation_label = new QLabel(
                "Change shortcuts by clicking on them and typing the new shortcut. "
                "Remove shortcuts with delete key. "
                "Actions are documented in man 1 beamerpresenter-ui (in \"tool selector\").",
                shortcuts
            );
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
        QLabel *label = new QLabel(action_to_string.value(*it, "unknown"), shortcuts);
        label->setToolTip(action_to_description.value(*it));
        layout->addRow(label, input_shortcut);
    }
    QMap<int, QString> tool_to_string;
    for (auto it=string_to_tool.cbegin(); it!=string_to_tool.cend(); ++it)
        tool_to_string[*it] = it.key();
    const auto &key_tools = preferences()->key_tools;
    for (auto it=key_tools.cbegin(); it!=key_tools.cend(); ++it)
    {
        input_shortcut = new KeyInputLabel(it.key(), *it, shortcuts);
        QLabel *label = new QLabel(tool_to_string.value((*it)->tool(), "unknown"), shortcuts);
        label->setToolTip(tool_to_description.value((*it)->tool()));
        layout->addRow(label, input_shortcut);
    }
    QPushButton *add_shortcut_button = new QPushButton("Add new shortcut", shortcuts);
    connect(add_shortcut_button, &QPushButton::clicked, this, &SettingsWidget::appendShortcut);
    layout->addRow(add_shortcut_button);

    shortcuts->setLayout(layout);
}

void SettingsWidget::initRendering()
{
    QFormLayout *layout = new QFormLayout();

    // Cache management
    QLabel *explanation_label = new QLabel(
                "Set general settings for rendering. Many oft these settings only take effect after restarting the program.\n\n"
                "Configure cache. Slides are rendered to compressed cache. These settings defined the allowed cache size. Negative values are interpreted as infinity."
                ,
                rendering);
    explanation_label->setTextFormat(Qt::PlainText);
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

    // Renderer
    explanation_label = new QLabel("Depending on your installation, different PDF engines may be available. Note that using an external renderer requires a proper configuration of rendering command and rendering arguments as documented in man 5 beamerpresenter.conf.", misc);
    explanation_label->setTextFormat(Qt::PlainText);
    explanation_label->setWordWrap(true);
    layout->addRow(explanation_label);

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

    QLineEdit *line_edit = new QLineEdit(rendering);
    line_edit->setText(preferences()->rendering_command);
    connect(line_edit, &QLineEdit::textChanged, writable_preferences(), &Preferences::setRenderingCommand);
    layout->addRow("rendering command", line_edit);

    line_edit = new QLineEdit(rendering);
    line_edit->setText(preferences()->rendering_arguments.join(","));
    connect(line_edit, &QLineEdit::textChanged, writable_preferences(), &Preferences::setRenderingArguments);
    layout->addRow("rendering arguments", line_edit);

    // Page part threshold
    explanation_label = new QLabel("Some programs (like LaTeX beamer) can create PDF pages split into one half for the audience one half for the speaker. This is assumed by BeamerPresenter if the aspect ratio (width/height) of the first slide lies above this threshold:", misc);
    explanation_label->setTextFormat(Qt::PlainText);
    explanation_label->setWordWrap(true);
    layout->addRow(explanation_label);

    QDoubleSpinBox *page_part_box = new QDoubleSpinBox(rendering);
    page_part_box->setMinimum(1.);
    page_part_box->setMaximum(20.);
    page_part_box->setValue(preferences()->page_part_threshold);
    connect(page_part_box, QOverload<double>::of(&QDoubleSpinBox::valueChanged), writable_preferences(), &Preferences::setPagePartThreshold);
    layout->addRow("page part threshold", page_part_box);

    rendering->setLayout(layout);
}

void SettingsWidget::initMisc()
{
    QFormLayout *layout = new QFormLayout();

    // Drawing history settings
    QLabel *explanation_label = new QLabel("Number of drawing history steps (undo/redo). Drawing history is kept separately for each slide.", misc);
    explanation_label->setTextFormat(Qt::PlainText);
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

    // Enable/disable logging output
    explanation_label = new QLabel("If opened in a terminal, slide changes can be logged to standard output with a time stamp.", misc);
    explanation_label->setTextFormat(Qt::PlainText);
    explanation_label->setWordWrap(true);
    layout->addRow(explanation_label);

    QCheckBox *box = new QCheckBox("log slide changes", misc);
    box->setChecked(preferences()->global_flags & Preferences::LogSlideChanges);
    connect(box, QOverload<bool>::of(&QCheckBox::clicked), writable_preferences(), &Preferences::setLogSlideChanges);
    layout->addRow(box);

    // Enable/disable automatic slide changes
    explanation_label = new QLabel("Enable/disable automatic slide switching if durations for slides are defined in the PDF.", misc);
    explanation_label->setTextFormat(Qt::PlainText);
    explanation_label->setWordWrap(true);
    layout->addRow(explanation_label);

    box = new QCheckBox("automatic slide changes", misc);
    box->setChecked(preferences()->global_flags & Preferences::AutoSlideChanges);
    connect(box, QOverload<bool>::of(&QCheckBox::clicked), writable_preferences(), &Preferences::setAutoSlideChanges);
    layout->addRow(box);

    // Drawing mode
    explanation_label = new QLabel(
                "<br>Define how drawings should be handled if multiple successive pages share the same page label (e.g. because they show overlays of the same slide).\n"
                "<ul>"
                "<li>\"per page\": all pages are treated separately</li>\n"
                "<li>\"per slide\": pages with the same label also have the same drawings.</li>\n"
                "<li>\"cumulative\": when reaching a page with no drawings (and no drawings history), which has the same label as the previous page, the drawings from the previous page are copied to this page.</li>"
                "</ul>",
                misc);
    explanation_label->setTextFormat(Qt::RichText);
    explanation_label->setWordWrap(true);
    layout->addRow(explanation_label);

    QComboBox *combo_box = new QComboBox(misc);
    for (auto it=string_to_overlay_mode.cbegin(); it!=string_to_overlay_mode.cend(); ++it)
        combo_box->addItem(it.key());
    combo_box->setCurrentText(string_to_overlay_mode.key(preferences()->overlay_mode));
    connect(combo_box, &QComboBox::currentTextChanged, writable_preferences(), &Preferences::setOverlayMode);
    layout->addRow("drawing mode for overlays", combo_box);

    misc->setLayout(layout);
}

void SettingsWidget::appendShortcut()
{
    QComboBox *select_menu = new QComboBox(shortcuts);
    select_menu->addItem("tool...");
    for (auto it=string_to_action_map.cbegin(); it!=string_to_action_map.cend(); ++it)
        select_menu->addItem(it.key());
    select_menu->setCurrentText("update");
    select_menu->setEditable(true);
    KeyInputLabel *input_shortcut = new KeyInputLabel(0, Action::InvalidAction, shortcuts);
    QFormLayout *layout = static_cast<QFormLayout*>(shortcuts->layout());
#if (QT_VERSION >= QT_VERSION_CHECK(5,14,0))
    connect(select_menu, &QComboBox::textActivated, input_shortcut, &KeyInputLabel::changeAction);
#else
    connect(select_menu, &QComboBox::currentTextChanged, input_shortcut, &KeyInputLabel::changeAction);
#endif
    connect(input_shortcut, &KeyInputLabel::sendName, select_menu, &QComboBox::setEditText);
    layout->addRow(select_menu, input_shortcut);
}
