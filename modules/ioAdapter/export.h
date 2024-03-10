#pragma once

#ifdef IO_ADAPTER_EXPORTS
#define IO_ADAPTER_API __declspec(dllexport)
#else
#define IO_ADAPTER_API __declspec(dllimport)
#endif