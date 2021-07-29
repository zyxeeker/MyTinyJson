cmake_minimum_required(VERSION 3.8)
project(MyTinyJson)
#set(${PROJECT_NAME} MyTinyJson)

set(CMAKE_CXX_STANDARD 11)

include(FetchContent)
FetchContent_Declare(
        googletest
        # Specify the commit you depend on and update it regularly.
        URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Now simply link against gtest or gtest_main as needed. Eg
add_executable(${PROJECT_NAME} test.cpp parse.hpp define.h json.hpp)
target_link_libraries(${PROJECT_NAME} gtest_main)
#add_test(NAME ${PROJECT_NAME}_test COMMAND ${PROJECT_NAME})