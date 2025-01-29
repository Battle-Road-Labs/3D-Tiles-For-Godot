#ifndef NETWORK_UTILS_H
#define NETWORK_UTILS_H
#include <string_view>
#include <array>

class NetworkUtils {

public:

#if defined(_WIN32) || defined(_WIN64)
	template <uint32_t N> static constexpr auto SystemOpenURL(const char(&url)[N])
	{
		const char* cmd = CompileTimeConcat("start ", url).data();
		return system(cmd);
	}
#elif defined(__APPLE__) && defined(__MACH__)
	static constexpr auto SystemOpenURL(const char(&url)[N])
	{
		const char* cmd = CompileTimeConcat("open ", url).data();
		return system(cmd);
	}
#elif defined(__linux__) && !defined(__ANDROID__)
	static constexpr auto SystemOpenURL(const char(&url)[N])
	{
		const char* cmd = CompileTimeConcat("xdg-open ", url).data();
		return system(cmd);
	}
#else
#error "Unknown compiler"
#endif
private:
	template <uint32_t N1, uint32_t N2>
	static constexpr auto CompileTimeConcat(const char(&str1)[N1], const char(&str2)[N2])
	{
		std::array<char, N1 + N2 - 1> result{}; // Subtract 1 for the null terminator

		for (uint32_t i = 0; i < N1 - 1; ++i)
		{
			result[i] = str1[i];
		}

		for (uint32_t i = 0; i < N2; ++i)
		{
			result[N1 - 1 + i] = str2[i];
		}

		return result;
	}
};

#endif // !NETWORK_UTILS_H
