cmake_minimum_required(VERSION 3.16)
project(NoiseGateTest CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

# ============================
# NoiseGate
# ============================

add_library(noisegate_lib
    ../../src/NoiseGate/noise_gate.cpp
)

target_include_directories(noisegate_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/Common
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/NoseGate
        ${CMAKE_CURRENT_SOURCE_DIR}/../common
)

# ============================
# GoogleTest取得
# ============================

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
)
FetchContent_MakeAvailable(googletest)

# ============================
# UnitTest実行ファイル
# ============================

add_executable(test_noisegate
    ../common/test_utils.cpp
    test_noise_gate.cpp
)

target_link_libraries(test_noisegate
    PRIVATE
        noisegate_lib
        GTest::gtest_main
)

# defineの定義/変更
target_compile_definitions(test_noisegate
    PRIVATE
      USE_GTEST
      PROJECT_SOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/../../\"
)

# ===========================
# CTest登録 (1ケース＝1プロセス)
# ===========================

add_test(NAME NoiseGate_Case1 COMMAND test_noisegate --gtest_filter=NoiseGate.Case1)
add_test(NAME NoiseGate_Case2 COMMAND test_noisegate --gtest_filter=NoiseGate.Case2)
add_test(NAME NoiseGate_Case3 COMMAND test_noisegate --gtest_filter=NoiseGate.Case3)
add_test(NAME NoiseGate_Case4 COMMAND test_noisegate --gtest_filter=NoiseGate.Case4)
add_test(NAME NoiseGate_Case5 COMMAND test_noisegate --gtest_filter=NoiseGate.Case5)
