cmake_minimum_required(VERSION 3.16)
project(DcCutTest CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

# ============================
# DcCut
# ============================

add_library(dccut_lib
    ../../src/DcCut/dc_cut.cpp
)

target_include_directories(dccut_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/Common
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/DcCut
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

add_executable(test_dccut
    ../common/test_utils.cpp
    test_dc_cut.cpp
)

target_link_libraries(test_dccut
    PRIVATE
        dccut_lib
        GTest::gtest_main
)

# defineの定義/変更
target_compile_definitions(test_dccut
    PRIVATE
      USE_GTEST
      PROJECT_SOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/../../\"
)

# ===========================
# CTest登録 (1ケース＝1プロセス)
# ===========================

add_test(NAME DcCut_Case1 COMMAND test_dccut --gtest_filter=DcCut.Case1)
add_test(NAME DcCut_Case2 COMMAND test_dccut --gtest_filter=DcCut.Case2)
add_test(NAME DcCut_Case3 COMMAND test_dccut --gtest_filter=DcCut.Case3)
add_test(NAME DcCut_Case4 COMMAND test_dccut --gtest_filter=DcCut.Case4)
add_test(NAME DcCut_Case5 COMMAND test_dccut --gtest_filter=DcCut.Case5)
