#ifndef VIRTUALLAB_IDATA_SET_H_
#define VIRTUALLAB_IDATA_SET_H_

#include <vector>
#include <string>
#include <map>
#include <typeinfo>
#include <vector>

namespace vl {

class IDataSet {
public:
    virtual ~IDataSet() {}
    
    template <typename T>
    T get(const T& defaultValue = T()) const;

    template <typename T>
    bool set(T val);

    template <typename T>
    bool isType() const;

    virtual const std::vector<std::string>& getKeys() const {
        static std::vector<std::string> keys;
        return keys;
    }

    virtual bool containsKey(std::string key) const { return false; }
    virtual const IDataSet& operator[](std::string key) const { return *this; }
    virtual IDataSet& operator[](std::string key) { return *this; }
    virtual void addData(std::string key, IDataSet* dataSet) {}

protected:
    virtual const void* getValue(const std::type_info& type) const { return NULL; }
    virtual void setValue(const std::type_info& type, void* val) {}
    virtual bool isValidType(const std::type_info& type) const { return false; }

    const void* getValue(const std::type_info& type, const IDataSet& dataSet) const { return dataSet.getValue(type); }
    void setValue(const std::type_info& type, void* val, IDataSet& dataSet) { dataSet.setValue(type, val); }
    bool isValidType(const std::type_info& type, const IDataSet& dataSet) const { return dataSet.isValidType(type); }
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
    bool containsKey(std::string key) const { return dataSets.find(key) != dataSets.end(); }
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

class DataSetRef : public IDataSet {
public:
    DataSetRef(IDataSet& dataSet) : dataSet(dataSet) {}
    ~DataSetRef() {}

    bool containsKey(std::string key) const { return dataSet.containsKey(key); }
    const std::vector<std::string>& getKeys() const { return dataSet.getKeys(); }
    const IDataSet& operator[](std::string key) const { return dataSet[key]; }
    IDataSet& operator[](std::string key) { return dataSet[key]; }
    void addData(std::string key, IDataSet* ds) { dataSet.addData(key, ds); }

protected:
    const void* getValue(const std::type_info& type) const { return IDataSet::getValue(type, dataSet); }
    void setValue(const std::type_info& type, void* val) { IDataSet::setValue(type, val, dataSet); }
    bool isValidType(const std::type_info& type) const { return IDataSet::isValidType(type, dataSet); }

private:
    IDataSet& dataSet;
};

class DataSetStack : public IDataSet {
public:
    DataSetStack() {
        dataSets.push_back(new CompositeDataSet());
    }
    ~DataSetStack() {
        for (IDataSet* dataSet : dataSets) {
            delete dataSet;
        }
    }

    void pop() {
        if (dataSets.size() > 1) {
            delete dataSets.back();
            dataSets.pop_back();
        }
    }

    void push() {
        dataSets.push_back(new CompositeDataSet());
    }

    IDataSet& top() {
        return *dataSets.back();
    }

    bool containsKey(std::string key) const {
        for (int i = dataSets.size()-1; i >= 0; i--) {
            if (dataSets[i]->containsKey(key)) {
                return true;
            }
        }

        return false;
    }


    const std::vector<std::string>& getKeys() const { return dataSets.back()->getKeys(); }

    const IDataSet& operator[](std::string key) const {
        for (int i = dataSets.size()-1; i >= 0; i--) {
            if (dataSets[i]->containsKey(key)) {
                return (*dataSets[i])[key];
            }
        }

        // need default
    }

    IDataSet& operator[](std::string key) {
        for (int i = dataSets.size()-1; i >= 0; i--) {
            if (dataSets[i]->containsKey(key)) {
                return (*dataSets[i])[key];
            }
        }

        // need default
    }

    void addData(std::string key, IDataSet* ds) { dataSets.back()->addData(key, ds); }

protected:
    const void* getValue(const std::type_info& type) const { return IDataSet::getValue(type, *dataSets.back()); }
    void setValue(const std::type_info& type, void* val) { IDataSet::setValue(type, val, *dataSets.back()); }
    bool isValidType(const std::type_info& type) const { return IDataSet::isValidType(type, *dataSets.back()); }

private:
    std::vector<IDataSet*> dataSets;
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
inline bool IDataSet::isType() const {
    static const std::type_info& type = typeid(T);
    return isValidType(type);
}

}

#endif