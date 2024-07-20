#ifndef ENGINE_EXPORTS_H
#define ENGINE_EXPORTS_H

#ifdef _WIN32
  #define ENGINE_API __declspec(dllexport)  // For Windows, export symbols
#else
  #define ENGINE_API __attribute__((visibility("default")))  // For Unix-like systems, make symbols visible
#endif

#endif