/*
 * Copyright (c) 2026, Andrew Kaster <andrew@ladybird.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <UI/GTK/Window.h>

namespace Ladybird {

Window::Window(Vector<URL::URL> const& initial_urls, IsPopupWindow is_popup_window = IsPopupWindow::No, Tab* parent_tab = nullptr, Optional<u64> page_index = {})
{
    m_tab_view = adw_tab_view_new();
    m_tab_overview = adw_tab_overview_new();

    m_tab_overview->set_view(m_tab_view);
}

Window::~Window()
{
    g_clear_object(&m_tab_overview);
    g_clear_object(&m_tab_view);
}

}
