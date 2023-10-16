#pragma once

//Custom DataType Atlas.
typedef signed char        Int8;
typedef short              Int16;
typedef int                Int32;
typedef long long          Int64;

typedef unsigned char      UInt8;
typedef unsigned short     UInt16;
typedef unsigned int       UInt32;
typedef unsigned long long UInt64;

typedef float  Float32;
typedef double Float64;

//Platform detection using predefined macros
#ifdef _WIN32
	#ifdef _WIN64
		#define PLATFORM_WINDOWS
		#define PLATFORM_WIN32 1
	#else
		#error "x86 Builds are not supported!"
#endif
#elif defined(__APPLE__) || defined(__MACH__)
	#include <TargetConditionals.h>

	#if TARGET_IPHONE_SIMULATOR == 1
		#error "IOS simulator is not supported!"
	#elif TARGET_OS_IPHONE == 1
		#define PLATFORM_IOS
		#error "IOS is not supported!"
	#elif TARGET_OS_MAC == 1
		#define PLATFORM_MACOS
		#error "MacOS is not supported!"
	#else
		#error "Unknown Apple platform!"
	#endif
#elif defined(__ANDROID__)
	#define PLATFORM_ANDROID
	#error "Android is not supported!"
#elif defined(__linux__)
	#define PLATFORM_LINUX
	#error "Linux is not supported!"
#else
	#error "Unknown platform!"
#endif