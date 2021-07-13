#include <string>
#include <vector>
#include <optional>
#include <chrono>
#include <iostream>

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/chrono.h>
#include <pybind11/stl.h>

#include "ert/ecl/ecl_sum.hpp"
#include "ert/ecl/ecl_sum_data.hpp"
#include "ert/ecl/smspec_node.hpp"

#include "detail/ecl/ecl_sum.hpp"
#include "detail/ecl/ecl_sum_data.hpp"
#include "detail/ecl/ecl_sum_file_data.hpp"
#include "detail/ecl/ecl_smspec.hpp"

extern "C" double ecl_sum_data_vector_iget(const ecl_sum_data_type *data,
                                           time_t sim_time, int params_index,
                                           bool is_rate, int time_index1,
                                           int time_index2, double weight1,
                                           double weight2);

void ecl_sum_data_init_interp_from_sim_time(const ecl_sum_data_type *data,
                                            time_t sim_time, int *index1,
                                            int *index2, double *weight1,
                                            double *weight2);

namespace py = pybind11;
using std::chrono::system_clock;

using namespace pybind11::literals;

using std::optional;
using std::vector;

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

const ecl::smspec_node &_get_smspec_by_name(const ecl_sum_type *sum,
                                            const std::string &key) {
    const auto &index = sum->smspec->gen_var_index;
    auto it = index.find(key.c_str());
    if (it == index.end())
        throw std::out_of_range("Invalid lookup summary object");
    return *it->second;
}

const ecl::smspec_node &_get_node_by_params_index(const ecl_smspec_type *smspec,
                                                  int params_index) {
    auto node_index = smspec->inv_index_map.at(params_index);
    return *smspec->smspec_nodes[node_index];
}
} // namespace

py::array interpolated(const ecl_sum_data_type *data, const ecl::smspec_node &smspec,
                       const vector<system_clock::time_point> &time_points) {
    time_t start_time = ecl_sum_data_get_data_start(data);
    time_t end_time = ecl_sum_data_get_sim_end(data);
    float start_value = 0;
    float end_value = 0;

    if (!smspec.is_rate()) {
        start_value = ecl_sum_data_iget_first_value(data, smspec.get_params_index());
        end_value = ecl_sum_data_iget_last_value(data, smspec.get_params_index());
    }

    py::array_t<float, py::array::c_style> array(time_points.size());
    auto output_data = array.mutable_data();
    for (size_t i{}; i < time_points.size(); ++i) {
        auto sim_time = system_clock::to_time_t(time_points[i]);

        float value{};
        if (sim_time < start_time) {
            value = start_value;
        } else if (sim_time > end_time) {
            value = end_value;
        } else {
            int time_index1, time_index2;
            double weight1, weight2;
            ecl_sum_data_init_interp_from_sim_time(
                data, sim_time, &time_index1, &time_index2, &weight1, &weight2);
            value = ecl_sum_data_vector_iget(data, sim_time, smspec.get_params_index(),
                                             smspec.is_rate(), time_index1, time_index2,
                                             weight1, weight2);
        }

        output_data[i] = value;
    }
    return std::move(array);
}

py::array
ecl_sum_to_numpy(py::object self, const std::string &key,
                 const optional<vector<system_clock::time_point>> &time_index,
                 bool report_only) {
    auto ecl_sum = _get_object_from_cwrap<ecl_sum_type>(self);

    const auto &smspec = _get_smspec_by_name(ecl_sum, key);

    auto data = ecl_sum->data;

    if (time_index.has_value()) {
        return interpolated(data, smspec, time_index.value());
    }

    // ecl_sum_data_init_double_vector__ from ecl_sum_data.cpp@898
    py::array_t<float, py::array::c_style> array(data->index.length());
    int offset = 0;
    auto output_data = array.mutable_data();
    for (const auto &index_node : data->index) {
        const auto &data_file = data->data_files[index_node.data_index];
        const auto &params_map = index_node.params_map;
        int params_index = params_map[smspec.get_params_index()];

        if (params_index >= 0)
            data_file->get_data(params_index, index_node.length,
                                &output_data[offset]);
        else {
            const ecl::smspec_node &smspec_node =
                _get_node_by_params_index(data->smspec, smspec.get_params_index());
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
