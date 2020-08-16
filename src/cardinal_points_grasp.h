/******************************************************************************
 *                                                                            *
 * Copyright (C) 2020 Fondazione Istituto Italiano di Tecnologia (IIT)        *
 * All Rights Reserved.                                                       *
 *                                                                            *
 ******************************************************************************/

#ifndef CARDINAL_POINTS_GRASP_H
#define CARDINAL_POINTS_GRASP_H

#include <memory>
#include <vector>
#include <tuple>
#include <utility>
#include <string>
#include <cmath>
#include <limits>
#include <algorithm>

#include <yarp/os/Value.h>
#include <yarp/os/Bottle.h>
#include <yarp/dev/CartesianControl.h>
#include <yarp/sig/Vector.h>
#include <yarp/sig/Matrix.h>
#include <yarp/math/Math.h>

#include <iCub/iKin/iKinFwd.h>

namespace cardinal_points_grasp {

typedef std::tuple<std::string, double, yarp::sig::Matrix> rankable_candidate;

/******************************************************************************/
class CardinalPointsGrasp {
    const std::string hand;
    const std::vector<double> pregrasp_fingers_posture;

    double pregrasp_aperture{0.};
    double hand_half_height{0.};
    double fingers_max_length{0.};
    double dist_center_index_middle{0.};
    double approach_min_distance{0.};
    yarp::sig::Matrix approach;

    std::vector<std::unique_ptr<iCub::iKin::iCubFinger>> fingers;

    auto composeCandidate(const yarp::sig::Vector& axis_x, const yarp::sig::Vector& axis_y,
                          const yarp::sig::Vector& axis_z, const yarp::sig::Vector& point,
                          const yarp::sig::Vector& dir) const {
        auto candidate = yarp::math::zeros(4, 4);
        candidate.setSubcol(axis_x, 0, 0);
        candidate.setSubcol(axis_y, 0, 1);
        candidate.setSubcol(axis_z, 0, 2);
        candidate.setSubcol(point, 0, 3);
        candidate(3, 3) = 1.;
        candidate = candidate * approach;
        candidate.setSubcol(candidate.getCol(3).subVector(0, 2) + approach_min_distance * dir, 0, 3);
        return candidate;
    }

    /**************************************************************************/
    auto evaluateCandidate(const yarp::sig::Matrix& candidate,
                           yarp::dev::ICartesianControl* iarm) const {
        const auto xd = candidate.getCol(3).subVector(0, 2);
        const auto od = yarp::math::dcm2axis(candidate);

        yarp::sig::Vector xdhat, odhat, qdhat;
        if (iarm->askForPose(xd, od, xdhat, odhat, qdhat)) {
            // always enforce reaching in position first
            if (yarp::math::norm(xd - xdhat) < .005) {
                const auto rot = yarp::math::dcm2axis(yarp::math::axis2dcm(od) *
                                 yarp::math::axis2dcm(odhat).transposed());
                // cost in [0, 1] = normalized angle difference
                const auto cost = std::abs(rot[3] / M_PI);
                if (180. * cost < 10.) {
                    return cost;
                }
            }
        }
        return std::numeric_limits<double>::infinity();
    }

public:
    /**************************************************************************/
    CardinalPointsGrasp() = delete;

    /**************************************************************************/
    CardinalPointsGrasp(const std::string& hand_, const std::vector<double>& pregrasp_fingers_posture_) :
                        hand(hand_), pregrasp_fingers_posture(pregrasp_fingers_posture_) {
        // create fingers and set them up in the pregrasp posture
        std::vector<std::string> fingers_names{"thumb", "index", "middle", "ring", "little"};
        for (auto& name:fingers_names) {
            fingers.push_back(std::make_unique<iCub::iKin::iCubFinger>(iCub::iKin::iCubFinger(hand+"_"+name)));
            auto& finger = fingers.back();
            finger->asChain()->setAllConstraints(false);
            yarp::sig::Vector motorEncoders(pregrasp_fingers_posture.size(), pregrasp_fingers_posture.data());
            yarp::sig::Vector chainJoints;
            finger->getChainJoints(motorEncoders, chainJoints);
            finger->setAng(chainJoints * (M_PI / 180.0));
        }

        // compute the pre-grasp quantities 
        const auto p_thumb = fingers[0]->EndEffPosition();
        const auto p_index = fingers[1]->EndEffPosition();
        const auto p_middle = fingers[2]->EndEffPosition();
        const auto dist_1 = yarp::math::norm(p_thumb - p_index);
        const auto dist_2 = yarp::math::norm(p_thumb - p_middle);
        pregrasp_aperture = std::min(dist_1, dist_2);
        const auto p_ = (dist_1 < dist_2 ? p_index : p_middle);
        const auto pregrasp_aperture_angle = std::acos(yarp::math::dot(p_thumb, p_) / 
                                                       (yarp::math::norm(p_thumb) * yarp::math::norm(p_)));

        const auto sector_beg = std::atan2(std::abs(p_[2]), p_[0]);
        const auto sector_end = std::atan2(std::abs(p_thumb[2]), p_thumb[0]);

        // account for safety margin due to ring and little fingers that might hamper the approach
        for (size_t i = 3; i < fingers.size(); i++) {
            auto& finger = fingers[i];
            for (size_t link = 0; link < finger->getN(); link++) {
                auto v = fingers[i]->Position(link);
                auto ang = std::atan2(std::abs(v[2]), v[0]);
                if ((ang >= sector_beg) && (ang <= sector_end)) {
                    v[1] = 0.;
                    approach_min_distance = std::max(yarp::math::norm(v), approach_min_distance);
                }
            }
        }
        approach_min_distance += .005;

        // use little finger to account for safety margin to avoid hitting the table when side-grasping
        hand_half_height = fingers.back()->EndEffPosition()[1] + .01;

        // compute the fingers max length
        for (auto& finger:fingers) {
            auto length = 0.;
            auto p = finger->EndEffPosition();
            for (size_t i = 0; i < 3; i++) {
                auto j = finger->getDOF() - i - 1;
                auto pi = (j >= 0 ? finger->Position(j) : finger->getH0().getCol(3).subVector(0, 2));
                length += yarp::math::norm(pi - p);
                p = pi;
            }
            fingers_max_length = std::max(length, fingers_max_length);
        }

        // for 3-fingers grasps, better off aligning the hand with the center between index and middle fingers
        const auto p0_index = fingers[1]->Position(1);
        const auto p0_middle = fingers[2]->getH0().getCol(3).subVector(0, 2);
        dist_center_index_middle = std::abs(p0_index[1] - p0_middle[1]) / 2.;

        // compute the approach frame wrt the canonical hand-centered frame
        approach = yarp::math::axis2dcm({0., 1., 0., sector_beg + pregrasp_aperture_angle * (hand == "right" ? -.3 : .3)});
    }

    /**************************************************************************/
    static auto compareCandidates(const rankable_candidate& c1, const rankable_candidate& c2) {
        return (std::get<1>(c1) < std::get<1>(c2)); 
    } 

    /**************************************************************************/
    auto getCandidates(const yarp::os::Bottle& sq_params, yarp::dev::ICartesianControl* iarm) const {
        std::vector<rankable_candidate> candidates;
        if (iarm==nullptr) {
            return std::make_pair(candidates, 0);
        }

        // back up current context
        int backup;
        iarm->storeContext(&backup);

        // increase reaching accuracy
        yarp::os::Bottle options;
        auto& opt1 = options.addList();
        opt1.addString("max_iter"); opt1.addInt(1000);
        auto& opt2 = options.addList();
        opt2.addString("tol"); opt1.addDouble(.0001);
        auto& opt3 = options.addList();
        opt3.addString("constr_tol"); opt2.addDouble(.000001);
        iarm->tweakSet(options);
        yarp::sig::Vector dof;
        iarm->getDOF(dof);
        dof = 1.;
        iarm->setDOF(dof, dof);

        // retrieve SQ parameters
        const auto x = sq_params.get(0).asDouble();
        const auto y = sq_params.get(1).asDouble();
        const auto z = sq_params.get(2).asDouble();
        const auto angle = sq_params.get(3).asDouble() * (M_PI / 180.);
        const auto bx = sq_params.get(4).asDouble();
        const auto by = sq_params.get(5).asDouble();
        const auto bz = sq_params.get(6).asDouble();
        const auto max_b = std::max(bx, std::max(by, bz));

        const yarp::sig::Vector center{x, y, z};
        const std::vector<yarp::sig::Vector> side_points{{bx, 0., 0., 1.}, {0., -by, 0., 1.},
                                                         {-bx, 0., 0., 1.}, {0., by, 0., 1.}};

        // generate side-grasp candidates
        // account for size limitations
        if (bz > hand_half_height + dist_center_index_middle) {
            for (size_t i = 0; i < side_points.size(); i++) {
                // prune by comparing the pre-grasp aperture with the SQ relative size
                if (((i & 0x01) && (2. * bx < .5 * pregrasp_aperture)) ||
                    (!(i & 0x01) && (2. * by < .5 * pregrasp_aperture))) {
                    auto dir = (yarp::math::axis2dcm({0., 0., 1., angle}) * side_points[i]).subVector(0, 2);
                    const auto side = center + dir;
                    dir /= yarp::math::norm(dir);
                    yarp::sig::Vector axis_y{0., 0., -1.};
                    const auto axis_z = (hand == "right" ? -1.: 1.) * dir;
                    const auto axis_x = yarp::math::cross(axis_y, axis_z);
                    const auto candidate = composeCandidate(axis_x, axis_y, axis_z,
                                                            side + axis_y * dist_center_index_middle, dir);
                    auto cost = evaluateCandidate(candidate, iarm);
                    if (cost != std::numeric_limits<double>::infinity()) {
                        // penalize candidate further from COG
                        if (bz != max_b) {
                            cost += 5. / 180.;
                        }
                        candidates.push_back(std::make_tuple(hand, cost, candidate));
                    }
                }
            }
        }

        // generate top-grasp candidates
        const auto top = center + yarp::sig::Vector{0., 0., bz};
        for (size_t i = 0; i < side_points.size(); i++) {
            // prune by comparing the pre-grasp aperture with the SQ relative size
            if (((i & 0x01) && (2. * by < .5 * pregrasp_aperture)) ||
                (!(i & 0x01) && (2. * bx < .5 * pregrasp_aperture))) {
                auto axis_x = (yarp::math::axis2dcm({0., 0., 1., angle}) * side_points[i]).subVector(0, 2);
                axis_x /= yarp::math::norm(axis_x);
                const yarp::sig::Vector axis_z{0., 0., hand == "right" ? -1.: 1.};
                const auto axis_y = yarp::math::cross(axis_z, axis_x);
                const auto candidate = composeCandidate(axis_x, axis_y, axis_z,
                                                        top + axis_y * dist_center_index_middle, {0., 0., 1.});
                // account for size limitations
                if (candidate(2, 3) - fingers_max_length > center[2] - bz) {
                    auto cost = evaluateCandidate(candidate, iarm);
                    if (cost != std::numeric_limits<double>::infinity()) {
                        // penalize candidate further from COG
                        if (bz == max_b) {
                            cost += 5. / 180.;
                        }
                        candidates.push_back(std::make_tuple(hand, cost, candidate));
                    }
                }
            }
        }

        std::sort(candidates.begin(), candidates.end(), compareCandidates);

        // restore backup context
        int context;
        iarm->storeContext(&context);
        iarm->restoreContext(backup);
        iarm->deleteContext(backup);
        return std::make_pair(candidates, context);
    }
};

}

#endif