#ifndef VIRTUALLAB_UTIL_JSON_SERIALIZER_H_
#define VIRTUALLAB_UTIL_JSON_SERIALIZER_H_

#include <picojson.h>
#include "VirtualLab/IDataSet.h"
#include "VirtualLab/DataValue.h"

namespace vl {

class JSONSerializer {
public:
    static const JSONSerializer& instance() {
        static JSONSerializer instance;
        return instance;
    }

    /*std::string serialize(const IDataSet& dataSet) const {
        std::string serializedValue = serializeJSON(dataSet).serialize();

        return serializedValue;
    }

    void deserialize(const std::string& json, IDataSet& dataSet) const {
        picojson::value val;
        picojson::parse(val, json);
        std::string err = picojson::parse(val, json);
        deserializeJSON(val, dataSet);
    }*/

    std::string serialize(const DataValue& dataSet) const {
        std::string serializedValue = serializeJSON(dataSet).serialize();

        return serializedValue;
    }

    void deserialize(const std::string& json, DataValue& dataSet) const {
        picojson::value val;
        picojson::parse(val, json);
        std::string err = picojson::parse(val, json);
        deserializeJSON(val, dataSet);
    }

    /*picojson::value serializeJSON(const IDataSet& dataSet) const {
        picojson::value val;

        if (dataSet.isType<double>()) {
            double d = dataSet.get<double>();
            val = picojson::value(d);
        }
        else {
            picojson::object obj;

            for (std::string key : dataSet.getKeys()) {
                obj[key] = serializeJSON(dataSet[key]);                
            }
            
            val = picojson::value(obj);
        }

        return val;
    }

    void deserializeJSON(picojson::value& json, IDataSet& dataSet) const {
        if (json.is<double>()) {
            dataSet.set<double>(json.get<double>());
        }

        else if (json.is<picojson::object>()) {
            picojson::object obj = json.get<picojson::object>();
            for (picojson::object::iterator it = obj.begin(); it != obj.end(); it++) {
                if (!dataSet.containsKey(it->first)) {
                    IDataSet* ds = createDataSet(it->second);
                    if (ds) {
                        dataSet.addData(it->first, ds);
                    }
                }
                deserializeJSON(it->second, dataSet[it->first]);
            }
        }
    }*/

    picojson::value serializeJSON(const DataValue& dataSet) const {
        picojson::value val;

        if (dataSet.isType<double>()) {
            double d = dataSet.get<double>();
            if (!std::isnan(d) || std::isinf(d)) {
                val = picojson::value(d);
            }
            else {
                // TODO: handle nan or inf value
                val = picojson::value((double)0);
            }
        }
        else if (dataSet.isType<std::string>()) {
            std::string str = dataSet.get<std::string>();
            val = picojson::value(str);
        }
        else if (dataSet.isType<Object>()) {
            picojson::object obj;
            const Object& dataSetObj = dataSet.get<Object>();

            for (Object::const_iterator it = dataSetObj.begin(); it != dataSetObj.end(); it++) {
                obj[it->first] = serializeJSON(it->second);  
            }
            
            val = picojson::value(obj);
        }
        else if (dataSet.isType<vl::Array>()) {
            picojson::array arr;
            const vl::Array& dataSetArr = dataSet.get<vl::Array>();

            for (vl::Array::const_iterator it = dataSetArr.begin(); it != dataSetArr.end(); it++) {
                arr.push_back(serializeJSON(*it));  
            }
            
            val = picojson::value(arr);
        }

        return val;
    }

    void deserializeJSON(picojson::value& json, DataValue& dataSet) const {
        if (json.is<double>()) {
            dataSet.set<double>(json.get<double>());
        }
        else if (json.is<std::string>()) {
            dataSet.set<std::string>(json.get<std::string>());
        }
        else if (json.is<picojson::object>()) {
            picojson::object obj = json.get<picojson::object>();
            if (!dataSet.isType<Object>()) {
                dataSet = DataObject();
            }

            for (picojson::object::iterator it = obj.begin(); it != obj.end(); it++) {
                dataSet.get<Object>()[it->first] = createDataSet(it->second);
                deserializeJSON(it->second, dataSet.get<Object>()[it->first]);
            }
        }
        else if (json.is<picojson::array>()) {
            picojson::array arr = json.get<picojson::array>();
            if (!dataSet.isType<vl::Array>()) {
                dataSet = DataArray();
            }

            for (picojson::array::iterator it = arr.begin(); it != arr.end(); it++) {
                DataValue val = createDataSet(*it);
                deserializeJSON(*it, val);
                dataSet.get<vl::Array>().push_back(val);
            }
        }
    }

    virtual DataValue createDataSet(const picojson::value& val) const {
        if (val.is<double>()) {
            return DoubleDataValue();
        }
        else if (val.is<std::string>()) {
            return StringDataValue();
        }
        else if (val.is<picojson::object>()) {
            return DataObject();
        }
        else if (val.is<picojson::array>()) {
            return DataArray();
        }

        return DataValue();
    }

    /*virtual IDataSet* createDataSet(const picojson::value& val) const {
        if (val.is<double>()) {
            return new TypedData<double>();
        }
        else if (val.is<picojson::object>()) {
            return new CompositeDataSet();
        }

        return NULL;
    }*/

};

}

#endif