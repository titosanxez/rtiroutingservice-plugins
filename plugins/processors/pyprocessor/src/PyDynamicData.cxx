/*
 * (c) 2019 Copyright, Real-Time Innovations, Inc.  All rights reserved.
 *
 * RTI grants Licensee a license to use, modify, compile, and create derivative
 * works of the Software.  Licensee has the right to distribute object form
 * only for use with RTI products.  The Software is provided "as is", with no
 * warranty of any type, including any warranty for fitness for any purpose.
 * RTI is under no obligation to maintain or support the Software.  RTI shall
 * not be liable for any incidental or consequential damages arising out of the
 * use or inability to use the software.
 */

#include "Python.h"

#include <stack>
#include "dds/core/xtypes/StructType.hpp"
#include "dds/core/xtypes/UnionType.hpp"
#include "dds/core/xtypes/MemberType.hpp"
#include "dds/core/xtypes/AliasType.hpp"

#include "PyDynamicData.hpp"
#include "NativeUtils.hpp"


using namespace dds::core::xtypes;

namespace rti { namespace routing { namespace py {

DynamicDataConverter::DynamicDataConverter(
        PyObject *py_dict,
        dds::core::xtypes::DynamicData& data)
{
    context_stack_.push(py_dict);
    build_dynamic_data(data);
}

DynamicDataConverter::DynamicDataConverter(const dds::core::xtypes::DynamicData& data)
{
    context_stack_.push(PyDict_New());
    dds::core::xtypes::DynamicData& casted_data =
            const_cast<dds::core::xtypes::DynamicData&>(data);
    for (int i = 0; i < data.member_count(); i++) {
        if (data.member_exists(i+1)) {
            build_dictionary(casted_data, data.member_info(i + 1));
        }
    }
}

PyObject* DynamicDataConverter::to_dictionary(
        const dds::core::xtypes::DynamicData& data)
{
    DynamicDataConverter converter(data);
    return converter.context_stack_.top();
}


template<>
void DynamicDataConverter::from_native_primitive<std::string, const char*>(
        const dds::core::xtypes::DynamicData &data,
        const rti::core::xtypes::DynamicDataMemberInfo& member_info,
        std::function<PyObject*(const char*) > to_python_object)
{

    std::string value = data.value<std::string>(member_info.member_index());
    if (PyDict_Check(context_stack_.top())) {
        if (PyDict_SetItemString(
                context_stack_.top(),
                member_info.member_name().c_str(),
                to_python_object(value.c_str())) != 0) {
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
                context_stack_.top().index,
                to_python_object(value.c_str())) != 0) {
            PyErr_Print();
            throw dds::core::Error(
                    "DynamicDataConverter::build_dictionary: error element="
                    + std::to_string(context_stack_.top().index));
        }
    }
}

void DynamicDataConverter::build_dictionary(
        dds::core::xtypes::DynamicData& data,
        const rti::core::xtypes::DynamicDataMemberInfo& member_info)
{
    using rti::core::xtypes::LoanedDynamicData;
    using rti::core::xtypes::DynamicDataMemberInfo;

    switch (member_info.member_kind().underlying()) {
    case TypeKind::STRUCTURE_TYPE:
    {
        PyObject *py_dict = PyDict_New();
        if (py_dict == NULL) {
            PyErr_Print();
            throw dds::core::Error(
                    "DynamicDataConverter::build_dictionary: error creating dictionary");
        }

        if (PyDict_Check(context_stack_.top())) {
            if (PyDict_SetItemString(
                    context_stack_.top(),
                    member_info.member_name().c_str(),
                    py_dict) != 0) {
                PyErr_Print();
                throw dds::core::Error(
                        "DynamicDataConverter::build_dictionary: error member="
                        + member_info.member_name().to_std_string()
                        + " of type=struct");
            }
        } else {
            assert(PyList_Check(context_stack_.top()));
            if (PyList_SetItem(
                    context_stack_.top(),
                    context_stack_.top().index,
                    py_dict) != 0) {
                PyErr_Print();
                throw dds::core::Error(
                        "DynamicDataConverter::build_dictionary: error member="
                        + member_info.member_name().to_std_string()
                        + " at index=" + std::to_string(context_stack_.top().index));
            }
        }


        context_stack_.push(py_dict);

        LoanedDynamicData loaned_member =
                data.loan_value(member_info.member_index());
        for (int i = 0; i < loaned_member.get().member_count(); i++) {
            build_dictionary(
                    loaned_member.get(),
                    loaned_member.get().member_info(i + 1));
        }

        context_stack_.pop();
    }
        break;

    case TypeKind::SEQUENCE_TYPE:
    {
        PyObject *py_list = PyList_New(member_info.element_count());
        if (py_list == NULL) {
            PyErr_Print();
            throw dds::core::Error(
                    "DynamicDataConverter::build_dictionary: error creating list");
        }
        if (PyDict_SetItemString(
                context_stack_.top(),
                member_info.member_name().c_str(),
                py_list) != 0) {
            PyErr_Print();
            throw dds::core::Error(
                    "DynamicDataConverter::build_dictionary: error member="
                    + member_info.member_name().to_std_string()
                    + " of type=sequence");
        }

        context_stack_.push(py_list);

        LoanedDynamicData loaned_array =
                data.loan_value(member_info.member_name());
        for (uint32_t i = 0; i < member_info.element_count(); i++) {
            build_dictionary(
                    loaned_array.get(),
                    loaned_array.get().member_info(i + 1));
            ++(context_stack_.top().index);
        }

        context_stack_.pop();
    }
        break;

    case TypeKind::ARRAY_TYPE:
    {

        const StructType struct_type = static_cast<const StructType &>(data.type());
        std::vector<uint32_t> dimension_indexes;
        std::vector<uint32_t> dimensions;
        uint32_t dimension_count = 0;
        dds::core::xtypes::DynamicType member_type =
                struct_type.member(member_info.member_index() - 1).type();

        if (member_type.kind() == TypeKind::ARRAY_TYPE) {
            const ArrayType& array_type =
                    static_cast<const ArrayType &>(member_type);
            dimension_count = array_type.dimension_count();
            dimension_indexes.resize(dimension_count);
            dimensions.resize(dimension_count);
            for (uint32_t j = 0; j < dimension_count; j++) {
                dimensions[j] = array_type.dimension(j);
            }
        } else {
            // bug in dynamic type
            dimension_count = 1;
            dimension_indexes.resize(1);
            dimensions.resize(1);
            dimensions[0] = member_info.element_count();
        }

        LoanedDynamicData loaned_array =
                data.loan_value(member_info.member_name());
        uint32_t element_count = 0;
        while (element_count < member_info.element_count()) {
            for (uint32_t j = 0; j < dimension_count; j++) {

                if ((j < dimension_count - 1)
                        && dimension_indexes[j+1] != 0) {
                    continue;
                }

                if (dimension_indexes[j] == 0) {
                    PyObject *py_list = PyList_New(dimensions[j]);
                    if (py_list == NULL) {
                        PyErr_Print();
                        throw dds::core::Error(
                                "DynamicDataConverter::build_dictionary: error creating list");
                    }
                    if (PyDict_Check(context_stack_.top())) {
                        if (PyDict_SetItemString(
                                context_stack_.top(),
                                member_info.member_name().c_str(),
                                py_list) != 0) {
                            PyErr_Print();
                            throw dds::core::Error(
                                    "DynamicDataConverter::build_dictionary: error member="
                                    + member_info.member_name().to_std_string()
                                    + " of type=array");
                        }
                    } else {
                        assert(PyList_Check(context_stack_.top()));
                        if (PyList_SetItem(
                                context_stack_.top(),
                                context_stack_.top().index,
                                py_list) != 0) {
                            PyErr_Print();
                            throw dds::core::Error(
                                    "DynamicDataConverter::build_dictionary: error member="
                                    + member_info.member_name().to_std_string()
                                    + " at index=" + std::to_string(context_stack_.top().index));
                        }
                    }
                    context_stack_.push(py_list);
                }
            }


            build_dictionary(
                    loaned_array.get(),
                    loaned_array.get().member_info(element_count + 1));
            ++(context_stack_.top().index);

            ++dimension_indexes[dimension_count - 1];
            for (int64_t j = dimension_count - 1; j >= 0; j--) {
                if (dimension_indexes[j] == dimensions[j]) {
                    if (j > 0) {
                        ++dimension_indexes[j - 1];
                    }
                    dimension_indexes[j] = 0;
                    context_stack_.pop();
                    ++(context_stack_.top().index);
                }
            }

            ++element_count;
        }
    }
        break;


    case TypeKind::BOOLEAN_TYPE:

        from_native_primitive<DDS_Boolean, DDS_Long>(
                data,
                member_info,
                PyLong_FromLong);

        break;

    case TypeKind::CHAR_8_TYPE:

        from_native_primitive<DDS_Char, DDS_Long>(
                data,
                member_info,
                PyLong_FromLong);

        break;

    case TypeKind::UINT_8_TYPE:

        from_native_primitive<uint8_t, DDS_Long>(
                data,
                member_info,
                PyLong_FromLong);

        break;

    case TypeKind::INT_16_TYPE:

        from_native_primitive<int16_t, DDS_Long>(
                data,
                member_info,
                PyLong_FromLong);

        break;

    case TypeKind::UINT_16_TYPE:

        from_native_primitive<uint16_t, DDS_Long>(
                data,
                member_info,
                PyLong_FromLong);

        break;

    case TypeKind::INT_32_TYPE:

        from_native_primitive<int32_t, DDS_Long>(
                data,
                member_info,
                PyLong_FromLong);

        break;

    case TypeKind::UINT_32_TYPE:

        from_native_primitive<uint32_t, DDS_UnsignedLong>(
                data,
                member_info,
                PyLong_FromUnsignedLong);

        break;

    case TypeKind::INT_64_TYPE:
    case TypeKind::ENUMERATION_TYPE:

        from_native_primitive<int64_t, DDS_LongLong>(
                data,
                member_info,
                PyLong_FromLongLong);

        break;

    case TypeKind::UINT_64_TYPE:

        from_native_primitive<uint64_t, DDS_UnsignedLongLong>(
                data,
                member_info,
                PyLong_FromUnsignedLong);

        break;

    case TypeKind::STRING_TYPE:

        from_native_primitive<std::string, const char*>(
                data,
                member_info,
                PyUnicode_FromString);

        break;

    case TypeKind::FLOAT_32_TYPE:
    {
        from_native_primitive<float_t, double>(
                data,
                member_info,
                PyFloat_FromDouble);
    }

        break;

    case TypeKind::FLOAT_64_TYPE:
    {
        from_native_primitive<double, double>(
                data,
                member_info,
                PyFloat_FromDouble);
    }

        break;

    default:
        std::string message =
                "unsupported type for member="
                + member_info.member_name().to_std_string()
                + ". Skipping deserialization.";
        DDSLog_logWithFunctionName(
                RTI_LOG_BIT_WARN,
                "DynamicDataConverter::build",
                &RTI_LOG_ANY_s,
                message.c_str());
    }

}

void DynamicDataConverter::to_dynamic_data(
        dds::core::xtypes::DynamicData& data,
        PyObject* py_dict)
{
    DynamicDataConverter converter(py_dict, data);
}

void set_long(
        dds::core::xtypes::DynamicData& data,
        const rti::core::xtypes::DynamicDataMemberInfo& member_info,
        PyObject *py_value)
{
    uint64_t long_value = PyLong_AsUnsignedLongLong(py_value);

    switch (member_info.member_kind().underlying()) {
    case TypeKind::INT_32_TYPE:
    case TypeKind::ENUMERATION_TYPE:
        data.value<int32_t>(
                member_info.member_index(),
                (int32_t) long_value);
        break;

    case TypeKind::INT_64_TYPE:
        data.value<int64_t>(
                member_info.member_index(),
                (int64_t) long_value);
        break;
    case TypeKind::UINT_16_TYPE:
        data.value<uint16_t>(
                member_info.member_index(),
                (uint16_t) long_value);
        break;
    case TypeKind::UINT_32_TYPE:
        data.value<uint32_t>(
                member_info.member_index(),
                (uint32_t) long_value);
        break;

    case TypeKind::UINT_64_TYPE:
        data.value<uint64_t>(
                member_info.member_index(),
                long_value);

        break;
    default:
        throw dds::core::InvalidArgumentError(
                "inconsistent input value for member id="
                + std::to_string(member_info.member_index()));
    }
}

void set_float(
        dds::core::xtypes::DynamicData& data,
        const rti::core::xtypes::DynamicDataMemberInfo& member_info,
        PyObject *py_value)
{
    double double_value = PyFloat_AsDouble(py_value);

    switch (member_info.member_kind().underlying()) {
    case TypeKind::FLOAT_32_TYPE:
        data.value<float_t>(
                member_info.member_index(),
                (float_t) double_value);
        break;

    case TypeKind::FLOAT_64_TYPE:
        data.value<double>(
                member_info.member_index(),
                (double) double_value);
        break;
    default:
        throw dds::core::InvalidArgumentError(
                "inconsistent input value for member id="
                + std::to_string(member_info.member_index()));
    }
}

void DynamicDataConverter::build_dynamic_data(
        dds::core::xtypes::DynamicData& data)
{
    using rti::core::xtypes::LoanedDynamicData;
    rti::core::xtypes::DynamicDataMemberInfo aux_minfo;

    assert(PyDict_Check(context_stack_.top())
            || PyList_Check(context_stack_.top()));


    /* iterate dict or list */
    PyObjectGuard top = PySequence_Fast(
            (PyObject*) context_stack_.top(),
            "");
    for (uint64_t i = 0;
         i < PySequence_Fast_GET_SIZE(top.get());
         i++) {
        PyObject *entry = PySequence_Fast_GET_ITEM(top.get(), i);
        PyObject *value = NULL;

        // of top is a dict, the entry is the key
        if (PyDict_Check(context_stack_.top())) {
            value = PyDict_GetItem(context_stack_.top(), entry);
            char *member_name = PyUnicode_AsUTF8(entry);
            if (member_name == NULL) {
                PyErr_Print();
                throw dds::core::Error(
                        "DynamicDataConverter::build_dynamic_data: key is not a member name");
            }
            //member_id = data.member_info(member_name).member_index();
            const StructType struct_type =
                    static_cast<const StructType &>(data.type());
            dds::core::xtypes::Member member = struct_type.member(member_name);
            aux_minfo.native().member_id = member.get_id() + 1;
            aux_minfo.native().member_name = member_name;
            aux_minfo.native().member_kind = (DDS_TCKind)
                    member.native()._representation._typeCode->_kind;
        } else {
            // if the top is a list, the entry is the value
            value = entry;
            const CollectionType collection_type =
                    static_cast<const CollectionType &>(data.type());
            aux_minfo.native().member_kind = (DDS_TCKind)
                    collection_type.content_type().native()._data._kind;
            aux_minfo.native().member_id = context_stack_.top().index  + i + 1;
        }

        if (PyDict_Check(value)) {
            LoanedDynamicData loaned_member =
                    data.loan_value(aux_minfo.member_index());
            context_stack_.push(value);
            build_dynamic_data(loaned_member);
            context_stack_.pop();
        } else if (PyList_Check(value)) {
            bool top_is_list = PyList_Check(context_stack_.top());
            uint32_t index_offset = 0;

            /* One of the issues with the DynamicData API is that a
             * multidimensional array is accessed with a single loaned member
             * that represents a sequence of all the elements (e.g., for a
             * matrix, that would be m by n).
             *
             * Nevertheless, the Python dictionary contains the proper
             * representation (list of lists) so we need to:
             * - loan only the first array member (subsequent attempts will fail)
             * - Push down information about the offset within the loaned array
             *   that the element should be set, which is given by the total
             *   number of elements iterated from the current list (top) point
             *   of view (e.g, in a matrix, that would be the number of columns)
             */
            if (top_is_list) {
                index_offset = (PyList_GET_SIZE(value) * i);
            }
            context_stack_.push(Context(value, index_offset));

            build_dynamic_data(
                    top_is_list
                    ? data
                    : data.loan_value(aux_minfo.member_index()));

            context_stack_.pop();
        } else if (PyLong_Check(value)) {
            set_long(data, aux_minfo, value);
        } else if (PyFloat_Check(value)) {
            set_float(data, aux_minfo, value);
        } else if (PyUnicode_Check(value)) {
            data.value<std::string>(aux_minfo.member_index(), PyUnicode_AsUTF8(value));
        }

    }
}


} } }
