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

inline
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

template <typename T>
struct  PyNativeWrapper : public PyObject
{
public:

    PyNativeWrapper(typename T::native_type* native)
        : native_(native)
    {
    }

    ~PyNativeWrapper()
    {
    }

    void* operator new(size_t size)
    {
        return T::type()->tp_alloc(T::type(), size);
    }

    void operator delete(void* object)
    {
        Py_TYPE(object)->tp_free((PyObject *) object);
    }

    typename T::native_type* get()
    {
        return native_;
    }

    static PyObject * new_object(
            PyTypeObject *type,
            PyObject *args,
            PyObject *kwds)
    {
        return type->tp_alloc(type, type->tp_basicsize);
    }

    static void delete_object(PyObject *object)
    {
        Py_TYPE(object)->tp_free((PyObject *) object);
    }

protected:

    typename T::native_type *native_;
};

} } }

#endif /* NATIVEUTILS_H */

