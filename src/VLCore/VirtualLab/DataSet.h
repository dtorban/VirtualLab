#ifndef VIRTUALLAB_DATA_SET_H_
#define VIRTUALLAB_DATA_SET_H_

#include <vector>
#include <string>
#include <map>
#include <typeinfo>

namespace vl {

class IData {
public:
    virtual ~IData() {}
    
    template <typename T>
    T get(const T& defaultValue = T()) {
        static const std::type_info& type = typeid(T);
		if (isValidType(type)) {
            return *static_cast<T*>(getValue(type));
        }

        return defaultValue;
    }

    template <typename T>
    bool set(T val) {
        static const std::type_info& type = typeid(T);
        if (isValidType(type)) {
            setValue(type, &val);
            return true;
        }

        return false;
    }
    
    template <typename T>
    bool isType() {
        static const std::type_info& type = typeid(T);
        return isValidType(type);
    }

protected:
    virtual void* getValue(const std::type_info& type) = 0;
    virtual void setValue(const std::type_info& type, void* val) = 0;
    virtual bool isValidType(const std::type_info& type) const = 0;
};


class DataSet {
public:
    virtual ~DataSet() {}

    const std::vector<std::string>& getNames() const { return names; }
    IData& operator[](const std::string& name) { return *data[name]; }

    void setData(const std::string& name, IData* dataValue) {
        names.push_back(name);
        data[name] = dataValue;
    }

protected:
    std::vector<std::string> names;
    std::map<std::string, IData*> data;
};

template <typename T>
class TypedData : public IData {
public:
    TypedData(const T& defaultValue = T()) : value(defaultValue) {}
    virtual ~TypedData() {}

    void* getValue(const std::type_info& type) {
        return &value;
    }
    virtual void setValue(const std::type_info& type, void* val) {
        value = *static_cast<T*>(val);
    }
    virtual bool isValidType(const std::type_info& type) const {
        static const std::type_info& t = typeid(T);
        return t == type;
    }
private:
    T value;
};

}

#endif