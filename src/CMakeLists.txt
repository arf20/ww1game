set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file (GLOB SRC
    "*.cpp"
)

add_executable(ww1game ${SRC})

target_link_libraries(ww1game PRIVATE Threads::Threads SDL2main SDL2 SDL2_ttf SDL2_image GL)
