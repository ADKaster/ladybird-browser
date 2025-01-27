/*
 * Copyright (c) 2025, Andrew Kaster <andrew@ladybird.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <RequestServer/WebSocketImplCurl.h>

namespace RequestServer {

WebSocketImplCurl::WebSocketImplCurl(CURLM* multi)
    : m_multi_handle(multi)
{
}

WebSocketImplCurl::~WebSocketImplCurl()
{
    if (m_easy_handle) {
        curl_multi_remove_handle(m_multi_handle, m_easy_handle);
        curl_easy_cleanup(m_easy_handle);
    }

    for (auto* list : m_curl_string_lists) {
        curl_slist_free_all(list);
    }
}

void WebSocketImplCurl::connect(WebSocket::ConnectionInfo const& info)
{
    VERIFY(!m_easy_handle);

    m_easy_handle = curl_easy_init();
    VERIFY(m_easy_handle); // FIXME: Allow failure, and return ENOMEM

    auto set_option = [this](auto option, auto value) -> bool {
        auto result = curl_easy_setopt(m_easy_handle, option, value);
        if (result == CURLE_OK)
            return true;
        dbgln("WebSocketImplCurlL::connect: Failed to set curl option {}={}: {}", to_underlying(option), value, curl_easy_strerror(result));
        return false;
    };

    set_option(CURLOPT_PRIVATE, reinterpret_cast<uintptr_t>(this) | 1);
    set_option(CURLOPT_WS_OPTIONS, CURLWS_RAW_MODE);
    set_option(CURLOPT_WRITEFUNCTION, &WebSocketImplCurl::curl_write_callback);
    set_option(CURLOPT_WRITEDATA, this);

    // FIXME: Add a header function to validate the Sec-WebSocket headers that curl currently doesn't validate

    auto const& incoming_url = info.url();
    auto url = incoming_url;
    // if (incoming_url.scheme() == "ws"sv)
    //     url.set_scheme("http"_string);
    // else if (incoming_url.scheme() == "wss"sv)
    //     url.set_scheme("https"_string);
    dbgln("WebSocket connect to {}", url.to_string());

    set_option(CURLOPT_URL, url.to_byte_string().characters());
    set_option(CURLOPT_PORT, url.port_or_default());

    auto origin_header = ByteString::formatted("Origin: {}", info.origin());
    curl_slist* curl_headers = curl_slist_append(nullptr, origin_header.characters());

    for (auto const& [name, value] : info.headers().headers()) {
        auto header_string = ByteString::formatted("{}: {}", name, value);
        curl_headers = curl_slist_append(curl_headers, header_string.characters());
    }

    if (auto const& protocols = info.protocols(); !protocols.is_empty()) {
        StringBuilder protocol_builder;
        protocol_builder.append("Sec-WebSocket-Protocol: "sv);
        protocol_builder.append(ByteString::join(","sv, protocols));
        curl_headers = curl_slist_append(curl_headers, protocol_builder.to_byte_string().characters());
    }

    if (auto const& extensions = info.extensions(); !extensions.is_empty()) {
        StringBuilder protocol_builder;
        protocol_builder.append("Sec-WebSocket-Extensions: "sv);
        protocol_builder.append(ByteString::join(","sv, extensions));
        curl_headers = curl_slist_append(curl_headers, protocol_builder.to_byte_string().characters());
    }

    set_option(CURLOPT_HTTPHEADER, curl_headers);
    m_curl_string_lists.append(curl_headers);

    CURLMcode const err = curl_multi_add_handle(m_multi_handle, m_easy_handle);
    VERIFY(err == CURLM_OK);
}

bool WebSocketImplCurl::can_read_line()
{
    VERIFY_NOT_REACHED();
}

ErrorOr<ByteBuffer> WebSocketImplCurl::read(int max_size)
{
    auto buffer = TRY(ByteBuffer::create_uninitialized(max_size));
    auto const read_bytes = TRY(m_read_buffer.read_some(buffer));
    return buffer.slice(0, read_bytes.size());
}

ErrorOr<ByteString> WebSocketImplCurl::read_line(size_t)
{
    VERIFY_NOT_REACHED();
}

bool WebSocketImplCurl::send(ReadonlyBytes bytes)
{
    size_t sent = 0;
    CURLcode const result = curl_ws_send(m_easy_handle, bytes.data(), bytes.size(), &sent, 0, 0);
    (void)sent;

    if (result != CURLE_OK)
        dbgln("WebSocketImplCurl::send: curl_ws_send failed: {}", curl_easy_strerror(result));

    return result == CURLE_OK;
}

bool WebSocketImplCurl::eof()
{
    return m_read_buffer.is_eof();
}

void WebSocketImplCurl::discard_connection()
{
    curl_multi_remove_handle(m_multi_handle, m_easy_handle);
    curl_easy_cleanup(m_easy_handle);
    m_easy_handle = nullptr;
}

void WebSocketImplCurl::did_connect()
{
    m_connected = true;
    on_connected();
    if (m_read_buffer.used_buffer_size() > 0)
        on_ready_to_read();
}

size_t WebSocketImplCurl::curl_write_callback(char* buffer, size_t size, size_t num_items, void* user_data)
{
    auto* impl = static_cast<WebSocketImplCurl*>(user_data);
    auto const total_size = size * num_items;

    auto written_or_error = impl->m_read_buffer.write_some(Bytes { buffer, total_size });
    if (written_or_error.is_error()) {
        dbgln("WebSocketImplCurl::curl_write_callback: Error writing to buffer: {}", written_or_error.error());
        return 0;
    }

    if (!impl->m_connected)
        impl->did_connect();
    else
        impl->on_ready_to_read();

    return written_or_error.value();
}

}
