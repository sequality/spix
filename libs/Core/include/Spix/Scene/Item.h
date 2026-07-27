/***
 * Copyright (C) Falko Axmann. All rights reserved.
 * Licensed under the MIT license.
 * See LICENSE.txt file in the project root for full license information.
 ****/

#pragma once

#include <Spix/Data/Geometry.h>
#include <Spix/Data/Variant.h>

#include <string>

namespace spix {

/**
 * @brief Represents an item in a scene
 *
 * This object can be queried for basic properties of an item in the scene.
 * It will be implemented by the different backends, depending on whether this
 * is a Qml/Qt/Mock or other scene.
 */
class Item {
public:
    virtual ~Item() = default;

    // Item properties
    virtual Size size() const = 0;
    virtual Point position() const = 0;
    virtual Rect bounds() const = 0;
    virtual std::string stringProperty(const std::string& name) const = 0;
    virtual void setStringProperty(const std::string& name, const std::string& value) = 0;
    virtual bool invokeMethod(const std::string& method, const std::vector<Variant>& args, Variant& ret) = 0;
    virtual bool visible() const = 0;

    /**
     * @brief Whether the item is actually visible to the user on screen.
     *
     * Stricter than visible(): in addition to the visibility-property chain it
     * requires a non-zero size and that the item's rectangle actually falls
     * within the window and any clipping ancestors (e.g. a scrolled Flickable).
     * An item that is present and visible() == true but scrolled out of view,
     * clipped away, or zero-sized returns false here.
     *
     * The default implementation falls back to visible() for backends that have
     * no geometric notion of a viewport.
     */
    virtual bool visibleOnScreen() const { return visible(); }

    /**
     * @brief Scroll the item into view within any scrollable ancestors.
     *
     * Walks the ancestor chain and adjusts each scrollable container so that
     * this item's rectangle lies inside the visible viewport. Returns whether
     * the item ended up visible on screen. The default implementation is a
     * no-op returning false for backends without scrollable containers.
     */
    virtual bool ensureVisibleInViewport() { return false; }
};

} // namespace spix
