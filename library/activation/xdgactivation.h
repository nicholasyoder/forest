// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef XDGACTIVATION_H
#define XDGACTIVATION_H

#include <QPointer>
#include <QWaylandClientExtension>

#include <functional>

#include "qwayland-xdg-activation-v1.h"

class QAction;
class XdgActivationToken;

// Launches programs with an XDG_ACTIVATION_TOKEN so they can raise an
// existing window. QMenu hides before `triggered`, so the token is requested
// from an event filter while the menu surface still has focus.
class XdgActivation : public QWaylandClientExtensionTemplate<XdgActivation>,
                      public QtWayland::xdg_activation_v1 {
    Q_OBJECT

public:
    static XdgActivation *instance();

    // Request a token when `action` is clicked or Entered in a QMenu.
    void watch(QAction *action);
    // Request a token from `source`'s surface; call in the input handler, before hiding a popup.
    void request(QWidget *source);
    // Uses the token requested during the current input event, if any.
    void launch(const QString &program, const QStringList &arguments);
    // For launchers that spawn the process themselves (XdgDesktopFile): runs
    // `start` with the token in this process's environment, which it inherits.
    void launch(const std::function<void()> &start);

    // Raise `window` with `token`. Without one, a seat-less token lets the
    // compositor mark it urgent; Qt's own fallback token is rejected when unfocused.
    void activateWindow(QWidget *window, const QString &token);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    XdgActivation();

    QPointer<XdgActivationToken> pending;
};

#endif // XDGACTIVATION_H
