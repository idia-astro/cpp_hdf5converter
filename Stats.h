#ifndef __STATS_H
#define __STATS_H

#include "common.h"
#include "Util.h"

struct Stats {
    Stats() {}
    
    Stats(StatsDims dims) : dims(dims) {}
    
    void createDatasets(H5::Group group, std::string name) {
        H5::FloatType floatType(H5::PredType::NATIVE_FLOAT);
        floatType.setOrder(H5T_ORDER_LE);
        
        H5::IntType intType(H5::PredType::NATIVE_INT64);
        intType.setOrder(H5T_ORDER_LE);
        
        createHdf5Dataset(minDset, group, "Statistics/" + name + "/MIN", floatType, dims.statsDims);
        createHdf5Dataset(maxDset, group, "Statistics/" + name + "/MAX", floatType, dims.statsDims);
        createHdf5Dataset(sumDset, group, "Statistics/" + name + "/SUM", floatType, dims.statsDims);
        createHdf5Dataset(ssqDset, group, "Statistics/" + name + "/SUM_SQ", floatType, dims.statsDims);
        createHdf5Dataset(nanDset, group, "Statistics/" + name + "/NAN_COUNT", intType, dims.statsDims);
        
        if (dims.histSize) {
            createHdf5Dataset(histDset, group, "Statistics/" + name + "/HISTOGRAM", intType, dims.histDims);
        }
    }
    
    void createBuffers(hsize_t statsSize, hsize_t histSize = 0, hsize_t partialHistSize = 0) {
        // TODO this will be replaced with buffer code
        minVals.resize(statsSize, std::numeric_limits<double>::max());
        maxVals.resize(statsSize, -std::numeric_limits<double>::max());
        sums.resize(statsSize);
        sumsSq.resize(statsSize);
        nanCounts.resize(statsSize);
        
        histograms.resize(histSize);
        partialHistograms.resize(partialHistSize);
    }
    
    void resetBuffers() {
        // TODO this will be replaced with buffer code
        std::fill(minVals.begin(), minVals.end(), std::numeric_limits<double>::max());
        std::fill(maxVals.begin(), maxVals.end(), -std::numeric_limits<double>::max());
        std::fill(sums.begin(), sums.end(), 0);
        std::fill(sumsSq.begin(), sumsSq.end(), 0);
        std::fill(nanCounts.begin(), nanCounts.end(), 0);
        
        std::fill(histograms.begin(), histograms.end(), 0);
        std::fill(partialHistograms.begin(), partialHistograms.end(), 0);
    }
    
    void writeBasic(const std::vector<hsize_t>& dims, const std::vector<hsize_t>& count = EMPTY_DIMS, const std::vector<hsize_t>& start = EMPTY_DIMS) {
        writeHdf5Data(minDset, minVals, dims, count, start);
        writeHdf5Data(maxDset, maxVals, dims, count, start);
        writeHdf5Data(sumDset, sums, dims, count, start);
        writeHdf5Data(ssqDset, sumsSq, dims, count, start);
        writeHdf5Data(nanDset, nanCounts, dims, count, start);
    }
    
    void writeHistogram(const std::vector<hsize_t>& dims, const std::vector<hsize_t>& count = EMPTY_DIMS, const std::vector<hsize_t>& start = EMPTY_DIMS) {
        writeHdf5Data(histDset, histograms, dims, count, start);
    }
    
    // TODO this no longer needs to be static
    static hsize_t size(StatsDims dims) {
        return (4 * sizeof(double) + sizeof(int64_t)) * dims.statsSize + sizeof(int64_t) * (dims.histSize + dims.partialHistSize);
    }
    
    // TODO replace with individual properties
    StatsDims dims;

    // Buffers -- TODO replace with buffer objects
    std::vector<double> minVals;
    std::vector<double> maxVals;
    std::vector<double> sums;
    std::vector<double> sumsSq;
    std::vector<int64_t> nanCounts;
    
    std::vector<int64_t> histograms;
    std::vector<int64_t> partialHistograms;
    
    // Datasets
    H5::DataSet minDset;
    H5::DataSet maxDset;
    H5::DataSet sumDset;
    H5::DataSet ssqDset;
    H5::DataSet nanDset;
    
    H5::DataSet histDset;
};

#endif
