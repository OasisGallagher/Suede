#pragma once

#ifdef __EXPORT_ENGINE__
#define ENGINE_EXPORT __declspec(dllexport)
#else
#define ENGINE_EXPORT __declspec(dllimport)
#endif
