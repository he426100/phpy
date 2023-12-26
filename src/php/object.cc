/*
  +----------------------------------------------------------------------+
  | Phpy                                                                 |
  +----------------------------------------------------------------------+
  | This source file is subject to version 2.0 of the Apache license,    |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.apache.org/licenses/LICENSE-2.0.html                      |
  | If you did not receive a copy of the Apache2.0 license and are unable|
  | to obtain it through the world-wide-web, please send a note to       |
  | license@swoole.com so we can mail you a copy immediately.            |
  +----------------------------------------------------------------------+
  | Author: Tianfeng Han  <rango@swoole.com>                             |
  | Copyright: 上海识沃网络科技有限公司                                       |
  +----------------------------------------------------------------------+
 */

#include "phpy.h"
#include <tuple>

BEGIN_EXTERN_C()
#include "stubs/phpy_object_arginfo.h"
END_EXTERN_C()

using phpy::CallObject;

zend_class_entry *PyObject_ce;
static zend_object_handlers object_handlers;

struct Object {
    PyObject *object;
    PyObject *iterator;
    PyObject *current;
    uint32_t index;
    zend_object std;
};

static zend_always_inline Object *phpy_object_get_object(zend_object *object) {
    return (Object *) ((char *) object - object_handlers.offset);
}

static zend_always_inline Object *phpy_object_get_object(zval *zobject) {
    return (Object *) ((char *) Z_OBJ_P(zobject) - object_handlers.offset);
}

PyObject *phpy_object_get_handle(zend_object *object) {
    return phpy_object_get_object(object)->object;
}

PyObject *phpy_object_get_handle(zval *zobject) {
    return phpy_object_get_object(zobject)->object;
}

PyObject *phpy_object_get_iterator(zend_object *object) {
    return phpy_object_get_object(object)->iterator;
}

PyObject *phpy_object_get_iterator(zval *object) {
    return phpy_object_get_object(object)->iterator;
}

void phpy_object_iterator_reset(zval *object) {
    auto oo = phpy_object_get_object(object);
    oo->iterator = PyObject_GetIter(oo->object);
    oo->current = PyIter_Next(oo->iterator);
    oo->index = 0;
}

PyObject *phpy_object_iterator_next(zval *object) {
    auto oo = phpy_object_get_object(object);
    oo->current = PyIter_Next(oo->iterator);
    oo->index++;
    return oo->current;
}

bool phpy_object_iterator_valid(zval *object) {
    auto oo = phpy_object_get_object(object);
    return oo->current != NULL;
}

PyObject *phpy_object_iterator_current(zval *object) {
    return phpy_object_get_object(object)->current;
}

uint32_t phpy_object_iterator_index(zval *object) {
    return phpy_object_get_object(object)->index;
}

static zend_object *phpy_object_create_object(zend_class_entry *ce) {
    Object *object_object = (Object *) zend_object_alloc(sizeof(*object_object), ce);

    zend_object_std_init(&object_object->std, ce);
    object_properties_init(&object_object->std, ce);
    object_object->std.handlers = &object_handlers;

    return &object_object->std;
}

static void phpy_object_free_object(zend_object *object) {
    Object *object_object = phpy_object_get_object(object);
    if (object_object->object != NULL) {
        Py_DECREF(object_object->object);
    }
    zend_object_std_dtor(&object_object->std);
}

namespace phpy {
namespace php {
void new_object(zval *zv, PyObject *object) {
    new_object(zv, object, PyObject_ce);
}
void new_object(zval *zv, PyObject *object, zend_class_entry *ce) {
    object_init_ex(zv, ce);
    Py_INCREF(object);
    phpy_object_get_object(zv)->object = object;
}
PyObject *arg_1(INTERNAL_FUNCTION_PARAMETERS) {
    zval *zk;

    ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_ZVAL(zk)
    ZEND_PARSE_PARAMETERS_END_EX(return NULL);

    return php2py(zk);
}
PyObject *arg_1(INTERNAL_FUNCTION_PARAMETERS, zend_class_entry *ce) {
    zval *zk;

    ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_OBJECT_OF_CLASS(zk, ce)
    ZEND_PARSE_PARAMETERS_END_EX(return NULL);

    return php2py(zk);
}
static std::tuple<PyObject *, PyObject *> arg_2_empty = {NULL, NULL};
std::tuple<PyObject *, PyObject *> arg_2(INTERNAL_FUNCTION_PARAMETERS) {
    zval *arg0, *arg1;

    ZEND_PARSE_PARAMETERS_START(2, 2)
    Z_PARAM_ZVAL(arg0)
    Z_PARAM_ZVAL(arg1)
    ZEND_PARSE_PARAMETERS_END_EX(return arg_2_empty);

    return std::make_tuple(php2py(arg0), php2py(arg1));
}
std::tuple<PyObject *, PyObject *> arg_2(INTERNAL_FUNCTION_PARAMETERS, zend_class_entry *ce) {
    zval *arg0, *arg1;

    ZEND_PARSE_PARAMETERS_START(2, 2)
    Z_PARAM_OBJECT_OF_CLASS(arg0, ce)
    Z_PARAM_OBJECT_OF_CLASS(arg1, ce)
    ZEND_PARSE_PARAMETERS_END_EX(return arg_2_empty);

    return std::make_tuple(php2py(arg0), php2py(arg1));
}
}  // namespace php
}  // namespace phpy

int php_class_object_init(INIT_FUNC_ARGS) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "PyObject", class_PyObject_methods);
    PyObject_ce = zend_register_internal_class_ex(&ce, NULL);
    PyObject_ce->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES | ZEND_ACC_NOT_SERIALIZABLE;

    PyObject_ce->create_object = phpy_object_create_object;

    memcpy(&object_handlers, &std_object_handlers, sizeof(zend_object_handlers));
    object_handlers.offset = XtOffsetOf(Object, std);
    object_handlers.free_obj = phpy_object_free_object;

    return SUCCESS;
}

zend_class_entry *phpy_object_get_ce() {
    return PyObject_ce;
}

void phpy_object_ctor(zval *zobject, PyObject *object) {
    phpy_object_get_object(zobject)->object = object;
}

ZEND_METHOD(PyObject, __construct) {
    zval *zv = NULL;

    ZEND_PARSE_PARAMETERS_START(0, 1)
    Z_PARAM_OPTIONAL
    Z_PARAM_ZVAL(zv)
    ZEND_PARSE_PARAMETERS_END_EX(return );

    if (zv == NULL) {
        phpy_object_get_object(ZEND_THIS)->object = Py_None;
        Py_INCREF(Py_None);
    } else {
        phpy_object_get_object(ZEND_THIS)->object = php2py_object(zv);
    }
}

ZEND_METHOD(PyObject, __call) {
    char *name;
    size_t l_name;
    zval *arguments;

    ZEND_PARSE_PARAMETERS_START(2, 2)
    Z_PARAM_STRING(name, l_name)
    Z_PARAM_ARRAY(arguments)
    ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

    auto object = phpy_object_get_handle(ZEND_THIS);
    auto fn = PyObject_GetAttrString(object, name);
    if (!fn || !PyCallable_Check(fn)) {
        PyErr_Print();
        zend_throw_error(NULL, "PyObject: has no callable attribute '%s'", name);
        return;
    }
    CallObject caller(fn, return_value, arguments);
    caller.call();
    Py_DECREF(fn);
}

ZEND_METHOD(PyObject, __get) {
    char *name;
    size_t l_name;

    ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_STRING(name, l_name)
    ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

    auto object = phpy_object_get_handle(ZEND_THIS);
    auto value = PyObject_GetAttrString(object, name);
    if (value != NULL) {
        py2php(value, return_value);
        Py_DECREF(value);
    } else {
        PyErr_Print();
        zend_throw_error(NULL, "PyObject<%s> has no attribute '%s'", Py_TypeName(object), name);
    }
}

ZEND_METHOD(PyObject, __set) {
    char *name;
    size_t l_name;
    zval *zvalue;

    ZEND_PARSE_PARAMETERS_START(2, 2)
    Z_PARAM_STRING(name, l_name)
    Z_PARAM_ZVAL(zvalue)
    ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

    auto object = phpy_object_get_handle(ZEND_THIS);
    auto value = PyObject_SetAttrString(object, name, php2py(zvalue));
    if (value < 0) {
        PyErr_Print();
        zend_throw_error(NULL, "PyObject<%s> cannot write attribute '%s'", Py_TypeName(object), name);
    }
}

ZEND_METHOD(PyObject, __toString) {
    auto object = phpy_object_get_handle(ZEND_THIS);
    auto value = PyObject_Str(object);
    if (value != NULL) {
        Py_ssize_t sl;
        const char *sv = PyUnicode_AsUTF8AndSize(value, &sl);
        ZVAL_STRINGL(return_value, sv, sl);
        Py_DECREF(value);
    } else {
        PyErr_Print();
        zend_throw_error(NULL, "PyObject<%s> has no attribute '__str__'", Py_TypeName(object));
        return;
    }
}

ZEND_METHOD(PyObject, __invoke) {
    int argc = 0;
    zval *argv = NULL;
    HashTable *kwargs;

    ZEND_PARSE_PARAMETERS_START(0, -1)
    Z_PARAM_OPTIONAL
    Z_PARAM_VARIADIC_WITH_NAMED(argv, argc, kwargs)
    ZEND_PARSE_PARAMETERS_END();

    auto object = phpy_object_get_handle(ZEND_THIS);
    if (!object || !PyCallable_Check(object)) {
        PyErr_Print();
        zend_throw_error(NULL, "PyObject<%s>: object is not callable", Py_TypeName(object));
        return;
    }

    CallObject caller(object, return_value, argc, argv, kwargs);
    caller.call();
}
