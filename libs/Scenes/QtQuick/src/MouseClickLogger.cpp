#include <Spix/MouseClickLogger.h>

#include "QtItemTools.h"

#include <QGuiApplication>
#include <QLoggingCategory>
#include <QQuickItem>
#include <QQuickWindow>
#include <QStringList>

#include <algorithm>

namespace {
Q_LOGGING_CATEGORY(self, "mouseclicklogger")

// A name matches a NameSelector, "#type" a TypeSelector.
QString PathComponentForObject(QObject* object)
{
    const QString name = spix::qt::GetObjectName(object);
    if (!name.isEmpty()) {
        return name;
    }
    return QString("#") + spix::qt::TypeStringForObject(object);
}
} // namespace

namespace spix {

// Walks the visual tree, like FindQtItem does. Walking the QObject ownership
// tree instead would produce paths Spix can never match.
QString MouseClickLogger::pathForObject(QObject* object)
{
    QStringList components;

    while (object) {
        if (auto* item = qobject_cast<QQuickItem*>(object)) {
            QQuickWindow* window = item->window();

            // Spix starts searching at the contentItem, so that is the root of
            // the path rather than a component of it.
            if (window && item == window->contentItem()) {
                components.append(PathComponentForObject(window));
                break;
            }

            components.append(PathComponentForObject(object));

            if (QQuickItem* parentItem = item->parentItem()) {
                object = parentItem;
                continue;
            }
            // Top of the tree but not the contentItem, e.g. a Window root.
            if (window) {
                components.append(PathComponentForObject(window));
            }
            break;
        }

        // Non-visual QObjects have no visual parent.
        components.append(PathComponentForObject(object));
        object = object->parent();
    }

    std::reverse(components.begin(), components.end());
    return components.join("/");
}

void MouseClickLogger::logPath(QObject* object)
{
    qCInfo(self) << "path:" << pathForObject(object);
}

MouseClickLogger::MouseClickLogger(QObject* parent)
: QObject(parent)
{
}

MouseClickLogger::~MouseClickLogger()
{
    if (m_enabled) {
        detach();
    }
}

bool MouseClickLogger::eventFilter(QObject* watched, QEvent* event)
{
    switch (event->type()) {
    case QEvent::Type::TouchEnd:
    case QEvent::Type::TouchCancel:
    case QEvent::Type::MouseButtonRelease:
        if (watched->inherits("QAbstractButton") || watched->inherits("QQuickItem")
            || watched->inherits("QQuickControl") || watched->inherits("QQuickMouseArea")) {
            const QString path = pathForObject(watched);
            qCInfo(self) << "path:" << path;
            emit itemClicked(path);
        }
        break;
    default:
        break;
    }

    return QObject::eventFilter(watched, event);
}

void MouseClickLogger::attach()
{
    if (auto* app = QCoreApplication::instance()) {
        app->installEventFilter(this);
    }
}

void MouseClickLogger::detach()
{
    if (auto* app = QCoreApplication::instance()) {
        app->removeEventFilter(this);
    }
}

bool MouseClickLogger::enabled() const
{
    return m_enabled;
}

void MouseClickLogger::setEnabled(bool newEnabled)
{
    if (m_enabled == newEnabled)
        return;

    qCInfo(self) << "enabled:" << newEnabled;
    m_enabled = newEnabled;
    emit enabledChanged();

    if (m_enabled)
        attach();
    else
        detach();
}

} // namespace spix
