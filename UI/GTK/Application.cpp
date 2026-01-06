/*
 * Copyright (c) 2026, Andrew Kaster <andrew@ladybird.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <UI/GTK/Application.h>
#include <UI/GTK/EventLoopImplementationGLib.h>

#include <gtkmm/application.h>

namespace Ladybird {

Application::Application() = default;
Application::~Application() = default;

void Application::create_platform_options(WebView::BrowserOptions&, WebView::RequestServerOptions&, WebView::WebContentOptions&)
{
}

NonnullOwnPtr<Core::EventLoop> Application::create_platform_event_loop()
{
    if (!browser_options().headless_mode.has_value()) {
        Core::EventLoopManager::install(*new EventLoopManagerGLib);
        m_application = Glib::Application::create("org.ladybird.Ladybird");
    }

    return WebView::Application::create_platform_event_loop();
}

}
