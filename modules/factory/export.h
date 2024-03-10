#pragma once

#ifdef FACTORY_EXPORTS
#define FACTORY_API __declspec(dllexport)
#else
#define FACTORY_API __declspec(dllimport)
#endif