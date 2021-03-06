//
//  arena.cpp
//  Corridor-Simulation
//
//  Created by samuel on 7/7/18.
//  Copyright © 2018 Samuel. All rights reserved.
//

#include <cmath>
#include "arena.hpp"
#include <iostream>
#include "Bus.hpp"

double max(double x, double y){
    return x > y ? x : y;
}

double min(double x, double y){
    return x < y ? x : y;
}

double cdfexp(double t, double lambda){
    return 1.0 - exp(-lambda * t);
}

double randu(){
    return double(rand())/double(RAND_MAX);
}

double randn(){
    static const double two_pi = 2 * 3.1415926;
    double u1 = randu(), u2 = randu();
    return sqrt(-2.0 * log(u1)) * cos(two_pi * u2);
}

double gammaServTime(double avg, double cs){
    std::random_device r;
    std::default_random_engine generator(r());
    double k = 1.0/cs;
    double theta = avg/k;
    std::gamma_distribution<double> distribution(k, theta);
    double res = distribution(generator);
//    std::cout << res << std::endl;
    return res;
}

void expTime(std::deque<double> &result, double hdw, double duration){
    std::random_device r;
    std::default_random_engine generator(r());
    // hdw is second/veh
    std::exponential_distribution<double> distribution(1.0/hdw);
    double period = distribution(generator);
    if (period <= duration) {
        result.push_back(period);
    }else{
        return;
    }
    while (result.back() <= duration) {
        double period = distribution(generator);
        double moment = result.back() + period;
        if (moment <= duration) result.push_back(moment);
        else break;
    }
    return;
}

//void expTimeAnlt(std::deque<double> &result, double hdw, double duration){
//    while (true) {
//        double uniNumber = randu();
//        std::cout << "uniNumber: " << uniNumber << std::endl;
//        double nextTime = -log(uniNumber) / (1/hdw);
//        std::cout << "nextTime: " << nextTime << std::endl;
//        if (result.empty()) {
//            if (nextTime <= duration) {
//                result.push_back(nextTime);
//            }
//        }else{
//            if (result.back()+nextTime <= duration) {
//                double moment = result.back() + nextTime;
//                result.push_back(moment);
//            }else{
//                break;
//            }
//        }
//    }
//}


void gaussianTimeIndepent(std::deque<double> &result, double hdw, double cv, double offset, double duration){
    int lastScheduleNo = 0;
    while (true){
        // schedule 20 one time
        for(int l = lastScheduleNo; l < lastScheduleNo+20; l++){
//            double arriveTime = (l+offset) * hdw + randn() * (cv*hdw);
            double arriveTime = (l+1) * hdw + randn() * (cv*hdw);
            if (arriveTime >= duration) {
                // sort the arrival time
                std::sort(result.begin(), result.end());
                return;
            }else{
                result.push_back(arriveTime);
            }
        }
        lastScheduleNo += 20;
    }
    return;
}

void gaussianTime(std::deque<double> &result, double hdw, double cv, double duration){
    std::random_device r;
    std::default_random_engine generator(r());
    
    // hdw is the mean, second/veh
    std::normal_distribution<double> distribution(hdw, cv*hdw);
    double period = distribution(generator);
    if (period <= duration) {
        result.push_back(period);
    }else{
        return;
    }
    while (result.back() <= duration) {
        double period = distribution(generator);
//        std::cout << period << std::endl;
        double moment = result.back() + period;
        if (moment <= duration) result.push_back(moment);
        else break;
    }
    return;
}

int binomial(int val, double prob){
    std::random_device r;
    std::default_random_engine generator(r());
    std::binomial_distribution<int> distribution(val, prob);
    return distribution(generator);
}

void copyVector(std::vector<double> &base, std::vector<double> &&inc){
    base = move(inc);
}

void subVector(std::vector<double> &base, std::vector<double> &inc){
    size_t n = base.size();
    for(size_t i = 0; i < n; ++i){
        base[i] -= inc[i];
    }
}

void multiplyVector(std::vector<double> &base, double mul){
    size_t n = base.size();
    for(size_t i = 0; i < n; ++i){
        base[i] *= mul;
    }
}

double sumVector(vd &base){
    double res = 0.0;
    for (size_t i = 0; i < base.size(); i++) {
        res += base[i];
    }
    return res;
}

void addVector(std::vector<double> &base, std::vector<double> &inc){
    size_t n = base.size();
    for(size_t i = 0; i < n; ++i){
        base[i] += inc[i];
    }
}

double calMean(const std::vector<double> &vec){
    if (vec.size() == 0) return 0.0;
    double meanSum = 0.0;
    for(auto &x: vec){
        meanSum += x;
    }
    double size = double(int(vec.size()));
    double mean = meanSum / size;
    return  mean;
}

double calVariance(const std::vector<double> &vec){
    if (vec.size() == 0) return 0.0;
    double meanSum = 0.0, squareMeanSum = 0.0;
    for(auto &x: vec){
//        std::cout << "variance list is : " << x ;
        meanSum += x;
        squareMeanSum += (x*x);
    }
//    std::cout << std::endl;
    double size = double(int(vec.size()));
    double mean = meanSum / size;
    return  squareMeanSum/size - mean*mean;
}

int computeRuns(std::map<int, std::vector<double>> estimatingRunsMap){
//    std::vector<double> totalDelayVec;
//    for (auto &m: estimatingRunsMap){
//        double sums = sumVector(m.second);
//        totalDelayVec.push_back(sums);
//    }
//    double mean = sumVector(totalDelayVec) / (double)totalDelayVec.size();
//    double var = calVariance(totalDelayVec);
//    int nruns = int(var/(0.2*mean));
//    return nruns;
    
    std::vector<double> totalIndicatorVec;
    std::vector<double> largestIndicatorVec;
    for (auto &m: estimatingRunsMap){
        // total delay as indicator for repeating
        double sums = sumVector(m.second);
        totalIndicatorVec.push_back(sums); // in minute
//        std::cout << sums << std::endl;
//        std::cout << m.second[m.second.size()-2] << std::endl;
        largestIndicatorVec.push_back(m.second[m.second.size()-2]); // in minute
    }
//    double mean = sumVector(largestIndicatorVec) / (double)largestIndicatorVec.size();
//    double var = calVariance(totalIndicatorVec);
//    double var = calVariance(largestIndicatorVec);
    
//    double nruns = sqrt(var) / (mean * 0.02); //0.02, 0.1
//    std::cout << "variance is : " << var << "" << std::endl;
    
    // delay case
//    double parameter_delay = 0.001;
//    double delay_nruns = var / parameter_delay;
//    std::cout << "delay nruns is : " << delay_nruns << "" << std::endl;
    
    // cv case
//    double parameter_cv = 0.005;
//    double cv_nruns = var / parameter_cv;
//    std::cout << "cv nruns is: " << cv_nruns << "" << std::endl;
    
//    nruns = var / 0.1;
//    std::cout << nruns << std::endl;
//    return (int)nruns;
    return -100;
    
    
//    int stopSize = int(estimatingRunsMap[0].size()-1);
//    int maxRuns = 0;
//    for (int stop = 0; stop < stopSize; stop++) { // no consolidation stop, for simplicity
//        std::vector<double> eachStop;
//        for (auto &m: estimatingRunsMap){
//            eachStop.push_back(m.second[stop]);
//        }
//        double mean = sumVector(eachStop) / (double)(eachStop.size());
//        double var = calVariance(eachStop);
//        // 1.0 is the threshold
//        int nruns = int(var/(0.25*mean));
//        if (nruns > maxRuns) maxRuns = nruns;
//        eachStop.clear();
//    }
//    return maxRuns;
}

void computeMeanDelay(vd &stopDelays, vd &stopDelayStds, vd &meanDwellTimes, vd &cvDwellTimes, vd &stopEntryDelays, vd &stopExitDelays, vd &stopPaxNos, std::vector<std::shared_ptr<Bus>> busPtrs){
    
    
    int stopSize = int(meanDwellTimes.size());
    // last index (i.e. stopSize) is the consolidation stop
    for (auto d:stopDelays) d = 0.0; // initialization
    std::vector<int> stopSamples (stopSize+1);
    
    // initialization
    for (int k = 0; k < stopSize; k++){
        stopSamples[k] = 0;
    }
    stopSamples[stopSize] = 0;
    
    // collecting normal stop stats ...
    for (int s = 0; s < stopSize; s++) {
        std::vector<double> dwellArrayEachStop;
        std::vector<double> delaysArrayEachStop;
        for (auto &bus: busPtrs){
            if (bus->isEnterEachStop[s]) {
                dwellArrayEachStop.push_back(bus->serviceTimeAtEachStop[s]);
                delaysArrayEachStop.push_back(bus->delayAtEachStop[s]);
            }
        }
        meanDwellTimes[s] = calMean(dwellArrayEachStop);
        cvDwellTimes[s] = double(sqrt(calVariance(dwellArrayEachStop)) / meanDwellTimes[s]);
        stopDelays[s] = calMean(delaysArrayEachStop);
        stopDelayStds[s] = sqrt(calVariance(delaysArrayEachStop));
        
//        serviceSumsMap.insert(std::make_pair(s, stopDelays));
    }
    // colllecting ordering delays ...
    std::vector<double> delaysArrayAtOrdering;
    for (auto &bus: busPtrs) delaysArrayAtOrdering.push_back(bus->delayAtEachStop[-1]);
    stopDelays[stopSize] = calMean(delaysArrayAtOrdering);
    stopDelayStds[stopSize] = 0.0; // no need, just set to 0
    
    for (auto &bus: busPtrs){
        for (int s = 0; s < stopSize; s++) {
            if (bus->isEnterEachStop[s]) {
//                stopDelays[s] += bus->delayAtEachStop[s];
//                meanDwellTimes[s] += bus->serviceTimeAtEachStop[s];
                stopEntryDelays[s] += bus->entryDelayEachStop[s];
                stopExitDelays[s] += bus->exitDelayEachStop[s];
                stopPaxNos[s] += bus->paxNoEachStop[s];
                stopSamples[s] += 1;
            }
        }
//        stopDelays[stopSize] += bus->delayAtEachStop[-1];
        stopSamples[stopSize] += 1;
    }
    
    
    // calculate the mean
    for (int s = 0; s < stopSize; s++) {
        if (stopSamples[s] > 0) {
//            stopDelays[s] = stopDelays[s] / stopSamples[s];
//            meanDwellTimes[s] = meanDwellTimes[s] / stopSamples[s];
            stopEntryDelays[s] = stopEntryDelays[s] / stopSamples[s];
            stopExitDelays[s] = stopExitDelays[s] / stopSamples[s];
            stopPaxNos[s] = stopPaxNos[s] / stopSamples[s];
        }else{
//            stopDelays[s] = 0.0;
//            meanDwellTimes[s] = 0.0;
            stopEntryDelays[s] = 0.0;
            stopExitDelays[s] = 0.0;
            stopPaxNos[s] = 0.0;
        }
    }
//    if (stopSamples[stopSize] > 0) {
//        stopDelays[stopSize] = stopDelays[stopSize] / stopSamples[stopSize];
//    }else{
//        stopDelays[stopSize] = 0.0;
//    }
}

void getMapFromStringFlow(std::stringstream &ss, std::map<int, double> &map){
    int number = 0;
    while(ss.good()){
        std::string substr;
        getline(ss, substr, ',');
        map.insert(std::make_pair(number, std::stod(substr)));
        number ++;
    }
}

//void computeBunchingRMSE(vd &stopRMSE, std::vector<std::shared_ptr<Bus>> busPtrs, double busFlow, double travelTime, double warmupTime){
//    int stopSize = int(stopRMSE.size());
//    double lostTime = 10.0;
//    double boardingRate = 4.0;
//    // calculate the first peak bus schedule arrival
//    double headway = 3600/busFlow; //in seconds
//    double firstScheduleArrivalTime = ceil(warmupTime/headway) * headway;
//
//    // only calculate one specific line bunching rate
//    int count = 0;
//    for (auto &bus: busPtrs){
//        if (bus->busLine == 0) {
//            for (int s = 0; s < stopSize; s++) {
//                int busRuns = bus->busID;
//                double scheduledArrivalTime = 0.0;
//                if (s == 0) {
//                    scheduledArrivalTime = firstScheduleArrivalTime + busRuns*headway + travelTime;
//                }else{
//                    scheduledArrivalTime = firstScheduleArrivalTime + busRuns*headway + s*(lostTime+boardingRate*headway+travelTime);
//                }
//                double actualArriveTime = bus->arrivalTimeEachStop[s];
//                stopRMSE[s] += (actualArriveTime-scheduledArrivalTime) * (actualArriveTime-scheduledArrivalTime);
//            }
//            count ++;
//        }
//    }
//    for (int s = 0; s < stopSize; s++) {
//        stopRMSE[s] = sqrt(stopRMSE[s] / count);
//    }
//}

//void calculateBunchingRMSE(vd &stopRMSE, vd &stopDepartureRMSE, std::vector<std::shared_ptr<Bus>> busPtrs, double busFlow){
//    // first calculate the actual arrival headway
//    // i.e., h_{n,s} = a_{n,s} - a_{n-1,s}
//    int stopSize = int(stopRMSE.size());
//    double headway = 3600/busFlow; //in seconds
//    for (int s = 0; s < stopSize; s++) {
//        std::vector<double> actualArrivals;
//
//        for (auto &bus: busPtrs){
//            // only calculate one specific line's arrival headway
//            if (bus->busLine == 0) {
//                if (bus->arrivalTimeEachStop[s] > 0 ) { //0 means not reaching the downstream stop
//                    actualArrivals.push_back(bus->arrivalTimeEachStop[s]);
//                }
//            }
//        }
//        double squareErrorSum = 0.0;
//        int samples = int(actualArrivals.size());
//        std::sort(actualArrivals.begin(), actualArrivals.end());
//        for (int is = 0; is < samples-1; is++) {
//            if (actualArrivals[is+1] == 0 || actualArrivals[is] == 0) {
//                // some buses not reaching some downstream stops
//                // do not count
//            }else{
//                squareErrorSum += pow(actualArrivals[is+1] - actualArrivals[is] - headway, 2);
//            }
//        }
//        stopRMSE[s] = sqrt(squareErrorSum / (samples-1));
//    }
//}

void calculateHeadwayVariation(int kLine, vd &arrivalHeadwayMean, vd &arrivalHeadwayCv, vd &entryHeadwayMean, vd &entryHeadwayCv, vd &departureHeadwayMean, vd &departureHeadwayCv, std::vector<std::shared_ptr<Bus>> busPtrs){
    // first calculate the actual arrival headway
    // i.e., h_{n,s} = a_{n,s} - a_{n-1,s}
    int stopSize = int(entryHeadwayMean.size());
//    double headway = 3600/busFlow; //in seconds
    
    for (int s = 0; s < stopSize; s++) {
        std::vector<double> lineTotalArrHdwMeans;
        std::vector<double> lineTotalArrHdwVars;
        std::vector<double> lineTotalDptHdwMeans;
        std::vector<double> lineTotalDptHdwVars;
        std::vector<double> lineTotalEntryHdwMeans;
        std::vector<double> lineTotalEntryHdwVars;
        
        for (int kl = 0; kl < kLine; kl++) {
            std::vector<double> actualArrivals;
            std::vector<double> actualEntries;
            std::vector<double> actualDeparts;
            for (auto &bus: busPtrs){
                // calculate one specific line's arrival headway
                if (bus->busLine == kl) {
                    if (bus->arrivalTimeEachStop[s] > 0 ) { //0 means not reaching the downstream stop
                        actualArrivals.push_back(bus->arrivalTimeEachStop[s]);
                    }
                    if (bus->entryTimeEachStop[s] > 0) {
                        actualEntries.push_back(bus->entryTimeEachStop[s]);
                    }
                    if (bus->departureTimeEachStop[s] > 0) {
                        actualDeparts.push_back(bus->departureTimeEachStop[s]);
                    }
                }
            }
            auto arrivalResults = calHeadwayStatsFromTimes(actualArrivals);
            lineTotalArrHdwMeans.push_back(arrivalResults.first);
            lineTotalArrHdwVars.push_back(arrivalResults.second);
            auto entryResults = calHeadwayStatsFromTimes(actualEntries);
            lineTotalEntryHdwMeans.push_back(entryResults.first);
            lineTotalEntryHdwVars.push_back(entryResults.second);
            auto departResults = calHeadwayStatsFromTimes(actualDeparts);
            lineTotalDptHdwMeans.push_back(departResults.first);
            lineTotalDptHdwVars.push_back(departResults.second);
        }
        // take the average
        arrivalHeadwayMean[s] = calMean(lineTotalArrHdwMeans);
        arrivalHeadwayCv[s] = calMean(lineTotalArrHdwVars);
        entryHeadwayMean[s] = calMean(lineTotalEntryHdwMeans);
        entryHeadwayCv[s] = calMean(lineTotalEntryHdwVars);
        departureHeadwayMean[s] = calMean(lineTotalDptHdwMeans);
        departureHeadwayCv[s] = calMean(lineTotalDptHdwVars);
    }
}

std::pair<double, double> calHeadwayStatsFromTimes(vd &times){
//    double squareErrorSum = 0.0;
    std::vector<double> actualHeadways;
    int samples = int(times.size());
    std::sort(times.begin(), times.end());
    for (int is = 0; is < samples-1; is++) {
        if (times[is+1] == 0 || times[is] == 0) {
            // some buses not reaching some downstream stops
            // do not count
        }else{
            actualHeadways.push_back(times[is+1] - times[is]);
//            squareErrorSum += pow(times[is+1] - times[is] - headway, 2);
        }
    }
//    sqrt(squareErrorSum / (samples-1))
    double mean = calMean(actualHeadways);
    return std::make_pair(mean, sqrt(calVariance(actualHeadways)) / mean);
}

//std::pair<double, double> meanCvStats(const std::vector<double> &inputVector){
//    double mean = 0.0, stdvar = 0.0;
//    for(auto &x: inputVector){
//        mean += x;
//        stdvar += (x*x);
//    }
//    int test_runs = (int)inputVector.size();
//    mean /= double(test_runs);
//    stdvar /= double(test_runs);
//    stdvar = sqrt(stdvar - mean * mean);
//    return std::make_pair(mean, stdvar);
//}

//void print_vector(std::vector<double> const &input){
//    for (int i = 0; i < input.size(); i++) {
//        std::cout << input.at(i) << ' ';
//    }
//}

void writeJsonToFile(nlohmann::json js){
    std::string s = js.dump();
    std::ofstream file;
    file.open("/Users/samuel/Desktop/corridor-animation/data.json");
    file << s;
    file.close();
}





