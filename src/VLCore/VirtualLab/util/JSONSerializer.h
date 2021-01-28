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

private:

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
            val = picojson::value(d);
        }
        else if (dataSet.isType<Object>()) {
            picojson::object obj;
            const Object& dataSetObj = dataSet.get<Object>();

            for (Object::const_iterator it = dataSetObj.begin(); it != dataSetObj.end(); it++) {
                obj[it->first] = serializeJSON(it->second);  
            }
            
            val = picojson::value(obj);
        }

        return val;
    }

    void deserializeJSON(picojson::value& json, DataValue& dataSet) const {
        if (json.is<double>()) {
            dataSet.set<double>(json.get<double>());
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
    }

    virtual DataValue createDataSet(const picojson::value& val) const {
        if (val.is<double>()) {
            return DoubleDataValue();
        }
        else if (val.is<picojson::object>()) {
            return DataObject();
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