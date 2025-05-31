serenity_test(test-invalid-unicode-js.cpp LibJS LIBS LibJS LibUnicode)
serenity_test(test-value-js.cpp LibJS LIBS LibJS LibUnicode)

find_package(ICU 76 REQUIRED COMPONENTS data i18n uc)
serenity_testjs_test(test-js.cpp test-js LIBS LibGC ICU::i18n ICU::uc ICU::data)
set_tests_properties(test-js PROPERTIES ENVIRONMENT LADYBIRD_SOURCE_DIR=${SERENITY_PROJECT_ROOT})
