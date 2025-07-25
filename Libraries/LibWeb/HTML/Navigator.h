/*
 * Copyright (c) 2022, Andrew Kaster <akaster@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <LibWeb/Bindings/PlatformObject.h>
#include <LibWeb/Gamepad/NavigatorGamepad.h>
#include <LibWeb/HTML/MimeTypeArray.h>
#include <LibWeb/HTML/NavigatorBeacon.h>
#include <LibWeb/HTML/NavigatorConcurrentHardware.h>
#include <LibWeb/HTML/NavigatorDeviceMemory.h>
#include <LibWeb/HTML/NavigatorID.h>
#include <LibWeb/HTML/NavigatorLanguage.h>
#include <LibWeb/HTML/NavigatorOnLine.h>
#include <LibWeb/HTML/PluginArray.h>
#include <LibWeb/HTML/UserActivation.h>
#include <LibWeb/MediaCapabilitiesAPI/MediaCapabilities.h>
#include <LibWeb/StorageAPI/NavigatorStorage.h>

namespace Web::HTML {

class Navigator : public Bindings::PlatformObject
    , public NavigatorBeaconPartial
    , public NavigatorConcurrentHardwareMixin
    , public NavigatorDeviceMemoryMixin
    , public Gamepad::NavigatorGamepadPartial
    , public NavigatorIDMixin
    , public NavigatorLanguageMixin
    , public NavigatorOnLineMixin
    , public StorageAPI::NavigatorStorage {
    WEB_PLATFORM_OBJECT(Navigator, Bindings::PlatformObject);
    GC_DECLARE_ALLOCATOR(Navigator);

public:
    [[nodiscard]] static GC::Ref<Navigator> create(JS::Realm&);

    // FIXME: Implement NavigatorContentUtilsMixin

    // NavigatorCookies
    // FIXME: Hook up to Agent level state
    // https://html.spec.whatwg.org/multipage/system-state.html#dom-navigator-cookieenabled
    bool cookie_enabled() const { return true; }

    // NavigatorPlugins
    // https://html.spec.whatwg.org/multipage/system-state.html#dom-navigator-javaenabled
    bool java_enabled() const { return false; }

    bool pdf_viewer_enabled() const;

    bool webdriver() const;

    [[nodiscard]] GC::Ref<MimeTypeArray> mime_types();
    [[nodiscard]] GC::Ref<PluginArray> plugins();
    [[nodiscard]] GC::Ref<Clipboard::Clipboard> clipboard();
    [[nodiscard]] GC::Ref<Geolocation::Geolocation> geolocation();
    [[nodiscard]] GC::Ref<UserActivation> user_activation();
    [[nodiscard]] GC::Ref<CredentialManagement::CredentialsContainer> credentials();

    Optional<FlyString> do_not_track() const;

    GC::Ref<ServiceWorker::ServiceWorkerContainer> service_worker();

    GC::Ref<MediaCapabilitiesAPI::MediaCapabilities> media_capabilities();

    static WebIDL::Long max_touch_points();

    virtual ~Navigator() override;

protected:
    virtual void visit_edges(Cell::Visitor&) override;

private:
    explicit Navigator(JS::Realm&);

    virtual void initialize(JS::Realm&) override;

    // ^StorageAPI::NavigatorStorage
    virtual Bindings::PlatformObject const& this_navigator_storage_object() const override { return *this; }

    GC::Ptr<PluginArray> m_plugin_array;
    GC::Ptr<MimeTypeArray> m_mime_type_array;

    // https://w3c.github.io/clipboard-apis/#dom-navigator-clipboard
    GC::Ptr<Clipboard::Clipboard> m_clipboard;

    // https://w3c.github.io/geolocation/#navigator_interface
    GC::Ptr<Geolocation::Geolocation> m_geolocation;

    // https://html.spec.whatwg.org/multipage/interaction.html#dom-navigator-useractivation
    GC::Ptr<UserActivation> m_user_activation;

    // https://w3c.github.io/ServiceWorker/#navigator-serviceworker
    GC::Ptr<ServiceWorker::ServiceWorkerContainer> m_service_worker_container;

    // https://w3c.github.io/media-capabilities/#dom-navigator-mediacapabilities
    GC::Ptr<MediaCapabilitiesAPI::MediaCapabilities> m_media_capabilities;

    // https://w3c.github.io/webappsec-credential-management/#framework-credential-management
    GC::Ptr<CredentialManagement::CredentialsContainer> m_credentials;
};

}
