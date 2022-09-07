#pragma once

#include <windows.h>
#include <crtdbg.h>

#if defined( DEBUG ) || defined( _DEBUG )
#define _ASSERT_EXPR_A(expr, msg) \
	(void)((!!(expr)) || \
	(1 != _CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, NULL, "%s", msg)) || \
	(_CrtDbgBreak(), 0))
#else
#define  _ASSERT_EXPR_A(expr, expr_str) ((void)0)
#endif

inline LPWSTR hr_trace(HRESULT hr)
{
	LPWSTR msg{ 0 };
	FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPWSTR>(&msg), 0, NULL);
	return msg;
}

class benchmark
{
	LARGE_INTEGER ticks_per_second;
	LARGE_INTEGER start_ticks;
	LARGE_INTEGER current_ticks;

public:
	benchmark()
	{
		QueryPerformanceFrequency(&ticks_per_second);
		QueryPerformanceCounter(&start_ticks);
		QueryPerformanceCounter(&current_ticks);
	}
	~benchmark() = default;
	benchmark(const benchmark&) = delete;
	benchmark& operator=(const benchmark&) = delete;
	benchmark(benchmark&&) noexcept = delete;
	benchmark& operator=(benchmark&&) noexcept = delete;

	void begin()
	{
		QueryPerformanceCounter(&start_ticks);
	}
	float end()
	{
		QueryPerformanceCounter(&current_ticks);
		return static_cast<float>(current_ticks.QuadPart - start_ticks.QuadPart) / static_cast<float>(ticks_per_second.QuadPart);
	}
};

