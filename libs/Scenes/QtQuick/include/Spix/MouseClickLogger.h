#pragma once

#include <QObject>
#include <QString>

#include <Spix/spix_qtquick_export.h>

namespace spix {

/**
 * @brief Logs the Spix path of every clicked item
 *
 * While enabled, filters application events and logs a path usable in tests to
 * the "mouseclicklogger" category. Create one in `main()` and enable it.
 */
class SPIXQTQUICK_EXPORT MouseClickLogger : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged FINAL)

public:
    /// The Spix path of the given object.
    static QString pathForObject(QObject* object);

    /// Logs the Spix path of the given object.
    static void logPath(QObject* object);

    explicit MouseClickLogger(QObject* parent = nullptr);
    ~MouseClickLogger() override;

    bool enabled() const;
    void setEnabled(bool newEnabled);

signals:
    void enabledChanged();

    void itemClicked(const QString& path);

protected:
    void attach();
    void detach();

    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    bool m_enabled = false;
};

} // namespace spix
