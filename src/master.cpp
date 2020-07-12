#include "src/master.h"
#include "src/pdfmaster.h"
#include "src/slidescene.h"
#include "src/gui/containerwidget.h"

Master::Master()
{

}

Master::~Master()
{
    qDeleteAll(windows);
    qDeleteAll(documents);
}

bool Master::readGuiConfig(const QString &filename)
{
    // Read file into JSON document
    QFile file(filename);
    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qCritical() << "Could not read GUI config:" << filename;
        return false;
    }
    QJsonParseError *error = nullptr;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), error);
    if (error != nullptr)
    {
        qCritical() << "Parsing GUI config failed:" << error->errorString();
        return false;
    }
    if (doc.isNull() || doc.isEmpty())
    {
        qCritical() << "GUI config file is empty or parsing failed";
        return false;
    }
    if (!doc.isArray())
    {
        qCritical() << "Interpreting GUI file failed: must be a JSON array";
        return false;
    }
    const QJsonArray array = doc.array();
    for (auto it = array.cbegin(); it != array.cend(); ++it)
    {
        if (it->type() != QJsonValue::Type::Object)
        {
            qCritical() << "Ignoring invariant entry in GUI config.";
            continue;
        }
        QJsonObject obj = it->toObject();
        // Start recursive creation of widgets.
        QWidget* widget = createWidget(obj, nullptr);
        if (widget != nullptr)
            windows.append(widget);
    }
    // Return true (success) if at least one window and one document were created.
    return !windows.isEmpty() && !documents.isEmpty();
}

QWidget* Master::createWidget(QJsonObject &object, ContainerWidget *parent)
{
    if (!object.contains("type"))
    {
        if (object.contains("children"))
            object.insert("type", "container");
        else
        {
            qCritical() << "Ignoring entry in GUI config without type.";
            return nullptr;
        }
    }
    switch (string_to_widget_type.value(object.value("type").toString(), GuiWidget::InvalidType))
    {
    case GuiWidget::ContainerWidget:
    {
        ContainerWidget *widget = new ContainerWidget(parent);
        // TODO: connect
        const QString layoutString = object.value("layout").toString();
        QLayout* layout;
        if (layoutString.isNull() || layoutString == "horizontal")
            layout = new QHBoxLayout(widget);
        else if (layoutString == "vertical")
            layout = new QVBoxLayout(widget);
        else
        {
            qWarning() << "Could not understand layout";
            layout = new QHBoxLayout(widget);
        }
        const QJsonArray array = object.value("children").toArray();
        for (auto it = array.cbegin(); it != array.cend(); ++it)
        {
            if (it->type() != QJsonValue::Type::Object)
            {
                qCritical() << "Ignoring invariant entry in GUI config.";
                continue;
            }
            QJsonObject obj = it->toObject();
            // Create child widgets recursively
            layout->addWidget(createWidget(obj, widget));
        }
        widget->setLayout(layout);
        return widget;
    }
    case GuiWidget::StackedWidget:
        break;
    case GuiWidget::Slide:
    {
        // Calculate the shift for scene.
        int shift = object.value("shift").toInt();
        const QString overlays = object.value("overlays").toString();
        if (overlays == "first")
            shift |= ShiftOverlays::FirstOverlay;
        else if (overlays == "last")
            shift |= ShiftOverlays::LastOverlay;

        // Find PDF file name.
        // Usually "file" will be "presentation" or "notes". These are mapped to
        // filenames by this->files.
        QString file = object.value("file").toString();
        if (file.isNull())
            file = "presentation";
        file = preferences().file_alias.value(file, file);
        if (!QFile::exists(file))
        {
            qCritical() << "Could not load PDF file: Does not exist." << file;
            return nullptr;
        }

        // Check whether the PDF has been loaded already, load it if necessary.
        PdfMaster* doc = nullptr;
        SlideScene *scene = nullptr;
        for (auto docit = documents.cbegin(); docit != documents.cend(); ++docit)
        {
            if ((*docit)->getFilename() == file)
            {
                doc = *docit;
                break;
            }
        }
        if (doc == nullptr)
        {
            doc = new PdfMaster(file);
            connect(this, &Master::sendAction, doc, &PdfMaster::receiveAction);
            if (object.value("master").toBool())
                documents.prepend(doc);
            else
                documents.append(doc);
        }
        else {
            // If PDF files existed before, check whether we need a new SlideScene.
            for (auto &sceneit : scenes)
            {
                if (sceneit->identifier() == qHash(QPair<int, const void*>(shift, doc)))
                {
                    scene = sceneit;
                    break;
                }
            }
        }
        // Create new slide scene if necessary.
        if (scene == nullptr)
        {
            scene = new SlideScene(doc, parent);
            if (shift != 0)
                scene->setPageShift(shift);
            if (object.value("master").toBool())
                scenes.prepend(scene);
            else
                scenes.append(scene);
            connect(this, &Master::sendAction, scene, &SlideScene::receiveAction);
            connect(this, &Master::navigationSignal, scene, &SlideScene::navigationEvent);
        }
        else if (object.value("master").toBool())
            scenes.swapItemsAt(scenes.indexOf(scene), 0);
        // TODO: read other properties from config
        // Create slide view.
        SlideView *slide = new SlideView(scene, nullptr, parent);
        connect(slide, &SlideView::sendKeyEvent, this, &Master::receiveKeyEvent);
        connect(scene, &SlideScene::navigationToViews, slide, &SlideView::pageChanged);
        return slide;
    }
    case GuiWidget::Overview:
        break;
    case GuiWidget::TOC:
        break;
    case GuiWidget::Notes:
        break;
    case GuiWidget::Button:
        break;
    case GuiWidget::ToolSelector:
        break;
    case GuiWidget::Settings:
        break;
    case GuiWidget::Clock:
        break;
    case GuiWidget::Timer:
        break;
    case GuiWidget::SlideNumber:
        break;
    case GuiWidget::InvalidType:
        qCritical() << "Ignoring entry in GUI config with invalid type:" << object.value("type");
        return nullptr;
    }
    qWarning() << "Requested GUI type is not implemented yet:" << object.value("type");
    return nullptr;
}

void Master::showAll() const
{
    for (auto &widget : windows)
        widget->show();
}

void Master::receiveKeyEvent(const QKeyEvent* event)
{
    const QMultiMap<quint32, Action> &key_actions = preferences().key_actions;
    auto it = key_actions.constFind(event->key());
    while (it != key_actions.cend())
    {
#ifdef DEBUG_KEY_ACTIONS
        qDebug() << "Global key action:" << it.value();
#endif
        switch (it.value())
        {
        case InvalidAction:
            break;
        case Next:
            if (documents.first()->numberOfPages() > preferences().page + 1)
            {
                ++writable_preferences().page;
                emit navigationSignal(preferences().page);
            }
            break;
        case Previous:
            if (preferences().page > 0)
            {
                --writable_preferences().page;
                emit navigationSignal(preferences().page);
            }
            break;
        default:
            emit sendAction(it.value());
        }
        if ((++it).key() != static_cast<unsigned int>(event->key()))
            break;
    }
}
