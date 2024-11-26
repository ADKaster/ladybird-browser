set(SOURCES
    Connection.cpp
    Decoder.cpp
    Encoder.cpp
    Message.cpp
)

if (UNIX)
    list(APPEND SOURCES TransportSocket.cpp)
endif()

serenity_lib(LibIPC ipc)
target_link_libraries(LibIPC PRIVATE LibCore LibURL LibThreading)

if (ENABLE_SWIFT AND APPLE)
    include(swift_system)

    generate_clang_module_map(LibIPC)
    target_sources(LibIPC PRIVATE
        MachPort+Bootstrap.swift
        TransportMach.swift
    )
    #target_compile_definitions(LibIPC PUBLIC ENABLE_MACH_IPC=1)
    target_link_libraries(LibIPC PRIVATE AK SystemPackage)
    add_swift_target_properties(LibIPC LAGOM_LIBRARIES AK)
endif()
