set(TEST_SOURCES
    TestDisplayNames.cpp
    TestIDNA.cpp
    TestLocale.cpp
    TestSegmenter.cpp
    TestTimeZone.cpp
    TestUnicodeCharacterTypes.cpp
    TestUnicodeNormalization.cpp
)

foreach(source IN LISTS TEST_SOURCES)
    serenity_test("${source}" LibUnicode LIBS LibUnicode)
endforeach()

find_package(ICU 76 REQUIRED COMPONENTS data i18n uc)
target_link_libraries(TestTimeZone PUBLIC ICU::data ICU::i18n ICU::uc)
