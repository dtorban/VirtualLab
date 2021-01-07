#ifndef VIRTUALLAB_UTIL_JSON_SERIALIZER_H_
#define VIRTUALLAB_UTIL_JSON_SERIALIZER_H_

#include <picojson.h>
#include "VirtualLab/IDataSet.h"

namespace vl {

class JSONSerializer {
public:
    static const JSONSerializer& instance() {
        static JSONSerializer instance;
        return instance;
    }

    std::string serialize(const IDataSet& dataSet) const {
        std::string serializedValue = serializeJSON(dataSet).serialize();

        return serializedValue;
    }

    void deserialize(const std::string& json, IDataSet& dataSet) const {
        picojson::value val;
        picojson::parse(val, json);
        std::string err = picojson::parse(val, json);
        deserializeJSON(val, dataSet);
    }

    picojson::value serializeJSON(const IDataSet& dataSet) const {
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
    }

private:

    virtual IDataSet* createDataSet(const picojson::value& val) const {
        if (val.is<double>()) {
            return new TypedData<double>();
        }
        else if (val.is<picojson::object>()) {
            return new CompositeDataSet();
        }

        return NULL;
    }

};

}

#endif