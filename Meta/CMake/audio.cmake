include_guard()

# Audio backend -- how we output audio to the speakers.
if (ANDROID)
    set(LADYBIRD_AUDIO_BACKEND "AAUDIO") 
    return()
elseif (APPLE AND NOT IOS)
    set(LADYBIRD_AUDIO_BACKEND "AUDIO_UNIT")
    return()
elseif (NOT WIN32)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(PULSEAUDIO IMPORTED_TARGET libpulse)

    if (PULSEAUDIO_FOUND)
        set(LADYBIRD_AUDIO_BACKEND "PULSE")
        return()
    endif()
else()
    set(LADYBIRD_AUDIO_BACKEND "WASAPI")
    return()
endif()

message(WARNING "No audio backend available")
