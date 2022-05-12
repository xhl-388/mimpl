# Intro

A thread pool implementation

# Build
```
cmake -B build  
cmake --build build
```

# Usage

copy the lib into yours and add link it in the parent CMakeLists.txt  

The threadpool **staticly** creates some threads after calling `init()`  
When you want to add a **job**, just `submit()` the function in the format of `function + args`, then the job will get into a queue that will be consumed by the `ThreadWorker`s.  

You can use the return value of `submit` which is a `future` and `get()` on it to **synchronize** your program.

When you do not need it anymore, you can call `shutdown()` to **destory** the threadpool, but notice that this call will synchronize the program(or block the current thread).

Tip: use `std::ref` to pass reference, use `&` will not change the value.
# Env

gcc (Ubuntu 10.3.0-1ubuntu1~20.04) 10.3.0  
cmake version 3.23.1

# Ref
https://zhuanlan.zhihu.com/p/367309864
