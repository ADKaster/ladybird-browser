/*
 * Copyright (c) 2025-2026, Andrew Kaster <andrew@ladybird.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibMain/Main.h>
#include <LibWebView/Application.h>
#include <LibWebView/BrowserProcess.h>
#include <LibWebView/URL.h>
#include <LibWebView/Utilities.h>

#include <adwaita.h>

ErrorOr<int> ladybird_main(Main::Arguments arguments)
{
    AK::set_rich_debug_enabled(true);
    adw_init();

    auto app = TRY(Ladybird::Application::create(arguments));
    WebView::BrowserProcess browser_process;

    return app->execute();
}
