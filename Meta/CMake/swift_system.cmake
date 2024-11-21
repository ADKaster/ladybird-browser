include_guard(GLOBAL)

include(FetchContent)
find_package(Git REQUIRED QUIET)

set(FETCHCONTENT_TRY_FIND_PACKAGE_MODE OPT_IN)
FetchContent_Declare(SwiftSystem
    GIT_REPOSITORY https://github.com/apple/swift-system.git
    GIT_TAG 1.4.0
    PATCH_COMMAND ${GIT_EXECUTABLE} apply --ignore-whitespace
    "${CMAKE_CURRENT_LIST_DIR}/patches/swift-system/0001-CMake-Remove-top-level-binary-module-locations.patch"
    "${CMAKE_CURRENT_LIST_DIR}/patches/swift-system/0002-CSystem-Set-build-and-install-interfaces-on-included.patch"
    UPDATE_DISCONNECTED ON
    OVERRIDE_FIND_PACKAGE
)

set(BUILD_TESTING_SAVE ${BUILD_TESTING})
set(BUILD_TESTING OFF)
set(BUILD_EXAMPLES OFF)

FetchContent_MakeAvailable(SwiftSystem)
install(TARGETS SystemPackage CSystem EXPORT LagomTargets LIBRARY COMPONENT Lagom_Runtime)

set(BUILD_TESTING ${BUILD_TESTING_SAVE})
