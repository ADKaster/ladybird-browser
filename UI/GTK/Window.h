/*
 * Copyright (c) 2026, Andrew Kaster <andrew@ladybird.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <adwaita.h>
#include <glibmm/applicationwindow.h>

namespace Ladybird {

struct Tab;

class Window : Gtk::ApplicationWindow {
public:
    enum class IsPopupWindow {
        No,
        Yes,
    };

    Window(Vector<URL::URL> const& initial_urls, IsPopupWindow is_popup_window = IsPopupWindow::No, Tab* parent_tab = nullptr, Optional<u64> page_index = {});
    virtual ~Window() override;

private:
    AdwTabView* tab_view { nullptr };
    AdwTabOverview* m_tab_overview { nullptr };

    IsPopupWindow m_is_popup_window { IsPopupWindow::No };
};

}
