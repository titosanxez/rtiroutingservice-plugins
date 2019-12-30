#ifndef NATIVEUTILS_H
#define NATIVEUTILS_H

#include "Python.h"

#include <rti/routing/processor/Processor.hpp>

namespace rti { namespace routing { namespace py {

struct PythonInitializer {

    PythonInitializer()
    {
        Py_Initialize();
    }

    ~PythonInitializer()
    {
        Py_Finalize();
    }
};

struct PyObjectGuard {
public:
    PyObjectGuard(PyObject *object) : object_(object)
    {
    }

    PyObjectGuard() : object_(NULL)
    {
    }

    ~PyObjectGuard()
    {
        decref();
    }

    PyObject* get()
    {
        return object_;
    }

    void decref()
    {
        if (object_ != NULL) {
            Py_DECREF(object_);
            object_ = NULL;
        }
    }

    PyObject* release()
    {
        PyObject* object = object_;
        object_ = NULL;

        return object;
    }

private:
    PyObject *object_;
};

template <typename T, typename PYOBJECT>
struct PyAllocatorGeneric : public PYOBJECT
{
    void* operator new(size_t size)
    {
        return T::type()->tp_alloc(T::type(), size);
    }

    void operator delete(void* object)
    {
        Py_TYPE(object)->tp_free((PyObject *) object);
    }

    static void delete_object(PyObject *object)
    {
        Py_TYPE(object)->tp_free((PyObject *) object);
    }

};

template <typename T>
struct PyAllocator : public PyAllocatorGeneric<T, PyObject>
{
};

template <typename T>
struct  PyNativeWrapper : public PyAllocator<T>
{
public:

    PyNativeWrapper(typename T::native_type* native)
        : native_(native)
    {
    }

    ~PyNativeWrapper()
    {
    }

    typename T::native_type* get()
    {
        return native_;
    }

protected:

    typename T::native_type *native_;
};


/*
 * --- conversion utilities ---------------------------------------------------
 */
#define RTI_PY_ADD_DICT_ITEM_MEMBER(DICT, OBJECT, MEMBER, CONV) \
    if (PyDict_SetItemString(\
            (DICT), \
            #MEMBER, \
            (CONV)((OBJECT).MEMBER)) == -1) {\
        PyErr_Print();\
        throw dds::core::Error("from_native: error setting member="#MEMBER);\
    }

#define RTI_PY_ADD_DICT_ITEM_VALUE(DICT, MEMBER, CONV) \
    if (PyDict_SetItemString(\
            (DICT), \
            #MEMBER, \
            (CONV)(MEMBER)) == -1) {\
        PyErr_Print();\
        throw dds::core::Error("from_native: error setting member="#MEMBER);\
    }

template <typename T, typename U>
PyObject* from_native_array(
        const T* array,
        int32_t size,
        std::function<PyObject*(U)> to_python_object)
{
    PyObjectGuard py_list = PyList_New(size);
    if (py_list.get() == NULL) {
        PyErr_Print();
        throw dds::core::Error("from_native: error creating Python list");
    }
    for (int i = 0; i < size; i++) {
        if (PyList_SetItem(
                py_list.get(),
                i,
                to_python_object((U) array[i])) != 0) {
            PyErr_Print();
            throw dds::core::Error("from_native: error inserting element["
                    + std::to_string(i)
                    + "]");
        }
    }

    return py_list.release();
}

PyObject* from_native(
        DDS_Boolean boolean_value);

PyObject* from_native(
        DDS_Long long_value);

PyObject* from_native(
        const RTI_RoutingServiceProperties *properties);

PyObject* from_native(
        const DDS_InstanceHandle_t& handle);

PyObject* from_native(
        const DDS_SequenceNumber_t& sn);

PyObject* from_native(
        const DDS_SampleIdentity_t& identity);

PyObject* from_native(
        const RTICdrOctet *byte_array,
        int32_t size);

PyObject* from_native(
        const RTI_RoutingServiceStreamInfo& info);

PyObject * from_native(
        const DDS_SampleInfo& info);

DDS_Boolean to_native(
        DDS_Boolean& dest,
        PyObject* py_value);

DDS_Long to_native(
        DDS_Long& dest,
        PyObject* py_value);

DDS_UnsignedLong to_native(
        DDS_UnsignedLong& dest,
        PyObject* py_value);

DDS_SequenceNumber_t to_native(
        DDS_SequenceNumber_t& dest,
        PyObject* py_dict);

DDS_Time_t to_native(
        DDS_Time_t& dest,
        PyObject* py_dict);

DDS_InstanceHandle_t& to_native(
        DDS_InstanceHandle_t& dest,
        PyObject* py_handle);

DDS_SampleStateKind to_native(
        DDS_SampleStateKind& dest,
        PyObject* py_state);

DDS_ViewStateKind to_native(
        DDS_ViewStateKind& dest,
        PyObject* py_state);

DDS_InstanceStateKind to_native(
        DDS_InstanceStateKind& dest,
        PyObject* py_state);


DDS_GUID_t& to_native(
        DDS_GUID_t& dest,
        PyObject* py_guid);

DDS_SampleInfo& to_native(
        DDS_SampleInfo&,
        PyObject* py_info);


} } }

#endif /* NATIVEUTILS_H */

