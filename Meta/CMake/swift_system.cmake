include_guard(GLOBAL)

include(FetchContent)

set(FETCHCONTENT_TRY_FIND_PACKAGE_MODE OPT_IN)
FetchContent_Declare(SwiftSystem
    GIT_REPOSITORY https://github.com/apple/swift-system.git
    GIT_TAG 1.4.0
    PATCH_COMMAND "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_LIST_DIR}/patches/git-patch.cmake"
    "${CMAKE_CURRENT_LIST_DIR}/patches/swift-system/0001-CMake-Remove-top-level-binary-module-locations.patch"
    OVERRIDE_FIND_PACKAGE
)

set(BUILD_TESTING_SAVE ${BUILD_TESTING})
set(BUILD_TESTING OFF)
set(BUILD_EXAMPLES OFF)

FetchContent_MakeAvailable(SwiftSystem)

set(BUILD_TESTING ${BUILD_TESTING_SAVE})
