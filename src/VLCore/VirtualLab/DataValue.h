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
    const T& get(const T& defaultValue = T()) const {
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

    template <typename T>
    T& get() {
        static const std::type_info& type = typeid(T);
        if (impl) {
            void* val = impl->get(type, state);
            if (val) {
                return *static_cast<T*>(val);
            }
        }

        static T defaultValue;
        return defaultValue;
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
        return new T();
    }

    virtual void destroyState(void* state) const {
        T* s =static_cast<T*>(state);
        delete s;
    }

    virtual void copyState(const void* src, void* dest) const {
        const T* s = static_cast<const T*>(src);
        T* d = static_cast<T*>(dest);
        *d = *s;
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

    TypedDataValue(const TypedDataValue<T>& val) : DataValue(val) {}
};

typedef TypedDataValue<int> IntDataValue;
typedef TypedDataValue<float> FloatDataValue;
typedef TypedDataValue<double> DoubleDataValue;
typedef TypedDataValue<std::string> StringDataValue;
typedef std::map<std::string, DataValue> Object;
typedef std::vector<DataValue> Array;

class DataObject : public TypedDataValue<Object> {
public:
    DataObject() : TypedDataValue<Object>() {}
    DataObject(const DataObject& obj) : TypedDataValue<Object>(obj) {}
    DataObject(const Object& obj) : TypedDataValue<Object>(obj) {}

    DataValue& operator[](const std::string& key) {
        return get<Object>()[key];
    }
    const DataValue& operator[](const std::string& key) const {
        static DataValue val;
        Object::const_iterator it = get<Object>().find(key);
        if (it != get<Object>().end()) {
            return it->second;
        }

        return val;
    }

    void operator=(const Object& obj) {
        set<Object>(obj);
    }

    void operator=(const DataValue& val) {
        set<Object>(val.get<Object>());
    }

    typedef vl::Object::iterator iterator;
    typedef vl::Object::const_iterator const_iterator;

    iterator begin() {
        return get<Object>().begin();
    }

    iterator end() {
        return get<Object>().end();
    }

    iterator find(const std::string& key) {
        return get<Object>().find(key);
    }

    const_iterator begin() const {
        return get<Object>().begin();
    }

    const_iterator end() const {
        return get<Object>().end();
    }

    const_iterator find(const std::string& key) const {
        return get<Object>().find(key);
    }

};

class DataArray : public TypedDataValue<Array> {
public:
    DataArray() : TypedDataValue<Array>() {}
    DataArray(const DataArray& arr) : TypedDataValue<Array>(arr) {}
    void push_back(const DataValue& val) {
        get<Array>().push_back(val);
    }
    DataValue& operator[](int index) {
        return get<Array>()[index];
    }
    const DataValue& operator[](int index) const {
        return get<Array>()[index];
    }
    int size() const {
        return get<Array>().size();
    }
};

class DataObjectStack {
public:
    DataObjectStack() {
        DataObject d;
        stack.push_back(d);
    }

    virtual ~DataObjectStack() {}

    DataObject& top() { return stack[stack.size()-1]; }
    void push() { return stack.push_back(stack[stack.size()-1]); }
    void pop() { return stack.pop_back(); }

private:
    std::vector<DataObject> stack;
};


}

#endif