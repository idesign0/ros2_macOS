/* macOS compatibility shim, force-included via the toolchain on Apple only.
 * Provides glibc-isms that clang/libc++/BSD headers lack, so ports don't each
 * need their own #ifdef. Guarded so it never redefines an existing symbol. */
#ifndef ROS2_MACOS_COMPAT_H
#define ROS2_MACOS_COMPAT_H
#if defined(__APPLE__)
  #include <libkern/OSByteOrder.h>
  #ifndef htole16
    #define htole16(x) OSSwapHostToLittleInt16(x)
    #define htole32(x) OSSwapHostToLittleInt32(x)
    #define htole64(x) OSSwapHostToLittleInt64(x)
    #define le16toh(x) OSSwapLittleToHostInt16(x)
    #define le32toh(x) OSSwapLittleToHostInt32(x)
    #define le64toh(x) OSSwapLittleToHostInt64(x)
    #define htobe16(x) OSSwapHostToBigInt16(x)
    #define htobe32(x) OSSwapHostToBigInt32(x)
    #define htobe64(x) OSSwapHostToBigInt64(x)
    #define be16toh(x) OSSwapBigToHostInt16(x)
    #define be32toh(x) OSSwapBigToHostInt32(x)
    #define be64toh(x) OSSwapBigToHostInt64(x)
  #endif
  #ifndef SOCK_CLOEXEC
    #define SOCK_CLOEXEC 0
  #endif
#endif
#endif /* ROS2_MACOS_COMPAT_H */
