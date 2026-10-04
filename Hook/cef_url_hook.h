#pragma once
#include "loader.h"

// Spotify endpoint paths and hosts are ASCII. Match without case so a harmless
// URL-casing change cannot bypass a configured block rule.
inline bool contains_ascii_i(const char* text, const char* needle) noexcept
{
    if (!text || !needle || !*needle) return false;
    for (; *text; ++text) {
        const char* hay = text;
        const char* pattern = needle;
        while (*hay && *pattern) {
            auto left = static_cast<unsigned char>(*hay);
            auto right = static_cast<unsigned char>(*pattern);
            if (left >= 'A' && left <= 'Z') left = static_cast<unsigned char>(left + ('a' - 'A'));
            if (right >= 'A' && right <= 'Z') right = static_cast<unsigned char>(right + ('a' - 'A'));
            if (left != right) break;
            ++hay;
            ++pattern;
        }
        if (!*pattern) return true;
    }
    return false;
}

bool hook_cef_url(HMODULE libcef_dll_handle) noexcept;
void* cef_urlrequest_create_stub(void* request, void* client, void* request_context);
