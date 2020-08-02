#include "src/master.h"
#include "src/pdfmaster.h"
#include "src/slidescene.h"
#include "src/gui/containerwidget.h"

Master::Master()
{

}

Master::~Master()
{
    qDeleteAll(caches);
    for (const auto doc : documents)
    {
        qDeleteAll(doc->getScenes());
        doc->getScenes().clear();
    }
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
        const auto pair = createWidget(obj, nullptr);
        if (pair.first != nullptr)
            windows.append(pair.first);
    }

    // Return true (success) if at least one window and one document were created.
    return !windows.isEmpty() && !documents.isEmpty();
}

QPair<QWidget*, GuiWidget*> Master::createWidget(QJsonObject &object, ContainerWidget *parent)
{
    if (!object.contains("type"))
    {
        if (object.contains("children"))
            object.insert("type", "container");
        else
        {
            qCritical() << "Ignoring entry in GUI config without type.";
            return {nullptr, nullptr};
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
            const auto pair = createWidget(obj, widget);
            layout->addWidget(pair.first);
            widget->addGuiWidget(pair.second);
        }
        widget->setLayout(layout);
        const qreal height = object.value("height").toDouble(1.);
        const qreal width = object.value("width").toDouble(1.);
        widget->setPreferedSize(QSizeF(width, height));
        return {widget, widget};
    }
    case GuiWidget::StackedWidget:
        break;
    case GuiWidget::Slide:
    {
        // Calculate the shift for scene.
        int shift = object.value("shift").toInt() & ~ShiftOverlays::AnyOverlay;
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
            return {nullptr, nullptr};
        }
        const QString page_part_str = object.value("page part").toString().toLower();
        PagePart page_part = FullPage;
        if (page_part_str == "left")
            page_part = LeftHalf;
        else if (page_part_str == "right")
            page_part = RightHalf;

        // Check whether the PDF has been loaded already, load it if necessary.
        PdfMaster *doc = nullptr;
        SlideScene *scene = nullptr;
        for (auto docit = documents.cbegin(); docit != documents.cend(); ++docit)
        {
            if ((*docit)->getFilename() == file)
            {
                doc = *docit;
                if (preferences().page_part_threshold > 0.)
                {
                    const QSizeF reference = doc->getPageSize(0);
                    if (reference.width() < preferences().page_part_threshold * reference.height())
                        page_part = FullPage;
                }
                break;
            }
        }
        if (doc == nullptr)
        {
            doc = new PdfMaster(file);
            connect(this, &Master::sendAction, doc, &PdfMaster::receiveAction);
            connect(doc, &PdfMaster::nagivationSignal, this, &Master::navigationSignal);
            connect(this, &Master::navigationSignal, doc, &PdfMaster::distributeNavigationEvents);
            if (preferences().page_part_threshold > 0.)
            {
                const QSizeF reference = doc->getPageSize(0);
                if (reference.width() < preferences().page_part_threshold * reference.height())
                    page_part = FullPage;
            }
            if (object.value("master").toBool())
                documents.prepend(doc);
            else
                documents.append(doc);
        }
        else {
            // If PDF files existed before, check whether we need a new SlideScene.
            for (auto &sceneit : doc->getScenes())
            {
                if (sceneit->identifier() == qHash(QPair<int, const void*>(shift, doc)) + page_part)
                {
                    scene = sceneit;
                    break;
                }
            }
        }

        // Create new slide scene if necessary.
        if (scene == nullptr)
        {
            scene = new SlideScene(doc, page_part, parent);
            if (shift != 0)
                scene->setPageShift(shift);
            if (object.value("master").toBool())
                doc->getScenes().prepend(scene);
            else
                doc->getScenes().append(scene);
            connect(this, &Master::sendAction, scene, &SlideScene::receiveAction);
        }
        else if (object.value("master").toBool())
            doc->getScenes().swapItemsAt(doc->getScenes().indexOf(scene), 0);
        // TODO: read other properties from config

        // Get or create cache object.
        PixCache *pixcache = nullptr;
        int cache_hash = object.value("cache hash").toInt(-1);
        if (cache_hash == -1)
        {
            // -1 is the "default hash" and indicates that a new object has to
            // be created.
            // Set hash to -2 or smaller.
            cache_hash = (caches.isEmpty() || caches.firstKey() >= 0) ? -2 : caches.firstKey() - 1;
        }
        else
        {
            // Check if a PixCache object with the given hash already exists.
            pixcache = caches.value(cache_hash, nullptr);
        }
        // If necessary, create a new PixCache object an store it in caches.
        if (pixcache == nullptr)
        {
            // Read number of threads from GUI config.
            const int threads = object.value("threads").toInt(1);
            // Create the PixCache object.
            pixcache = new PixCache(scene->getPdfMaster()->getDocument(), threads, page_part);
            // Move the PixCache object to an own thread.
            pixcache->moveToThread(new QThread());
            // Make sure that pixcache is initialized when the thread is started.
            connect(pixcache->thread(), &QThread::started, pixcache, &PixCache::init);
            connect(this, &Master::navigationSignal, pixcache, &PixCache::pageNumberChanged);
            // Set maximum number of pages in cache from settings.
            pixcache->setMaxNumber(preferences().max_cache_pages);
            // Start the thread.
            pixcache->thread()->start();
            // Keep the new pixcache in caches.
            caches[cache_hash] = pixcache;
        }

        // Create slide view.
        SlideView *slide = new SlideView(scene, pixcache, parent);
        connect(slide, &SlideView::sendKeyEvent, this, &Master::receiveKeyEvent);
        connect(scene, &SlideScene::navigationToViews, slide, &SlideView::pageChanged);
        // Check layout.
        const qreal height = object.value("height").toDouble(1.);
        const qreal width = object.value("width").toDouble(1.);
        slide->setPreferedSize(QSizeF(width, height));
        return {slide, slide};
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
        return {nullptr, nullptr};
    }
    qWarning() << "Requested GUI type is not implemented yet:" << object.value("type");
    return {nullptr, nullptr};
}

void Master::showAll() const
{
    for (auto &widget : windows)
    {
        widget->setGeometry(0, 0, 400, 300);
        widget->show();
    }
}

void Master::receiveKeyEvent(const QKeyEvent* event)
{
    const QMultiMap<quint32, Action> &key_actions = preferences().key_actions;
    auto it = key_actions.constFind(event->key() | (event->modifiers() & ~Qt::KeypadModifier));
    while (it != key_actions.cend())
    {
#ifdef DEBUG_KEY_ACTIONS
        qDebug() << "Global key action:" << it.value();
        qDebug() << "Cache:" << getTotalCache();
#endif
        switch (it.value())
        {
        case InvalidAction:
        case NoAction:
            break;
        case NextPage:
            if (documents.first()->numberOfPages() > preferences().page + 1)
            {
                ++writable_preferences().page;
                emit navigationSignal(preferences().page);
            }
            break;
        case PreviousPage:
            if (preferences().page > 0)
            {
                --writable_preferences().page;
                emit navigationSignal(preferences().page);
            }
            break;
        case NextSkippingOverlays:
            writable_preferences().page = documents.first()->overlaysShifted(preferences().page, 1 | FirstOverlay);
            emit navigationSignal(preferences().page);
            break;
        case PreviousSkippingOverlays:
            writable_preferences().page = documents.first()->overlaysShifted(preferences().page, -1 & ~FirstOverlay);
            emit navigationSignal(preferences().page);
            break;
        case FirstPage:
            writable_preferences().page = 0;
            emit navigationSignal(0);
            break;
        case LastPage:
            writable_preferences().page = documents.first()->numberOfPages() - 1;
            emit navigationSignal(preferences().page);
            break;
        case Quit:
            for (auto window : windows)
                window->close();
            break;
        case ReloadFiles:
            for (auto doc : documents)
                doc->loadDocument();
            break;
        default:
            emit sendAction(it.value());
        }
        if ((++it).key() != static_cast<unsigned int>(event->key()))
            break;
    }
}

void Master::distributeMemory()
{
    if (preferences().max_memory < 0)
        return;
    float scale = 0.;
    for (const auto cache : caches)
        scale += cache->getPixels();
    if (scale <= 0)
        return;
    scale = preferences().max_memory / scale;
    for (const auto cache : caches)
        cache->setScaledMemory(scale);
}

qint64 Master::getTotalCache() const
{
    qint64 cache = 0;
    for (const auto px : caches)
        cache += px->getUsedMemory();
    return cache;
}
