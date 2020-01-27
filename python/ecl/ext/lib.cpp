#include <cstdarg>
#include <cstdio>
#include <dlfcn.h>
#include <sstream>
#include <stdexcept>

#include <pybind11/pybind11.h>

#include <ert/util/ecl_version.hpp>

namespace py = pybind11;
using namespace py::literals;

namespace {
class util_abort_error : public std::logic_error {
public:
    using std::logic_error::logic_error;
};

    std::string get_version() {
        std::ostringstream s;
        s << ecl_version_get_major_version() << '.'
          << ecl_version_get_minor_version() << '.'
          << ecl_version_get_micro_version();
        return s.str();
    }
} // namespace

/**
 * This overloads libecl.so's util_abort__ to throw a Python error instead of
 * std::abort.
 */
extern "C" void util_abort__(const char *file, const char *function, int line,
                             const char *fmt, ...)
{
    char *buf;

    va_list ap;
    va_start(ap, fmt);
    vasprintf(&buf, fmt, ap);
    va_end(ap);

    std::string error{buf};
    free(buf);

    throw util_abort_error(error);
}

PYBIND11_MODULE(_ecl, m)
{
    py::register_exception<util_abort_error>(m, "UtilAbort");

    /* Load libecl.so with RTLD_NOLOAD. */
    auto libecl_handle = dlopen("libecl.so.2", RTLD_NOW | RTLD_GLOBAL | RTLD_NOLOAD);
    if (!libecl_handle)
        throw util_abort_error("Could not dlopen libecl.so");
    m.add_object("libecl_handle", PyLong_FromVoidPtr(libecl_handle));

    /* Set version string */
    m.add_object("__version__", py::cast(get_version()));
}
