#ifndef VIRTUALLAB_DATA_VALUE_H_
#define VIRTUALLAB_DATA_VALUE_H_

#include <vector>
#include <string>
#include <map>
#include <typeinfo>
#include <vector>

namespace vl {

class DataValueImpl {
public:
    virtual ~DataValueImpl() {}

    virtual void* createState() const = 0;
    virtual void destroyState(void* state) const = 0;
    virtual void copyState(const void* src, void* dest) const = 0;
    virtual void* get(const std::type_info& type, void* state) const = 0;
    virtual bool set(const std::type_info& type, void* state, const void* val) const = 0;
};

class DataValue {
public:
    DataValue() : impl(NULL), state(NULL) {}

    DataValue(const DataValue& value) : impl(NULL), state(NULL) {
        *this = value;
    }

    virtual ~DataValue() {
        if (impl) {
            impl->destroyState(state);
        }
    }

    void operator=(const DataValue& value) {
        if (impl) {
            impl->destroyState(state);
        }
        impl = value.impl;
        if (impl) {
            state = impl->createState();
            impl->copyState(value.state, state);
        }
        else {
            state = NULL;
        }
    }
    
    template <typename T>
    T get(const T& defaultValue = T()) const {
        static const std::type_info& type = typeid(T);
        if (impl) {
            void* val = impl->get(type, state);
            if (val) {
                return *static_cast<T*>(val);
            }
        }

        return defaultValue;
    }

    template <typename T>
    bool set(T val) {
        static const std::type_info& type = typeid(T);
        if (impl) {
            return impl->set(type, state, &val);
        }

        return false;
    }

    template <typename T>
    bool isType() const {
        static const std::type_info& type = typeid(T);
        if (impl) {
            void* val = impl->get(type, state);
            if (val) {
                return true;
            }
        }

        return false;
    }

protected:
    DataValueImpl* impl;
    void* state;
};

template <typename T>
class TypedDataValueImpl : public DataValueImpl {
public:
    virtual ~TypedDataValueImpl() {}

    void* createState() const {
        std::cout << "createState" << std::endl;
        return new T();
    }

    virtual void destroyState(void* state) const {
        T* s =static_cast<T*>(state);
        std::cout << "destroyState " << *s << std::endl;
        delete s;
    }

    virtual void copyState(const void* src, void* dest) const {
        const T* s = static_cast<const T*>(src);
        T* d = static_cast<T*>(dest);
        *d = *s;
        std::cout << "copyState " << *s << std::endl;
    }

    virtual void* get(const std::type_info& type, void* state) const {
        static const std::type_info& t = typeid(T);
        if (t == type) {
            return static_cast<T*>(state);
        }

        return NULL;
    }

    virtual bool set(const std::type_info& type, void* state, const void* val) const {
        static const std::type_info& t = typeid(T);
        if (t == type) {
            T* s = static_cast<T*>(state);
            *s = *static_cast<const T*>(val);
            std::cout << "set " << *s << std::endl;
        }

        return false;
    }
};

template <typename T>
class TypedDataValue : public DataValue {
public:
    TypedDataValue(const T& defaultValue = T()) {
        static TypedDataValueImpl<T> dataValueImpl;
        impl = &dataValueImpl;
        state = impl->createState();
        set<T>(defaultValue);
    }
};

typedef TypedDataValue<float> FloatDataValue;
typedef TypedDataValue<double> DoubleDataValue;
typedef TypedDataValue<std::string> StringDataValue;

}

#endif