set(IMAGE_DECODER_SOURCE_DIR ${LADYBIRD_SOURCE_DIR}/Userland/Services/ImageDecoder)

set(CMAKE_AUTOMOC OFF)
set(CMAKE_AUTORCC OFF)
set(CMAKE_AUTOUIC OFF)

set(IMAGE_DECODER_SOURCES
    ${IMAGE_DECODER_SOURCE_DIR}/ConnectionFromClient.cpp
)

if (ANDROID)
    add_library(image_decoder SHARED
            ${IMAGE_DECODER_SOURCES}
            ../Android/src/main/cpp/ImageDecoderService.cpp
            ../Android/src/main/cpp/LadybirdServiceBaseJNI.cpp
            ../Utilities.cpp
            )
else()
    add_library(image_decoder STATIC ${IMAGE_DECODER_SOURCES})
endif()
set_target_properties(image_decoder PROPERTIES OUTPUT_NAME imagedecoder)

add_executable(ImageDecoder main.cpp)
target_link_libraries(ImageDecoder PRIVATE image_decoder LibCore LibMain)

target_include_directories(image_decoder PRIVATE ${LADYBIRD_SOURCE_DIR}/Userland/Services/)
target_include_directories(image_decoder PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/..)
target_link_libraries(image_decoder PRIVATE LibCore LibGfx LibIPC LibImageDecoderClient LibMain LibThreading)
set_helper_process_properties(ImageDecoder)
