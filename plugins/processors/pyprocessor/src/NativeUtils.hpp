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

DDS_InstanceHandle_t& to_native(
        DDS_InstanceHandle_t& dest,
        PyObject* py_handle);

class SampleInfoConverter {

public:
    static PyObject * to_dictionary(
            const dds::sub::SampleInfo& info);

    static void to_sample_info(
            dds::sub::SampleInfo& info,
            PyObject *py_dict);
};

} } }

#endif /* NATIVEUTILS_H */

