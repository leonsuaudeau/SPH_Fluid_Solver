include(FetchContent)

FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.5.0
)

FetchContent_MakeAvailable(Catch2)

add_executable(sph_tests
        general_tests.cpp
        general_tests.cpp
)

target_link_libraries(sph_tests PRIVATE
        sph_solver
        Catch2::Catch2WithMain
)

add_test(NAME sph_tests COMMAND sph_tests)