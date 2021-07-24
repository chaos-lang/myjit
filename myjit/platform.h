#ifndef MYJIT_PLATFORM_H
#define MYJIT_PLATFORM_H

/**
 * Determination a platform of an operation system
 * Fully supported supported only GNU GCC/G++, partially on Clang/LLVM
 */

#if defined(_WIN32)
#   define MYJIT_PLATFORM_WIN 1 // Windows
#   define MYJIT_KERNEL_WIN 1
#   define MYJIT_ABI_WIN 1
#elif defined(_WIN64)
#   define MYJIT_PLATFORM_WIN 1 // Windows
#   define MYJIT_KERNEL_WIN 1
#   define MYJIT_ABI_WIN 1
#elif defined(__CYGWIN__) && !defined(_WIN32)
#   define MYJIT_PLATFORM_WIN 1 // Windows (Cygwin POSIX under Microsoft Window)
#   define MYJIT_KERNEL_WIN 1
#   define MYJIT_ABI_WIN 1
#elif defined(__ANDROID__)
#   define MYJIT_PLATFORM_ANDROID 1 // Android (implies Linux, so it must come first)
#   define MYJIT_KERNEL_LINUX 1
#   define MYJIT_ABI_SYSTEMV 1
#elif defined(__linux__)
#   define MYJIT_PLATFORM_LINUX 1 // Debian, Ubuntu, Gentoo, Fedora, openSUSE, RedHat, Centos and other
#   define MYJIT_KERNEL_LINUX 1
#   define MYJIT_ABI_SYSTEMV 1
#elif defined(__unix__) || !defined(__APPLE__) && defined(__MACH__)
#   include <sys/param.h>
#   if defined(BSD)
#       define MYJIT_PLATFORM_BSD 1 // FreeBSD, NetBSD, OpenBSD, DragonFly BSD
#       define MYJIT_KERNEL_BSD 1
#       define MYJIT_ABI_BSD 1
#   endif
#elif defined(__hpux)
#   define MYJIT_PLATFORM_HP_UX 1 // HP-UX
#elif defined(_AIX)
#   define MYJIT_PLATFORM_AIX 1 // IBM AIX
#elif defined(__APPLE__) && defined(__MACH__) // Apple OSX and iOS (Darwin)
#   include <TargetConditionals.h>
#   if TARGET_IPHONE_SIMULATOR == 1
#       define MYJIT_PLATFORM_IOS 1 // Apple iOS
#   elif TARGET_OS_IPHONE == 1
#       define MYJIT_PLATFORM_IOS 1 // Apple iOS
#   elif TARGET_OS_MAC == 1
#       define MYJIT_PLATFORM_OSX 1 // Apple OSX
#   endif
#   define MYJIT_KERNEL_DARWIN 1
#   define MYJIT_ABI_SYSTEMV 1
#elif defined(__sun) && defined(__SVR4)
#   define MYJIT_PLATFORM_SOLARIS 1 // Oracle Solaris, Open Indiana
#else
#   define MYJIT_PLATFORM_UNKOWN 1
#endif

#endif