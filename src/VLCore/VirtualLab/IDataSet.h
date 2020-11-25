#ifndef VIRTUALLAB_IDATA_SET_H_
#define VIRTUALLAB_IDATA_SET_H_

#include <vector>
#include <string>
#include <map>
#include <typeinfo>

namespace vl {

class IDataSet {
public:
    virtual ~IDataSet() {}
    
    template <typename T>
    T get(const T& defaultValue = T()) const;

    template <typename T>
    bool set(T val);

    template <typename T>
    bool isType();

    virtual const std::vector<std::string>& getKeys() const {
        static std::vector<std::string> keys;
        return keys;
    }

    virtual const IDataSet& operator[](std::string key) const { return *this; }
    virtual IDataSet& operator[](std::string key) { return *this; }

protected:
    virtual const void* getValue(const std::type_info& type) const { return NULL; }
    virtual void setValue(const std::type_info& type, void* val) {}
    virtual bool isValidType(const std::type_info& type) const { return false; }
};

template <typename T>
class TypedData : public IDataSet {
public:
    TypedData(const T& defaultValue = T()) {
        value = defaultValue;
    }
    virtual ~TypedData() {}

protected:
    const void* getValue(const std::type_info& type) const { return &value; }
    void setValue(const std::type_info& type, void* val) { value = *(static_cast<T*>(val)); }
    bool isValidType(const std::type_info& type) const {
        static const std::type_info& t = typeid(T);
        return t == type;
    }

private:
    T value;
};

class CompositeDataSet : public IDataSet {
public:
    virtual ~CompositeDataSet() {
        for (std::map<std::string, IDataSet*>::iterator it = dataSets.begin(); it != dataSets.end(); it++) {
            delete it->second;
        }
    }

    const std::vector<std::string>& getKeys() const { return keys; }
    const IDataSet& operator[](std::string key) const { return *dataSets.find(key)->second; }
    IDataSet& operator[](std::string key) { return *dataSets.find(key)->second; }

    void addData(std::string key, IDataSet* dataSet) {
        keys.push_back(key);
        dataSets[key] = dataSet;
    }

private:
    std::vector<std::string> keys;
    std::map<std::string, IDataSet*> dataSets;
};

// ----------------------------------------------------


template <typename T>
inline T IDataSet::get(const T& defaultValue) const {
    static const std::type_info& type = typeid(T);
    if (isValidType(type)) {
        return *static_cast<const T*>(getValue(type));
    }

    return defaultValue;
}

template <typename T>
inline bool IDataSet::set(T val) {
    static const std::type_info& type = typeid(T);
    if (isValidType(type)) {
        setValue(type, &val);
        return true;
    }

    return false;
}

template <typename T>
inline bool IDataSet::isType() {
    static const std::type_info& type = typeid(T);
    return isValidType(type);
}

}

#endif