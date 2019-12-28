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
        build_dictionary(casted_data, data.member_info(i + 1));
    }
}

PyObject* DynamicDataConverter::to_dictionary(
        const dds::core::xtypes::DynamicData& data)
{

    DynamicDataConverter converter(data);
    return converter.context_stack_.top();
}


void DynamicDataConverter::build_dictionary(
        dds::core::xtypes::DynamicData& data,
        const rti::core::xtypes::DynamicDataMemberInfo& member_info)
{
    using rti::core::xtypes::LoanedDynamicData;

    switch (member_info.member_kind().underlying()) {
    case TypeKind::STRUCTURE_TYPE:
    {
        PyObject *py_dict = PyDict_New();
        if (py_dict == NULL) {
            PyErr_Print();
            throw dds::core::Error(
                    "DynamicDataConverter::build_dictionary: error creating dictionary");
        }

        if (PyDict_SetItemString(
                context_stack_.top(),
                member_info.member_name().c_str(),
                py_dict) != 0) {
            PyErr_Print();
            throw dds::core::Error(
                    "DynamicDataConverter::build_dictionary: error member="
                    + member_info.member_name().to_std_string()
                    + "of type=struct");
        }

        context_stack_.push(py_dict);
        LoanedDynamicData loaned_member =
                data.loan_value(member_info.member_name());
        for (int i = 0; i < loaned_member.get().member_count(); i++) {
            build_dictionary(loaned_member.get(), loaned_member.get().member_info(i));
        }
        context_stack_.pop();
    }
        break;

    case TypeKind::INT_32_TYPE:
    {
        int32_t int_value = data.value<int32_t>(member_info.member_index());
        if (PyDict_SetItemString(
                context_stack_.top(),
                member_info.member_name().c_str(),
                PyLong_FromLong(int_value)) != 0) {
            PyErr_Print();
            throw dds::core::Error(
                    "DynamicDataConverter::build_dictionary: error member="
                    + member_info.member_name().to_std_string()
                    + "of type=int32");
        }
    }

    case TypeKind::INT_64_TYPE:
    case TypeKind::ENUMERATION_TYPE:
    {
        int64_t int_value = data.value<int64_t>(member_info.member_index());
        if (PyDict_SetItemString(
                context_stack_.top(),
                member_info.member_name().c_str(),
                PyLong_FromLongLong(int_value)) != 0) {
            PyErr_Print();
            throw dds::core::Error(
                    "DynamicDataConverter::build_dictionary: error member="
                    + member_info.member_name().to_std_string()
                    + "of type=int64");
        }
    }
        break;

    case TypeKind::STRING_TYPE:
    {
        std::string string_value = data.value<std::string>(member_info.member_index());
        if (PyDict_SetItemString(
                context_stack_.top(),
                member_info.member_name().c_str(),
                PyUnicode_FromString(string_value.c_str())) != 0) {
            PyErr_Print();
            throw dds::core::Error(
                    "DynamicDataConverter::build_dictionary: error member="
                    + member_info.member_name().to_std_string()
                    + "of type=string");
        }
    }

        break;

    case TypeKind::FLOAT_32_TYPE:
    {
        float_t float_value = data.value<float_t>(member_info.member_index());
        if (PyDict_SetItemString(
                context_stack_.top(),
                member_info.member_name().c_str(),
                PyFloat_FromDouble(float_value)) != 0) {
            PyErr_Print();
            throw dds::core::Error(
                    "DynamicDataConverter::build_dictionary: error member="
                    + member_info.member_name().to_std_string()
                    + "of type=float32");
        }
    }

        break;

    case TypeKind::FLOAT_64_TYPE:
    {
        double double_value = data.value<double>(member_info.member_index());
        if (PyDict_SetItemString(
                context_stack_.top(),
                member_info.member_name().c_str(),
                PyFloat_FromDouble(double_value)) != 0) {
            PyErr_Print();
            throw dds::core::Error(
                    "DynamicDataConverter::build_dictionary: error member="
                    + member_info.member_name().to_std_string()
                    + "of type=float32");
        }
    }

        break;

    default:
        throw dds::core::Error(
                "DynamicDataConverter::build unsupported type for member="
                + member_info.member_name().to_std_string());

    }

//        switch (current_info.type_kind().underlying()) {
//
//    case TypeKind::UNION_TYPE:
//    {
//        const UnionType& union_type =
//                static_cast<const UnionType&> (member_type);
//        // Add discriminator column
//        current_info.add_child(ColumnInfo(
//               union_type.name() + ".disc",
//               union_type.discriminator()));
//
//        // Recurse members
//        build_complex_member_column_info(current_info, union_type);
//        }
//        break;
//
//    case TypeKind::STRUCTURE_TYPE:
//    {
//        const StructType& struct_type =
//                static_cast<const StructType&> (member_type);
//
//        // Type can be extended, so a parent will exist
//        if (struct_type.has_parent()) {
//            build_column_info(
//                    current_info,
//                    struct_type.parent());
//        }
//
//        // Recurse members
//        build_complex_member_column_info(current_info, struct_type);
//        }
//        break;
//
//    case TypeKind::ARRAY_TYPE:
//    {
//        const ArrayType& array_type =
//                static_cast<const ArrayType &>(member_type);
//        std::vector<uint32_t> dimension_indexes;
//        dimension_indexes.resize(array_type.dimension_count());
//        uint32_t element_count = 0;
//        while (element_count < array_type.total_element_count()) {
//            std::ostringstream element_item;
//            element_item << current_info.name();
//            for (uint32_t j = 0; j < array_type.dimension_count(); j++) {
//                element_item << "[" << dimension_indexes[j] << "]";
//            }
//            // add array item branch
//            ColumnInfo& child = current_info.add_child(ColumnInfo(
//                    element_item.str(),
//                    array_type.content_type()));
//            build_column_info(
//                    child,
//                    array_type.content_type());
//
//            ++dimension_indexes[array_type.dimension_count() - 1];
//            for (uint32_t j = array_type.dimension_count() - 1; j > 0; j--) {
//                if (dimension_indexes[j] ==  array_type.dimension(j)) {
//                    ++dimension_indexes[j - 1];
//                    dimension_indexes[j] = 0;
//                }
//            }
//
//            ++element_count;
//        }
//    }
//        break;
//
//    case TypeKind::SEQUENCE_TYPE:
//    {
//        const SequenceType& sequence_type =
//                static_cast<const SequenceType &> (member_type);
//
//        /* length column*/
//        std::ostringstream length_item;
//        length_item << current_info.name() << ".length";
//        current_info.add_child(ColumnInfo(
//                length_item.str(),
//                rti::core::xtypes::PrimitiveType<int32_t>()));
//
//        /* item columns */
//        for (uint32_t i = 0; i < sequence_type.bounds(); i++) {
//            std::ostringstream element_item;
//            element_item << current_info.name() << "[" << i << "]";
//
//            // add array item branch
//            ColumnInfo& child = current_info.add_child(ColumnInfo(
//                    element_item.str(),
//                    sequence_type.content_type()));
//            build_column_info(
//                    child,
//                    sequence_type.content_type());
//        }
//    }
//        break;
//
//    case TypeKind::ALIAS_TYPE:
//    {
//        const AliasType& alias_type =
//                static_cast<const AliasType &>(member_type);
//        current_info.type_kind(alias_type.related_type().kind());
//        build_column_info(
//                current_info,
//                alias_type.related_type());
//    }
//
//        break;
//
//    default:
//        // leaf reached
//        break;
//    }

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
                member_info.member_name().to_std_string(),
                (int32_t) long_value);
        break;

    case TypeKind::INT_64_TYPE:
        data.value<int64_t>(
                member_info.member_name().to_std_string(),
                (int64_t) long_value);
        break;
    case TypeKind::UINT_16_TYPE:
        data.value<uint16_t>(
                member_info.member_name().to_std_string(),
                (uint16_t) long_value);
        break;
    case TypeKind::UINT_32_TYPE:
        data.value<uint32_t>(
                member_info.member_name().to_std_string(),
                (uint32_t) long_value);
        break;

    case TypeKind::UINT_64_TYPE:
        data.value<uint64_t>(
                member_info.member_name().to_std_string(),
                long_value);

        break;
    default:
        throw dds::core::InvalidArgumentError(
                "inconsistent input value for member="
                + member_info.member_name().to_std_string());
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
                member_info.member_name().to_std_string(),
                (float_t) double_value);
        break;

    case TypeKind::FLOAT_64_TYPE:
        data.value<double>(
                member_info.member_name().to_std_string(),
                (double) double_value);
        break;
    default:
        throw dds::core::InvalidArgumentError(
                "inconsistent input value for member="
                + member_info.member_name().to_std_string());
    }
}

void DynamicDataConverter::build_dynamic_data(
        dds::core::xtypes::DynamicData& data)
{
    using rti::core::xtypes::LoanedDynamicData;

    assert(PyDict_Check(context_stack_.top()));

    PyObject *key = NULL;
    PyObject *value = NULL;
    Py_ssize_t pos = 0;
    while (PyDict_Next(context_stack_.top(), &pos, &key, &value)) {
        char *member_name = PyUnicode_AsUTF8(key);
        if (member_name == NULL) {
            PyErr_Print();
            throw dds::core::Error(
                    "DynamicDataConverter::build_dynamic_data: key is not a member name");
        }

        if (PyDict_Check(value)) {
            LoanedDynamicData loaned_member = data.loan_value(member_name);
            context_stack_.push(value);
            build_dynamic_data(loaned_member);
            context_stack_.pop();
        } else if (PyLong_Check(value)) {
            set_long(data, data.member_info(member_name), value);
        } else if (PyFloat_Check(value)) {
            set_float(data, data.member_info(member_name), value);
        } else if (PyUnicode_Check(value)) {
            data.value<std::string>(member_name, PyUnicode_AsUTF8(value));
        }
    }
}


} } }
