#include <string>
#include <vector>
#include <optional>
#include <chrono>

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/chrono.h>
#include <pybind11/stl.h>

#include "ert/ecl/ecl_sum.hpp"
#include "ert/ecl/smspec_node.hpp"

#include "detail/ecl/ecl_sum.hpp"
#include "detail/ecl/ecl_sum_data.hpp"
#include "detail/ecl/ecl_sum_file_data.hpp"
#include "detail/ecl/ecl_smspec.hpp"

namespace py = pybind11;

using namespace pybind11::literals;

using std::vector;
using std::optional;
using time_point = std::chrono::system_clock::time_point;

namespace ecl {
namespace data {

void assert_ecl_type(const void *obj, int type_id) {
    if (!(*reinterpret_cast<const int *>(obj) == type_id)) {
        throw py::type_error("Unexpected ecl type");
    }
}

} // namespace data
} // namespace ecl

namespace {
template <typename T> const T *_get_object_from_cwrap(py::object &obj) {
    static auto basecclass = py::module_::import("cwrap").attr("BaseCClass");
    if (!py::isinstance(obj, basecclass)) {
        throw py::type_error("Expected a BaseCClass type");
    }

    auto address = py::cast<uint64_t>(obj.attr("_address")());
    auto object = reinterpret_cast<const T *>(address);
    ecl::data::assert_ecl_type(object, ECL_SUM_ID);
    return object;
}
int _get_params_index(const ecl_sum_type *sum, const std::string &key) {
    const auto &index = sum->smspec->gen_var_index;
    auto it = index.find(key);
    if (it == index.end())
        throw std::out_of_range("Invalid lookup summary object");
    return it->second->get_params_index();
}
const ecl::smspec_node &_get_node_by_params_index(const ecl_smspec_type *smspec,
                                                  int params_index) {
    auto node_index = smspec->inv_index_map.at(params_index);
    return *smspec->smspec_nodes[node_index];
}
} // namespace

py::array ecl_sum_to_numpy(py::object self, const std::string &key,
                           optional<vector<time_point>> time_index, bool report_only) {
    auto ecl_sum = _get_object_from_cwrap<ecl_sum_type>(self);

    auto main_params_index =
        _get_params_index(ecl_sum, static_cast<std::string>(key));

    auto data = ecl_sum->data;

    // ecl_sum_data_init_double_vector__ from ecl_sum_data.cpp@898
    py::array_t<float, py::array::c_style> array(data->index.length());
    int offset = 0;
    auto output_data = array.mutable_data();
    for (const auto &index_node : data->index) {
        const auto &data_file = data->data_files[index_node.data_index];
        const auto &params_map = index_node.params_map;
        int params_index = params_map[main_params_index];

        if (params_index >= 0)
            data_file->get_data(params_index, index_node.length,
                                &output_data[offset]);
        else {
            const ecl::smspec_node &smspec_node =
                _get_node_by_params_index(data->smspec, main_params_index);
            for (int i = 0; i < index_node.length; i++)
                output_data[offset + i] = smspec_node.get_default();
        }
        offset += index_node.length;
    }

    return std::move(array);
}


void _native_summary(py::module_ m) {
    m.def("ecl_sum_to_numpy", &ecl_sum_to_numpy, py::arg("self"),
          py::arg("key"), py::arg("time_index"), py::arg("report_only"));
}
