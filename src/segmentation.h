/******************************************************************************
 *                                                                            *
 * Copyright (C) 2020 Fondazione Istituto Italiano di Tecnologia (IIT)        *
 * All Rights Reserved.                                                       *
 *                                                                            *
 ******************************************************************************/

#ifndef SEGMENTATION_H
#define SEGMENTATION_H

#include <memory>
#include <vector>
#include <cmath>
#include <algorithm>
#include <random>
#include <functional>
#include <limits>
#include <yarp/sig/PointCloud.h>

using namespace std;
using namespace yarp::sig;

/******************************************************************************/
class Segmentation {
public:
    /**************************************************************************/
    static double RANSAC(shared_ptr<yarp::sig::PointCloud<DataXYZRGBA>> pc_scene,
                         shared_ptr<yarp::sig::PointCloud<DataXYZRGBA>> pc_table,
                         shared_ptr<yarp::sig::PointCloud<DataXYZRGBA>> pc_object,
                         const int num_points = 100) {

        // generate random indexes
        random_device rnd_device;
        mt19937 mersenne_engine(rnd_device());
        uniform_int_distribution<int> dist(0, pc_scene->size() - 1);
        auto gen = bind(dist, mersenne_engine);
        vector<int> remap(num_points);
        generate(begin(remap), end(remap), gen);

        // implement RANSAC
        auto threshold_1 = .01F; // [cm]
        for (size_t i = 0; i < remap.size(); i++) {
            auto& pi = (*pc_scene)(remap[i]);
            auto h = 0.F;
            size_t n = 0;
            for (size_t j = 0; j < remap.size(); j++) {
                auto& pj = (*pc_scene)(remap[j]);
                if (fabs(pj.z - pi.z) < threshold_1) {
                    h += pj.z;
                    n++;
                }
            }
            h /= n;

            if (n > (remap.size() >> 1)) {
                pc_table->clear();
                pc_object->clear();
                auto threshold_2 = h + 1.1F * threshold_1;
                for (size_t i = 0; i < pc_scene->size(); i++) {
                    auto& p = (*pc_scene)(i);
                    if (p.z < threshold_2) {
                        pc_table->push_back(p);
                    } else {
                        pc_object->push_back(p);
                    }
                }
                return h;
            }
        }

        return numeric_limits<double>::quiet_NaN();
    }
};

#endif