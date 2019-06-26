#include <pybind11/pybind11.h>

#include "misc/random.h"

namespace py = pybind11;

namespace SingularityTrainer
{
int add(int i, int j)
{
    return i + j;
}

PYBIND11_MODULE(example, m)
{
    m.doc() = "pybind11 example plugin"; // optional module docstring

    m.def("add", &add, "A function which adds two numbers");

    py::class_<Random>(m, "Random")
        .def(py::init<int>())
        .def("next_float", py::overload_cast<float, float>(&Random::next_float));
}
}