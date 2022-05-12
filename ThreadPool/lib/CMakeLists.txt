file(GLOB mylib_sources threadpool/*.cpp threadpool/*.hpp threadpool/*.h)

add_library(mylib OBJECT ${mylib_sources})

# 这里应该用 PRIVATE 还是 PUBLIC？
target_include_directories(mylib PUBLIC .)
