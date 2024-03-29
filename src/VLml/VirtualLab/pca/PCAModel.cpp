#include "VirtualLab/pca/PCAModel.h"

#ifdef USE_MLPACK
#include <mlpack/prereqs.hpp>
#include <mlpack/methods/pca/pca.hpp>
#include <mlpack/methods/pca/decomposition_policies/exact_svd_method.hpp>
#include <mlpack/methods/pca/decomposition_policies/quic_svd_method.hpp>
#include <mlpack/methods/pca/decomposition_policies/randomized_svd_method.hpp>
using namespace mlpack;
using namespace mlpack::pca;
using namespace mlpack::util;

#include <mlpack/methods/kmeans/kmeans.hpp>
using namespace mlpack::kmeans;

namespace vl {

template<typename DecompositionPolicy>
void RunPCA(arma::mat& dataset,
            const size_t newDimension,
            const bool scale,
            const double varToRetain)
{
  PCA<DecompositionPolicy> p(scale);
  double varRetained;

    varRetained = p.Apply(dataset, newDimension);
}

}

#endif

namespace vl {

class DensityGrid {
public:
    DensityGrid(int width, int height) : width(width), height(height) {
        totalSamples = 0;
        grid = new int[width*height];
        showGrid = new int[width*height];
        clear();
    }
    ~DensityGrid() {
        delete[] grid;
        delete[] showGrid;
    }

    void clear() {
        totalSamples = 0;
        for (int f = 0; f < width*height; f++) {
            grid[f] = 0;
            showGrid[f] = 0;
        }
    }

    void addSamplePoint(double x, double y, double* bounds) {
        int xVal = 1.0*(x-bounds[0])*width/(bounds[2]-bounds[0]);
        int yVal = 1.0*(y-bounds[1])*height/(bounds[3]-bounds[1]);
        if (xVal < 0) {xVal = 0;}
        if (yVal < 0) {yVal = 0;}
        if (xVal >= width) {xVal = width-1;}
        if (yVal >= height) {yVal = height-1;}
        //std::cout << xVal << " " << yVal << std::endl;
        grid[yVal*width + xVal]++;
        totalSamples++;
    }

    void calculateShown(int totalShown) {
        int leftToShow = totalShown;
        for (int i = 0; i < width*height; i++) {
            if (grid[i] > 0) {
                showGrid[i]++;
                leftToShow--;
            }
        }

        for (int i = 0; i < width*height; i++) {
            if (grid[i] > 0) {
                showGrid[i] += std::round(1.0*leftToShow*grid[i]/totalSamples);
            }
        }
        
        /*for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
                std::cout << grid[y*width + x] << " ";
            }
            std::cout << std::endl;
        }
        
        std::cout << " totalSamples: " << totalSamples << std::endl;*/

        /*int numShown = 0;
        for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
                std::cout << showGrid[y*width + x] << " ";
                numShown+=showGrid[y*width + x];
            }
            std::cout << std::endl;
        }

        std::cout << numShown << std::endl;*/
    }

    bool showSamplePoint(double x, double y, double* bounds) {
        int xVal = 1.0*(x-bounds[0])*width/(bounds[2]-bounds[0]);
        int yVal = 1.0*(y-bounds[1])*height/(bounds[3]-bounds[1]);
        if (xVal < 0) {xVal = 0;}
        if (yVal < 0) {yVal = 0;}
        if (xVal >= width) {xVal = width-1;}
        if (yVal >= height) {yVal = height-1;}
        if (showGrid[yVal*width + xVal] > 0) {
            showGrid[yVal*width + xVal]--;
            return true;
        }
        return false;
    }

private:
    int width;
    int height;
    int* grid;
    int* showGrid;
    int totalSamples;
};

class PCAModelSample : public IModelSample, public IDataConsumer {
public:
	PCAModelSample(const DataObject& params, PCAModel::SampleInfo& info, PCAModel& model) : params(params), model(model), callback(NULL), kmeans_calc(true), prevColumnSize(0), info(info), densityGrid(30,30), zoomGrid(30,30) {
        data["pca"] = DataArray();
        data["bounds"] = DataArray();
        data["vdi"] = DataArray();
        pca = &data["pca"].get<vl::Array>();
        bound = &data["bounds"].get<vl::Array>();
        vdi = &data["vdi"].get<vl::Array>();
    }
    virtual ~PCAModelSample() {
        std::cout << "Delete " << this << std::endl;
        model.removeConsumer(this);
        std::cout << "Delete remove " << this << std::endl;
    }

	virtual const DataObject& getParameters() const { return params; }
    virtual DataObject& getNavigation() { return nav; }
    virtual const DataObject& getData() const { return data; }
    virtual void update();
	void update(IUpdateCallback* callback);
	virtual void consume(IModel& model, IModelSample& sample);

private:
    DataObject params;
    DataObject nav;
    DataObject data;
    vl::Array* pca;
    vl::Array* bound;
    vl::Array* vdi;
	IUpdateCallback* callback;
    bool kmeans_calc;
    DataObject keys;
    std::map<std::string, int> keyType;
    int prevColumnSize;
    PCAModel::SampleInfo& info;
    DensityGrid densityGrid;
    DensityGrid zoomGrid;
    PCAModel& model;

    struct PcaInfo {
        PcaInfo(double x, double y) : x(x), y(y) {}
        void setValue(double x, double y) {
            this->x = this->x*0.95 + 0.05*x;
            this->y = this->y*0.95 + 0.05*y;
        }
        double x, y;
    };

    std::vector<PcaInfo> averagePCA;
    std::vector<PcaInfo> averageBounds;
#ifdef USE_MLPACK
    arma::Row<size_t> assignments;
    arma::mat centroids;
    std::vector< std::vector< std::pair<double, int> > > closest;
#endif
};


IModelSample* PCAModel::create(const DataObject& params) {
		PCAModelSample* sample = new PCAModelSample(params, info, *this);
		consumers.push_back(sample);
		return sample;
}

void PCAModelSample::update() {
    if (callback) {
        //std::cout << "update PCA" << std::endl;

        nav["keys"] = keys;
        DataObject zoom = nav["zoom"].get<vl::Object>();
        double zoomK = zoom["k"].get<double>();
        double zoomX = zoom["x"].get<double>();
        double zoomY = zoom["y"].get<double>();
        kmeans_calc = kmeans_calc || (zoom["u"].get<double>() > 0.000001);
        vl::Array zBounds = zoom["bounds"].get<vl::Array>();
        //std::cout << zoom["k"].get<double>() << std::endl;
        int params = 0;
        int other = 0;

        std::vector<std::string> cols;
        for (DataObject::const_iterator it = keys.begin(); it != keys.end(); it++) {
            if (it->second.get<double>() > 0.0001) {
                cols.push_back(it->first);
                int type = keyType[it->first];
                if (type == 0) {
                    params++;
                }
                else {
                    other++;
                }
            }
        }
        //std::cout << numCols << std::endl;

        if (prevColumnSize != cols.size()) {
            kmeans_calc = true;//info.dataRows.size()/100-1;
        }
        prevColumnSize = cols.size();

        pca->clear();
        bound->clear();
        vdi->clear();
        densityGrid.clear();
        zoomGrid.clear();

#ifdef USE_MLPACK
        if (cols.size() > 1 && (info.dataRows.size() > 2 || info.samplePtr.size() > 2)) {
                int numRows = info.dataRows.size();
                if (params > 0 && other == 0) {
                    numRows = info.paramRows.size();
                }

                arma::mat A(numRows, cols.size());
                
                if (params > 0 && other ==0) {
                    int i = 0;
                    for (std::map<IModelSample*, DataObject>::iterator it = info.paramRows.begin(); it != info.paramRows.end(); it++) {
                        ParameterHelper helper(it->second);
                        for (int f = 0; f < cols.size(); f++) {
                            A(i,f) = helper.scale(cols[f], it->second[cols[f]].get<double>());
                        }
                        i++;
                    }
                }
                else {
                    //B << sample->getNavigation()["t"].get<double>() << arma::endr << sample->getNavigation()["t"].get<double>() << arma::endr;
                    for (int i = 0; i < numRows; i++) {

                        DataObject& ps = info.paramRows[info.samplePtr[i]];
                        ParameterHelper helper(ps);
                        for (int f = 0; f < cols.size(); f++) {
                            double val;
                            const std::string& key = cols[f];
                            int type = keyType[key];
                            switch (type)
                            {
                            case 0:
                                val = helper.scale(key, ps[key].get<double>());
                                break;
                            case 1:
                                val = info.navRows[i][key].get<double>();
                                break;
                            case 2:
                                val = info.dataRows[i][key].get<double>();
                                break;
                            }
                            A(i,f) = val;
                        }
                    }
                }

                A = A.t();
                
                if (cols.size() != 2) { 
                    RunPCA<ExactSVDPolicy>(A, 2, true, 1.0);
                }

                for (int f = 0; f < numRows; f++) {
                    double x = A(0,f);
                    double y = A(1,f);
                    if (averagePCA.size() <= f) {
                        averagePCA.push_back(PcaInfo(x,y));
                    }
                    else {
                        averagePCA[f].setValue(x,y);
                    }
                    A(0,f) = averagePCA[f].x;
                    A(1,f) = averagePCA[f].y;
                }

                int clusterNum = this->params["clusters"].get<double>();
                if (clusterNum > 0) {
                    clusterNum = this->nav["clusters"].get<double>();
                }
                
                double bounds[4];
                double zoomBounds[4];

                for (int f = 0; f < numRows; f++) {
                    if (f == 0) {
                        // x,y min
                        bounds[0] = A(0,f);
                        bounds[1] = A(1,f);
                        // x,y max
                        bounds[2] = A(0,f);
                        bounds[3] = A(1,f);
                    }
                    else {
                        if (bounds[0] > A(0,f)) { bounds[0] = A(0,f); }
                        if (bounds[1] > A(1,f)) { bounds[1] = A(1,f); }
                        if (bounds[2] < A(0,f)) { bounds[2] = A(0,f); }
                        if (bounds[3] < A(1,f)) { bounds[3] = A(1,f); } 
                    }

                }

                if (averageBounds.size() <= 2) {
                    averageBounds.push_back(PcaInfo(bounds[0], bounds[1]));
                    averageBounds.push_back(PcaInfo(bounds[2], bounds[3]));
                }
                else {
                    averageBounds[0].setValue(bounds[0], bounds[1]);
                    averageBounds[1].setValue(bounds[2], bounds[3]);
                    bounds[0] = averageBounds[0].x;
                    bounds[1] = averageBounds[0].y;
                    bounds[2] = averageBounds[1].x;
                    bounds[3] = averageBounds[1].y;
                }
                
                for (int i = 0; i < 4; i++) {
                    if (zBounds.size() > 0) {
                        zoomBounds[i] = zBounds[i].get<double>();
                    }
                    else {
                        zoomBounds[i] = bounds[i];
                    }
                }

                

                for (int f = 0; f < numRows; f++) {
                    double x = A(0,f);
                    double y = A(1,f);
                    densityGrid.addSamplePoint(x,y,bounds);
                    if (x >= zoomBounds[0] && y >= zoomBounds[1] && x <= zoomBounds[2] && y <= zoomBounds[3]) {
                        zoomGrid.addSamplePoint(x,y,zoomBounds);
                    }
                }

                densityGrid.calculateShown(2000);
                zoomGrid.calculateShown(2000);


                std::vector<int> zoomIndices;

                for (int f = numRows-1; f >= 0; f--) {
                    if (f % 1 == 0) {
                        
                        vl::DataObject obj;
                        double x = A(0,f);
                        double y = A(1,f);
                        obj["x"] = DoubleDataValue(x);
                        obj["y"] = DoubleDataValue(y);
                        if (assignments.size() <= f) {
                            obj["cluster"] = DoubleDataValue(1);
                        }
                        else {
                            obj["cluster"] = DoubleDataValue(1);//assignments[f]+1);
                        }
                        //double radius = std::sqrt(std::pow(x - (bounds[2]+bounds[0])/2.0 + zoomX*(bounds[2]-bounds[0])/zoomK ,2) + std::pow(y - (bounds[3]+bounds[1])/2.0 - zoomY*(bounds[3]-bounds[1])/zoomK,2));
                        //double radius = std::sqrt(std::pow(x + zoomX*(bounds[2]-bounds[0])/zoomK ,2) + std::pow(y - zoomY*(bounds[3]-bounds[1])/zoomK,2));
                        //double radius = ;
                        //std::cout << radius << std::endl;
                        //if (std::abs(x-bounds[0] - (bounds[2]-bounds[0])/2.0/zoomK + zoomX*(bounds[2]-bounds[0])/zoomK) < (bounds[2]-bounds[0])/zoomK/4.0) {
                            //if (std::abs(y-bounds[1] - 0.0*(bounds[3]-bounds[1])/2.0/zoomK + zoomY*(bounds[3]-bounds[1])/zoomK) < (bounds[3]-bounds[1])/zoomK/4.0) {
                            //if (std::abs(y-bounds[1] - (bounds[3]-bounds[1])/2.0/zoomK - zoomY*(bounds[3]-bounds[1])/zoomK) < (bounds[3]-bounds[1])/zoomK/4.0) {
                            //if (std::abs(y-bounds[1] - (bounds[3]-bounds[1])/2.0) < (bounds[3]-bounds[1])/zoomK/4.0) {
                                //obj["cluster"] = DoubleDataValue(2);
                            //}
                        //}
                        /*if (std::abs((x-bounds[0] - (bounds[2]-bounds[0])/2.0) + 0.25*(bounds[2]-bounds[0])) < (bounds[2]-bounds[0])/zoomK/16.0) {
                            obj["cluster"] = DoubleDataValue(3);
                        }*/

                        /*if (zoomBounds.size() > 0) {
                            std::cout << x << " " << y << " " <<  zoomBounds[0].get<double>()   << " " << zoomBounds[1].get<double>()  << " " << zoomBounds[2].get<double>()  << " " << zoomBounds[3].get<double>() << std::endl;
                        }
                        if (zoomBounds.size() > 0 
                            && x >= zoomBounds[0].get<double>() 
                            && y >= zoomBounds[1].get<double>() 
                            && x <= zoomBounds[2].get<double>() 
                            && y <= zoomBounds[3].get<double>())
                        {
                            std::cout << "bounds" << std::endl;
                            obj["cluster"] = DoubleDataValue(3);
                        }*/

                        if (densityGrid.showSamplePoint(x, y, bounds)) {
                            pca->push_back(obj);
                        }
                        if (zoomGrid.showSamplePoint(x, y, zoomBounds)) {
                            pca->push_back(obj);
                            zoomIndices.push_back(f);
                        }
                        //(bounds[2]-bounds[0])/2.0


                        /*if (calcClosest && centroids.size() == clusterNum*2) {    
                            for (int i = 0; i < clusterNum; i++) {
                                double dist = std::sqrt(std::pow(centroids(0,i)-x,2) + std::pow(centroids(1,i)-y,2));
                                if (closest.size() < clusterNum) {
                                    closest.push_back(std::pair<double, int>(dist, f));
                                }
                                else if(dist < closest[i].first) {
                                    closest[i] = std::pair<double, int>(dist, f);
                                }
                            }
                        }*/
                    }
                }

                if (clusterNum > 0) {
                    //int blah = numRows/100;
                    if (kmeans_calc) {
                        for (int i = 0; i < numRows; i++) {
                            double x = A(0,i);
                            double y = A(1,i);
                            /*if (x >= zoomBounds[0] && y >= zoomBounds[1] && x <= zoomBounds[2] && y <= zoomBounds[3]) {
                                zoomIndices.push_back(i);
                            }*/
                        }

                        if (zoomIndices.size() > 0) {
                            arma::mat B(2, zoomIndices.size());
                            
                            for (int i = 0; i < zoomIndices.size(); i++) {
                                double x = A(0,zoomIndices[i]);
                                double y = A(1,zoomIndices[i]);
                                B(0,i) = x;
                                B(1,i) = y;
                            }

                            //std::cout << "Calc KMeans" << std::endl;
                            //kmeans_calc++;
                            kmeans_calc = false;
                            // The dataset we are clustering.
                            //extern arma::mat data;
                            // The number of clusters we are getting.
                            //extern size_t clusters = 5;
                            // The assignments will be stored in this vector.
                            //arma::Row<size_t> assignments;
                            // The centroids will be stored in this matrix.
                            //arma::mat centroids;
                            // Initialize with the default arguments.
                            KMeans<> k;
                            k.Cluster(B, clusterNum, assignments, centroids);
                            closest.clear();
                        }


                    }
                }

                

                bool calcClosest = closest.size() == 0;

                for (int f = numRows-1; f >= 0; f--) {
                //for (int f = 0; f < zoomIndices.size(); f++) {
                       
                    //double x = A(0,zoomIndices[f]);
                    //double y = A(1,zoomIndices[f]);
                    double x = A(0,f);
                    double y = A(1,f);

                    if (calcClosest && centroids.size() == clusterNum*2) {    
                        for (int i = 0; i < clusterNum; i++) {
                            double dist = std::sqrt(std::pow(centroids(0,i)-x,2) + std::pow(centroids(1,i)-y,2));
                            if (closest.size() < clusterNum) {
                                closest.push_back(std::vector< std::pair<double, int> >());
                            }

                            if (closest[i].size() < 10) {
                                closest[i].push_back(std::pair<double, int>(dist, f));
                            }
                            else {
                                for (int j = 0; j < closest[i].size(); j++) {
                                    if (closest[i][j].first > dist) {
                                        closest[i].insert(closest[i].begin()+j, std::pair<double, int>(dist, f));
                                        closest[i].erase(closest[i].begin()+closest[i].size() -1);
                                        break;
                                    }
                                }
                            } 
                        }
                    }
                    
                }

                if (centroids.size() == clusterNum*2) {                   
                    for (int f = 0; f < clusterNum; f++) {
                            vl::DataObject obj;
                            //obj["x"] = DoubleDataValue(centroids(0,f));
                            //obj["y"] = DoubleDataValue(centroids(1,f));
                            obj["x"] = DoubleDataValue(A(0,closest[f][0].second));
                            obj["y"] = DoubleDataValue(A(1,closest[f][0].second));
                            obj["cluster"] = DoubleDataValue(0);//DoubleDataValue(0);
                            pca->push_back(obj);

                            if (calcClosest) {
                                vl::DataObject close;
                                close["id"] = DoubleDataValue(closest[f][0].second);
                                //close["centroid_id"] = DoubleDataValue(pca->size()-1);
                                close["data"] = info.dataRows[closest[f][0].second];
                                //close["x"] = DoubleDataValue(centroids(0,f));
                                //close["y"] = DoubleDataValue(centroids(1,f));
                                close["x"] = DoubleDataValue(A(0,closest[f][0].second));
                                close["y"] = DoubleDataValue(A(1,closest[f][0].second));
                                DataArray neighbors;
                                for (int i = 0; i < closest[f].size(); i++) {
                                    vl::DataObject n;
                                    n["id"] = DoubleDataValue(closest[f][i].second);
                                    n["data"] = info.dataRows[closest[f][i].second];
                                    n["x"] = DoubleDataValue(A(0,closest[f][i].second));
                                    n["y"] = DoubleDataValue(A(1,closest[f][i].second));
                                    neighbors.get<vl::Array>().push_back(n);
                                }
                                close["n"] = neighbors;
                                vdi->push_back(close);
                            }
                            
                    }
                }

                for (int i = 0; i < 4; i++) {
                    bound->push_back(DoubleDataValue(bounds[i]));
                }
        }
#endif

        callback->onComplete();
        delete callback;
    }
    callback = NULL;
}

void PCAModelSample::update(IUpdateCallback* callback) {
    keys = this->nav["keys"];
    this->callback = callback;
}

void PCAModelSample::consume(IModel& model, IModelSample& sample) {
    if (!callback) {
        return;
    }

    std::cout << "consume called " << this << std::endl;

    const DataObject& obj = sample.getData();
    const DataObject& nav = sample.getNavigation();
    const DataObject& params = sample.getParameters();

    /*navRows.push_back(nav);
    dataRows.push_back(obj);
    samplePtr.push_back(&sample);
    if (paramRows.find(&sample) == paramRows.end()) {
        paramRows[&sample] = sample.getParameters();
    }*/

    bool paramsEnabled = this->params["params"].get<double>() > 0.0001 ? 1 : 0;
 
    for (DataObject::const_iterator it = params.begin(); it != params.end(); it++) {
        if (it->second.isType<double>()) {   
            if (keys.find(it->first) == keys.end()) {
                //std::cout << it->first << " " << paramsEnabled << std::endl;
                keys[it->first] = DoubleDataValue(paramsEnabled);
                keyType[it->first] = 0;
            }
        }
    }
    for (DataObject::const_iterator it = nav.begin(); it != nav.end(); it++) {
        if (it->second.isType<double>()) { 
            if (keys.find(it->first) == keys.end()) {
                //std::cout << it->first << std::endl;
                keys[it->first] = DoubleDataValue(0);
                keyType[it->first] = 1;
            }
        }
    }

    bool dataEnabled = this->params["data"].get<double>() > 0.0001 ? 1 : 0;

    for (DataObject::const_iterator it = obj.begin(); it != obj.end(); it++) {
        if (it->second.isType<double>()) { 
            if (keys.find(it->first) == keys.end()) {
                //std::cout << it->first << std::endl;
                keys[it->first] = DoubleDataValue(dataEnabled);
                keyType[it->first] = 2;
            }
        }
    }

    this->nav["keys"] = keys;

    const Object& dataSetObj = obj.get<Object>();

	update();
    std::cout << "consume end " << this << std::endl;
}

}