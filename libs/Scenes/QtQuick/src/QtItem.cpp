/***
 * Copyright (C) Falko Axmann. All rights reserved.
 * Licensed under the MIT license.
 * See LICENSE.txt file in the project root for full license information.
 ****/

#include "QtItem.h"

#include <QMetaEnum>
#include <QMetaObject>
#include <QMetaProperty>
#include <QQuickItem>
#include <QQuickWindow>
#include <QRectF>
#include <QVariant>

#include <QtItemTools.h>

namespace {

// The moving content holder of a QQuickFlickable (its `contentItem` property).
QQuickItem* flickableContentItem(QQuickItem* flickable)
{
    return flickable->property("contentItem").value<QQuickItem*>();
}

// Which axes the flickable is allowed to scroll, read from its
// `flickableDirection` enum via the meta-object (so we depend on neither the
// private QQuickFlickable header nor hard-coded enum values). Unknown/auto
// directions leave both axes allowed, to be gated by actual overflow.
void flickAxesAllowed(QQuickItem* flickable, bool& horizAllowed, bool& vertAllowed)
{
    horizAllowed = true;
    vertAllowed = true;

    const QMetaObject* meta = flickable->metaObject();
    const int propIndex = meta->indexOfProperty("flickableDirection");
    if (propIndex < 0) {
        return;
    }
    const QMetaEnum metaEnum = meta->property(propIndex).enumerator();
    if (!metaEnum.isValid()) {
        return;
    }

    const int current = flickable->property("flickableDirection").toInt();
    const int verticalOnly = metaEnum.keyToValue("VerticalFlick");
    const int horizontalOnly = metaEnum.keyToValue("HorizontalFlick");

    if (verticalOnly >= 0 && current == verticalOnly) {
        horizAllowed = false;
    }
    if (horizontalOnly >= 0 && current == horizontalOnly) {
        vertAllowed = false;
    }
}

// Adjust a Flickable's contentX/contentY so that `target`'s rectangle lies
// inside the flickable's viewport, scrolling as little as necessary. Works for
// any QQuickFlickable subclass (Flickable, ListView, GridView, ScrollView's
// inner flickable), since they all share the contentX/contentY offset.
void scrollFlickableToShow(QQuickItem* flickable, QQuickItem* target)
{
    auto* content = flickableContentItem(flickable);
    if (!content) {
        return;
    }

    // target rectangle expressed in the flickable's content coordinates
    const QRectF r = target->mapRectToItem(content, QRectF(0, 0, target->width(), target->height()));

    const qreal viewportW = flickable->width();
    const qreal viewportH = flickable->height();
    const qreal contentW = flickable->property("contentWidth").toReal();
    const qreal contentH = flickable->property("contentHeight").toReal();

    // Only move an axis that this flickable is actually allowed to scroll AND
    // that has real overflow. This keeps a vertical-only flickable from being
    // nudged horizontally (and vice versa) even if it reports a content size
    // slightly larger than its viewport (margins, scrollbar insets, ...).
    bool horizAllowed = true;
    bool vertAllowed = true;
    flickAxesAllowed(flickable, horizAllowed, vertAllowed);

    const qreal overflowEpsilon = 1.0;
    const bool scrollX = horizAllowed && (contentW > viewportW + overflowEpsilon);
    const bool scrollY = vertAllowed && (contentH > viewportH + overflowEpsilon);

    if (scrollX) {
        qreal contentX = flickable->property("contentX").toReal();
        qreal newX = contentX;
        if (r.left() < contentX) {
            newX = r.left();
        } else if (r.right() > contentX + viewportW) {
            newX = r.right() - viewportW;
        }
        newX = qBound(qreal(0), newX, contentW - viewportW);
        if (qAbs(newX - contentX) > 0.5) {
            flickable->setProperty("contentX", newX);
        }
    }

    if (scrollY) {
        qreal contentY = flickable->property("contentY").toReal();
        qreal newY = contentY;
        if (r.top() < contentY) {
            newY = r.top();
        } else if (r.bottom() > contentY + viewportH) {
            newY = r.bottom() - viewportH;
        }
        newY = qBound(qreal(0), newY, contentH - viewportH);
        if (qAbs(newY - contentY) > 0.5) {
            flickable->setProperty("contentY", newY);
        }
    }
}

} // namespace

namespace spix {

QtItem::QtItem(QQuickItem* item)
: m_item(item)
{
}

QtItem::QtItem(QQuickWindow* window)
: m_item(window)
{
}

Size QtItem::size() const
{
    return Size {qquickitem()->width(), qquickitem()->height()};
}

Point QtItem::position() const
{
    // the point (0, 0) in item coordinates...
    QPointF localPoint {0.0, 0.0};
    // ...is mapped to global to get the item position on screen
    auto globalPoint = qquickitem()->mapToGlobal(localPoint);

    return Point {globalPoint.rx(), globalPoint.ry()};
}

Rect QtItem::bounds() const
{
    Rect bounds {0.0, 0.0, 0.0, 0.0};
    bounds.topLeft = position();
    bounds.size = size();
    return bounds;
}

std::string QtItem::stringProperty(const std::string& name) const
{
    auto value = qobject()->property(name.c_str());
    return value.toString().toStdString();
}

void QtItem::setStringProperty(const std::string& name, const std::string& value)
{
    qobject()->setProperty(name.c_str(), value.c_str());
}

bool QtItem::invokeMethod(const std::string& method, const std::vector<Variant>& args, Variant& ret)
{
    if (args.size() > 10)
        return false;

    std::vector<QVariant> qtVars;
    for (auto arg : args)
        qtVars.push_back(qt::VariantToQVariant(arg));

    QMetaMethod match;
    bool matched = spix::qt::GetMethodMetaForArgs(*qobject(), method, qtVars, match);
    if (!matched)
        return false;

    qt::QMLReturnVariant retVar;
    QGenericReturnArgument retArg = qt::GetReturnArgForQMetaType(match.returnType(), retVar);
    std::vector<QGenericArgument> qtArgs = qt::ConvertAndCreateQArgumentsForMethod(match, qtVars);

    bool success = match.invoke(qobject(), Qt::ConnectionType::DirectConnection, retArg, qtArgs[0], qtArgs[1],
        qtArgs[2], qtArgs[3], qtArgs[4], qtArgs[5], qtArgs[6], qtArgs[7], qtArgs[8], qtArgs[9]);
    if (success) {
        ret = qt::QMLReturnVariantToVariant(retVar);
        return true;
    }
    return false;
}

bool QtItem::visible() const
{
    return qquickitem()->isVisible();
}

bool QtItem::visibleOnScreen() const
{
    const QQuickItem* item = qquickitem();
    if (!item || !item->isVisible()) {
        return false;
    }
    if (item->width() <= 0 || item->height() <= 0) {
        return false;
    }

    const QQuickWindow* window = item->window();
    if (!window) {
        return false;
    }

    // The item's rectangle in scene coordinates...
    QRectF visibleRect = item->mapRectToScene(QRectF(0, 0, item->width(), item->height()));

    // ...clipped to the window bounds...
    visibleRect = visibleRect.intersected(QRectF(0, 0, window->width(), window->height()));

    // ...and clipped to every clipping ancestor (a scrolled Flickable/ListView
    // with clip:true, or any other clipping container). Non-clipping ancestors
    // genuinely let content overflow and be seen, so they are not intersected.
    for (const QQuickItem* ancestor = item->parentItem(); ancestor; ancestor = ancestor->parentItem()) {
        if (ancestor->clip()) {
            const QRectF ancestorRect
                = ancestor->mapRectToScene(QRectF(0, 0, ancestor->width(), ancestor->height()));
            visibleRect = visibleRect.intersected(ancestorRect);
        }
    }

    return visibleRect.width() > 0 && visibleRect.height() > 0;
}

bool QtItem::ensureVisibleInViewport()
{
    QQuickItem* item = qquickitem();
    if (!item) {
        return false;
    }

    // Nudge every scrollable ancestor (innermost first) so the item's rect ends
    // up inside its viewport. Handles nested scrollers.
    for (QQuickItem* ancestor = item->parentItem(); ancestor; ancestor = ancestor->parentItem()) {
        if (ancestor->inherits("QQuickFlickable")) {
            scrollFlickableToShow(ancestor, item);
        }
    }

    return visibleOnScreen();
}

QQuickItem* QtItem::qquickitem()
{
    if (std::holds_alternative<QQuickWindow*>(m_item))
        return std::get<QQuickWindow*>(m_item)->contentItem();
    else
        return std::get<QQuickItem*>(m_item);
}

const QQuickItem* QtItem::qquickitem() const
{
    if (std::holds_alternative<QQuickWindow*>(m_item))
        return std::get<QQuickWindow*>(m_item)->contentItem();
    else
        return std::get<QQuickItem*>(m_item);
}

QObject* QtItem::qobject()
{
    return std::visit([](auto i) { return static_cast<QObject*>(i); }, m_item);
}

const QObject* QtItem::qobject() const
{
    return std::visit([](auto i) { return static_cast<const QObject*>(i); }, m_item);
}

} // namespace spix
