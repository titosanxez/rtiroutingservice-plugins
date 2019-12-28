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
