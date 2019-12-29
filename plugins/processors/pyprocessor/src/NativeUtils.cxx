#include "Python.h"

#include "NativeUtils.hpp"

namespace rti { namespace routing { namespace py {


PyObject* from_native(
        const RTICdrOctet *byte_array,
        int32_t size)
{
    PyObject* py_bytes = PyBytes_FromStringAndSize(
            (const char *) byte_array,
            size);
    if (py_bytes == NULL) {
        PyErr_Print();
        throw dds::core::Error("from_native: error creating byte array");
    }

    return py_bytes;
}

PyObject* from_native(
        const struct RTI_RoutingServiceProperties *properties)
{
    PyObjectGuard py_dict = PyDict_New();
    if (py_dict.get() == NULL) {
        PyErr_Print();
        throw dds::core::Error("error creating Python dictionary");
    }

    for (int i = 0; i < properties->count; i++) {
        PyObjectGuard value = Py_BuildValue("s", properties->properties[i].value);
        if (value.get() == NULL) {
            PyErr_Print();
            throw dds::core::Error("error creating string value");
        }

        if (PyDict_SetItemString(
                py_dict.get(),
                properties->properties[i].name,
                value.get()) == -1) {
            PyErr_Print();
            throw dds::core::Error("error inserting property element into dictionary");
        }
    }

    return py_dict.release();
}

PyObject* from_native(const DDS_InstanceHandle_t& handle)
{
    PyObjectGuard py_dict = PyDict_New();
    if (py_dict.get() == NULL) {
        PyErr_Print();
        throw dds::core::Error("from_native: error creating Python dictionary");
    }
    if (PyDict_SetItemString(
            py_dict.get(),
            "valid",
            PyLong_FromLong(handle.isValid)) == -1) {
        PyErr_Print();
        throw dds::core::Error("from_native: error setting valid element");
    }

    PyObjectGuard py_bytes = from_native(
            handle.keyHash.value,
            handle.keyHash.length);
    if (py_bytes.get() == NULL) {
        PyErr_Print();
        throw dds::core::Error("from_native: error creating Python list");
    }
    if (PyDict_SetItemString(
            py_dict.get(),
            "key_hash",
            py_bytes.get()) == -1) {
        PyErr_Print();
        throw dds::core::Error("from_native: error setting key_hash element");
    }

    return py_dict.release();
}

PyObject* from_native(const DDS_SequenceNumber_t& sn)
{
    PyObjectGuard py_dict = PyDict_New();
    if (py_dict.get() == NULL) {
        PyErr_Print();
        throw dds::core::Error("from_native: error creating Python dictionary");
    }
    if (PyDict_SetItemString(
            py_dict.get(),
            "low",
            PyLong_FromUnsignedLong(sn.low)) == -1) {
        PyErr_Print();
        throw dds::core::Error("from_native: error setting 'low' element");
    }
    if (PyDict_SetItemString(
            py_dict.get(),
            "high",
            PyLong_FromLong(sn.high)) == -1) {
        PyErr_Print();
        throw dds::core::Error("from_native: error setting 'high' element");
    }

    return py_dict.release();
}


PyObject* from_native(const DDS_GUID_t& guid)
{
    return from_native(guid.value, DDS_GUID_LENGTH);
}

PyObject* from_native(
        const DDS_SampleIdentity_t& identity)
{
    PyObjectGuard py_dict = PyDict_New();
    if (py_dict.get() == NULL) {
        PyErr_Print();
        throw dds::core::Error("from_native: error creating Python dictionary");
    }
    PyObjectGuard py_list = from_native(identity.writer_guid);
    if (py_list.get() == NULL) {
        PyErr_Print();
        throw dds::core::Error("from_native: error creating Python list");
    }
    if (PyDict_SetItemString(
            py_dict.get(),
            "writer_guid",
            py_list.get()) == -1) {
        PyErr_Print();
        throw dds::core::Error("from_native: error setting 'writer_guid' element");
    }
    py_list.release();

    PyObjectGuard py_sn = from_native(identity.sequence_number);
    if (PyDict_SetItemString(
            py_dict.get(),
            "sequence_number",
            py_sn.get()) == -1) {
        PyErr_Print();
        throw dds::core::Error("from_native: error setting 'sequence_number' element");
    }
    py_sn.release();

    return py_dict.release();
}

PyObject* from_native(DDS_SampleStateKind state)
{
    return PyLong_FromLong(state);
}

PyObject* from_native(DDS_ViewStateKind state)
{
    return PyLong_FromLong(state);
}


PyObject* from_native(DDS_InstanceStateMask state)
{
    return PyLong_FromLong(state);
}

PyObject* from_native(const DDS_Time_t& time)
{
    PyObjectGuard py_dict = PyDict_New();
    if (py_dict.get() == NULL) {
        PyErr_Print();
        throw dds::core::Error("from_native: error creating Python dictionary");
    }
    if (PyDict_SetItemString(
            py_dict.get(),
            "low",
            PyLong_FromLong(time.sec)) == -1) {
        PyErr_Print();
        throw dds::core::Error("from_native: error setting 'sec' element");
    }
    if (PyDict_SetItemString(
            py_dict.get(),
            "high",
            PyLong_FromUnsignedLong(time.nanosec)) == -1) {
        PyErr_Print();
        throw dds::core::Error("from_native: error setting 'nanosec' element");
    }

    return py_dict.release();
}

PyObject* from_native(const RTI_RoutingServiceStreamInfo& info)
{
    PyObjectGuard py_dict = PyDict_New();
    if (py_dict.get() == NULL) {
        PyErr_Print();
        throw dds::core::Error("from_native: error creating Python dictionary");
    }
    RTI_PY_ADD_DICT_ITEM_MEMBER(
            py_dict.get(),
            info,
            stream_name,
            PyUnicode_FromString);
    RTI_PY_ADD_DICT_ITEM_MEMBER(
            py_dict.get(),
            info.type_info,
            type_name,
            PyUnicode_FromString);

    return py_dict.release();
}

DDS_InstanceHandle_t& to_native(DDS_InstanceHandle_t& dest, PyObject* py_handle)
{
    if (!PyDict_Check(py_handle)) {
        throw dds::core::Error("to_native: object is not a dictionary");
    }

    PyObject *py_valid = PyDict_GetItemString(py_handle, "valid");
    if (py_valid == NULL) {
        throw dds::core::Error("to_native: member=valid not found");
    } else if (!PyLong_Check(py_valid)) {
        throw dds::core::Error("to_native: member=valid is not an integer");
    }
    dest.isValid = (int32_t) PyLong_AsLong(py_valid);

    PyObject *py_key_hash = PyDict_GetItemString(py_handle, "key_hash");
    if (py_key_hash == NULL) {
        throw dds::core::Error("to_native: member=key_hash not found");
    } else if (!PyBytes_Check(py_key_hash)) {
        throw dds::core::Error("to_native: member=key_hash is not a list");
    } else if (PyBytes_GET_SIZE(py_key_hash) != MIG_RTPS_KEY_HASH_MAX_LENGTH) {
         throw dds::core::Error(
                 "to_native: member=key_hash list must have size="
                 + std::to_string(MIG_RTPS_KEY_HASH_MAX_LENGTH));
    }

    std::memcpy(
            dest.keyHash.value,
            PyBytes_AsString(py_key_hash),
            MIG_RTPS_KEY_HASH_MAX_LENGTH);

    return dest;
}


/*
 * --- PySampleInfoConverter --------------------------------------------------
 */
#define PY_SAMPLE_INFO_CONVERTER_SET_MEMBER_NATIVE(DATA, MEMBER) \
{\
    PyObjectGuard py_guard = from_native((DATA)->native().MEMBER); \
    if (PyDict_SetItemString( \
            py_dict.get(), \
            #MEMBER, \
            py_guard.get()) == -1) {\
        PyErr_Print();\
        throw dds::core::Error("from_native: error setting member=" #MEMBER);\
    }\
    py_guard.release(); \
}


PyObject*
SampleInfoConverter::to_dictionary(const dds::sub::SampleInfo& info)
{
    PyObjectGuard py_dict = PyDict_New();
    if (py_dict.get() == NULL) {
        PyErr_Print();
        throw dds::core::Error("from_native: error creating Python dictionary");
    }

    PY_SAMPLE_INFO_CONVERTER_SET_MEMBER_NATIVE(info, instance_handle);
    PY_SAMPLE_INFO_CONVERTER_SET_MEMBER_NATIVE(info, publication_handle);
    PY_SAMPLE_INFO_CONVERTER_SET_MEMBER_NATIVE(info, sample_state);
    PY_SAMPLE_INFO_CONVERTER_SET_MEMBER_NATIVE(info, view_state);
    PY_SAMPLE_INFO_CONVERTER_SET_MEMBER_NATIVE(info, instance_state);
    PY_SAMPLE_INFO_CONVERTER_SET_MEMBER_NATIVE(info, valid_data);
    PY_SAMPLE_INFO_CONVERTER_SET_MEMBER_NATIVE(info, flag);
    PY_SAMPLE_INFO_CONVERTER_SET_MEMBER_NATIVE(info, original_publication_virtual_sequence_number);
    PY_SAMPLE_INFO_CONVERTER_SET_MEMBER_NATIVE(info, original_publication_virtual_guid);
    PY_SAMPLE_INFO_CONVERTER_SET_MEMBER_NATIVE(info, related_original_publication_virtual_sequence_number);
    PY_SAMPLE_INFO_CONVERTER_SET_MEMBER_NATIVE(info, related_original_publication_virtual_guid);
    PY_SAMPLE_INFO_CONVERTER_SET_MEMBER_NATIVE(info, reception_sequence_number);
    PY_SAMPLE_INFO_CONVERTER_SET_MEMBER_NATIVE(info, publication_sequence_number);
    PY_SAMPLE_INFO_CONVERTER_SET_MEMBER_NATIVE(info, reception_timestamp);
    PY_SAMPLE_INFO_CONVERTER_SET_MEMBER_NATIVE(info, source_timestamp);

    return py_dict.release();
}

} } }

