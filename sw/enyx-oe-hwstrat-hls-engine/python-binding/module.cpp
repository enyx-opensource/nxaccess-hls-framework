
#include <sstream>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include <pybind11/stl_bind.h>

#include <enyx/oe/hwstrat/demo/StandAloneTrigger.hpp>

namespace hw = enyx::hw;
namespace demo = enyx::oe::hwstrat::demo;
namespace py = pybind11;

namespace {

std::string
to_str(const std::error_code& v) {
    std::stringstream ret;
    ret << v << " ( " << v.message() << " )";
    return ret.str();
}

std::error_code
trigger_adapter(demo::StandAloneTrigger& trigger,
                     uint16_t collection_id,
                     const std::vector<uint8_t>& arg0,
                     const std::vector<uint8_t>& arg1,
                     const std::vector<uint8_t>& arg2,
                     const std::vector<uint8_t>& arg3,
                     const std::vector<uint8_t>& arg4) {
    return trigger.trigger(collection_id, arg0, arg1, arg2, arg3, arg4);
}


} // anonymous namespace

void add_std(pybind11::module& module) {
    py::class_<std::error_code>(module, "error_code")
        .def(py::init<>())
        .def("value", &std::error_code::value)
        .def("message", &std::error_code::message)
        .def("__bool__", [](const std::error_code& v)
                { return static_cast<bool>(v); } )
        .def("__repr__", [](const std::error_code& v)
                { return to_str(v); }) ;
}


PYBIND11_MODULE(enyx_oe_hwstrat_hls_demo, module) {
    add_std(module);

    py::class_<demo::StandAloneTrigger>(module, "StandAloneTrigger")
        .def(py::init<uint8_t>())
        .def("trigger",
             &trigger_adapter,
             py::arg("collection_id"),
             py::arg("arg0"),
             py::arg("arg1") = std::vector<uint8_t>(),
             py::arg("arg2") = std::vector<uint8_t>(),
             py::arg("arg3") = std::vector<uint8_t>(),
             py::arg("arg4") = std::vector<uint8_t>()
            );

}