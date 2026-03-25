\page sanitizersbuilds Sanitizer Builds

## Original source and Copyright

The original work for these instructions is the project https://github.com/StableCoder/cmake-scripts/ licensed under the Apache-2 with the following copyright:

> Copyright (C) 2018-2022 by George Cave - gcave@stablecoder.ca

## Description

Sanitizers are tools that perform checks during a program’s runtime and returns issues, and as such, along with unit testing, code coverage and static analysis, is another tool to add to the programmers toolbox. And of course, like the previous tools, are tragically simple to add into any project using CMake, allowing any project and developer to quickly and easily use.

A quick rundown of the tools available, and what they do:
- [LeakSanitizer](https://clang.llvm.org/docs/LeakSanitizer.html) detects memory leaks, or issues where memory is allocated and never deallocated, causing programs to slowly consume more and more memory, eventually leading to a crash.
- [AddressSanitizer](https://clang.llvm.org/docs/AddressSanitizer.html) is a fast memory error detector. It is useful for detecting most issues dealing with memory, such as:
    - Out of bounds accesses to heap, stack, global
    - Use after free
    - Use after return
    - Use after scope
    - Double-free, invalid free
    - Memory leaks (using LeakSanitizer)
- [ThreadSanitizer](https://clang.llvm.org/docs/ThreadSanitizer.html) detects data races for multi-threaded code.
- [UndefinedBehaviourSanitizer](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html) detects the use of various features of C/C++ that are explicitly listed as resulting in undefined behaviour. Most notably:
    - Using misaligned or null pointer.
    - Signed integer overflow
    - Conversion to, from, or between floating-point types which would overflow the destination
    - Division by zero
    - Unreachable code
- [MemorySanitizer](https://clang.llvm.org/docs/MemorySanitizer.html) detects uninitialized reads.
- [Control Flow Integrity](https://clang.llvm.org/docs/ControlFlowIntegrity.html) is designed to detect certain forms of undefined behaviour that can potentially allow attackers to subvert the program's control flow.

These are used by declaring the `GZ_SANITIZER` CMake variable as string containing any of:
- Address
- Memory
- MemoryWithOrigins
- Undefined
- Thread
- Leak
- CFI

Multiple values are allowed, e.g. `-DGZ_SANITIZER=Address,Leak` but some sanitizers cannot be combined together, e.g.`-DGZ_SANITIZER=Address,Memory` will result in configuration error. The delimeter character is not required and `-DGZ_SANITIZER=AddressLeak` would work as well.
