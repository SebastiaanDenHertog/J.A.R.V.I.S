
#ifndef PROMETHEUS_EXPORT_H
#define PROMETHEUS_EXPORT_H

#ifdef PROMETHEUS_STATIC_DEFINE
#define PROMETHEUS_EXPORT
#define PROMETHEUS_NO_EXPORT
#else
#ifndef PROMETHEUS_EXPORT
#ifdef PROMETHEUS_EXPORTS
/* We are building this library */
#define PROMETHEUS_EXPORT __attribute__((visibility("default")))
#else
/* We are using this library */
#define PROMETHEUS_EXPORT __attribute__((visibility("default")))
#endif
#endif

#ifndef PROMETHEUS_NO_EXPORT
#define PROMETHEUS_NO_EXPORT __attribute__((visibility("hidden")))
#endif
#endif

#ifndef PROMETHEUS_DEPRECATED
#define PROMETHEUS_DEPRECATED __attribute__((__deprecated__))
#endif

#ifndef PROMETHEUS_DEPRECATED_EXPORT
#define PROMETHEUS_DEPRECATED_EXPORT PROMETHEUS_EXPORT PROMETHEUS_DEPRECATED
#endif

#ifndef PROMETHEUS_DEPRECATED_NO_EXPORT
#define PROMETHEUS_DEPRECATED_NO_EXPORT PROMETHEUS_NO_EXPORT PROMETHEUS_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#ifndef PROMETHEUS_NO_DEPRECATED
#define PROMETHEUS_NO_DEPRECATED
#endif
#endif

#endif /* PROMETHEUS_EXPORT_H */
