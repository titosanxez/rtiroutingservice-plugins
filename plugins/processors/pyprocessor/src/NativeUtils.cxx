#include "Python.h"

#include "NativeUtils.hpp"

namespace rti { namespace routing { namespace py {


PyObject* from_native_long_array(
        void* array,
        int32_t size)
{
    int32_t *typed_array = (int32_t *) array;
    PyObjectGuard py_list = PyList_New(size);
    if (py_list.get() == NULL) {
        PyErr_Print();
        throw dds::core::Error("from_native: error creating Python list");
    }
    for (int i = 0; i < size; i++) {
        if (PyList_SetItem(
                py_list.get(),
                i,
                PyLong_FromLong(typed_array[i])) != 0) {
            PyErr_Print();
            throw dds::core::Error("from_native: error inserting element["
                    + std::to_string(i)
                    + "]");
        }
    }

    return py_list.release();
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
            PyBool_FromLong(handle.isValid)) == -1) {
        PyErr_Print();
        throw dds::core::Error("from_native: error setting valid element");
    }

    PyObjectGuard py_list = from_native_array<RTICdrOctet, long>(
            handle.keyHash.value,
            handle.keyHash.length,
            PyLong_FromLong);
    if (py_list.get() == NULL) {
        PyErr_Print();
        throw dds::core::Error("from_native: error creating Python list");
    }
    if (PyDict_SetItemString(
            py_dict.get(),
            "key_hash",
            py_list.get()) == -1) {
        PyErr_Print();
        throw dds::core::Error("from_native: error setting key_hash element");
    }
    py_list.release();


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
    return from_native_array<DDS_Octet, long>(
            guid.value,
            DDS_GUID_LENGTH,
            PyLong_FromLong);
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

