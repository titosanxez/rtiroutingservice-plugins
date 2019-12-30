#ifndef RTI_ROUTING_PY_DICTPRINTFORMAT_HPP_
#define RTI_ROUTING_PY_DICTPRINTFORMAT_HPP_

#include "Python.h"

#include <stack>

#include "dds/core/xtypes/DynamicData.hpp"

namespace rti { namespace routing { namespace py {


class DynamicDataConverter {

public:
    static PyObject * to_dictionary(
            const dds::core::xtypes::DynamicData& data);

    static void to_dynamic_data(
            dds::core::xtypes::DynamicData& data,
            PyObject *py_dict);

private:
    DynamicDataConverter(const dds::core::xtypes::DynamicData& data);
    DynamicDataConverter(PyObject *py_dict, dds::core::xtypes::DynamicData& data);

    class Context {
    public:

        Context(PyObject *object)
        : current(object)
        {
        }

        operator PyObject*() const
        {
            return current;
        }

    public:
        PyObject *current;
    };


private:

    template <typename T, typename U>
    void from_native_primitive(
            const dds::core::xtypes::DynamicData &data,
            const rti::core::xtypes::DynamicDataMemberInfo& member_info,
            std::function<PyObject*(U)> to_python_object)
    {
        T value = data.value<T>(member_info.member_index());
        if (PyDict_Check(context_stack_.top())) {
            if (PyDict_SetItemString(
                    context_stack_.top(),
                    member_info.member_name().c_str(),
                    to_python_object(static_cast<U> (value))) != 0) {
                PyErr_Print();
                throw dds::core::Error(
                        "DynamicDataConverter:"
                        + std::string(RTI_FUNCTION_NAME)
                        + "error member=" + member_info.member_name().to_std_string());
            }
        } else {
            assert(PyList_Check(context_stack_.top()));
            if (PyList_SetItem(
                    context_stack_.top(),
                    member_info.member_index() - 1,
                    to_python_object(static_cast<U> (value))) != 0) {
                PyErr_Print();
                throw dds::core::Error(
                        "DynamicDataConverter::build_dictionary: error element="
                        + std::to_string(member_info.member_index()-1));
            }
        }
    }

    void build_dictionary(
            dds::core::xtypes::DynamicData& data,
            const rti::core::xtypes::DynamicDataMemberInfo& member_info);
    void build_dynamic_data(
            dds::core::xtypes::DynamicData& data);

private:
    std::stack<Context> context_stack_;

};


} } }

#endif
