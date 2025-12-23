add_executable(match
    main.cpp
    string_finder.h
    string_finder.cpp
    task1.h
    task1.cpp
    task2.h
    task2.cpp
)

target_link_libraries(match
    PkgConfig::CLI11
    PkgConfig::TBB
)