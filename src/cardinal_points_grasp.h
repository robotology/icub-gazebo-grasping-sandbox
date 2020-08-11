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

typedef std::tuple<std::string, double, yarp::sig::Matrix> ranked_candidates;

/******************************************************************************/
class CardinalPointsGrasp {
    const std::string hand;
    const std::vector<double> pregrasp_fingers_posture;

    double pregrasp_aperture{0.};
    double hand_half_height{0.};
    yarp::sig::Vector approach_direction;
    yarp::sig::Matrix approach;

    std::vector<std::unique_ptr<iCub::iKin::iCubFinger>> fingers;

    /**************************************************************************/
    auto evaluateCandidate(const yarp::sig::Matrix& candidate,
                           yarp::dev::ICartesianControl* iarm) const {
        const auto xd = candidate.getCol(3).subVector(0, 2);
        const auto od = yarp::math::dcm2axis(candidate);

        yarp::sig::Vector xdhat, odhat, qdhat;
        if (iarm->askForPose(xd, od, xdhat, odhat, qdhat)) {
            if (yarp::math::norm(xd - xdhat) < .005) {
                const auto rot = yarp::math::dcm2axis(yarp::math::axis2dcm(od) * yarp::math::axis2dcm(odhat).transposed());
                return std::abs(rot[3] / M_PI);
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
        auto approach_min_distance{0.};
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

        // use little finger to account for safety margin to avoid hitting the table when side-grasping
        hand_half_height = fingers.back()->EndEffPosition()[1] + .01;

        // compute the approach frame wrt the canonical hand-centered frame
        yarp::sig::Vector rotz{0., 1., 0., sector_beg + pregrasp_aperture_angle * (hand == "right" ? -.65 : .65)};
        approach_direction = yarp::math::axis2dcm(rotz) * yarp::sig::Vector{1., 0., 0., 1.};
        approach_direction.pop_back();
        const auto angle = std::acos(yarp::math::dot(approach_direction, {0., 0., hand == "right" ? 1. : -1.}));
        approach = yarp::math::axis2dcm({0., 1., 0., hand == "right" ? -angle : angle});
        const auto approach_origin = -(approach_min_distance + .01) * approach_direction;
        approach.setSubcol(approach_origin, 0, 3);
    }

    /**************************************************************************/
    const auto& getApproachDirection() const {
        return approach_direction;
    }

    /**************************************************************************/
    static auto compareCandidates(const ranked_candidates& c1, const ranked_candidates& c2) { 
        return (std::get<1>(c1) < std::get<1>(c2)); 
    } 

    /**************************************************************************/
    auto getCandidates(const yarp::os::Bottle& sq_params, yarp::dev::ICartesianControl* iarm) const {
        std::vector<ranked_candidates> candidates;
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
        dof = 1;
        iarm->setDOF(dof, dof);

        // retrieve SQ parameters
        const auto x = sq_params.get(0).asDouble();
        const auto y = sq_params.get(1).asDouble();
        const auto z = sq_params.get(2).asDouble();
        const auto angle = sq_params.get(3).asDouble() * (M_PI / 180.);
        const auto bx = sq_params.get(4).asDouble();
        const auto by = sq_params.get(5).asDouble();
        const auto bz = sq_params.get(6).asDouble();

        const yarp::sig::Vector center{x, y, z};

        // generate side-grasp candidates while accounting for size limitations
        if (bz > hand_half_height) {
            std::vector<yarp::sig::Vector> side_points{{bx, 0., 0., 1.}, {0., -by, 0., 1.},
                                                       {-bx, 0., 0., 1.}, {0., by, 0., 1.}};
            for (size_t i = 0; i < side_points.size(); i++) {
                // prune side points comparing the pre-grasp aperture with the SQ relative size
                if (((i & 0x01) && (2. * bx < .5 * pregrasp_aperture)) ||
                    (!(i & 0x01) && (2. * by < .5 * pregrasp_aperture))) {
                    const auto dir = (yarp::math::axis2dcm({0., 0., 1., angle}) * side_points[i]).subVector(0, 2);
                    const auto side = center + dir;
                    yarp::sig::Vector axis_y{0., 0., -1.};
                    const auto axis_z = (hand == "right" ? -1.: 1.) * (dir / yarp::math::norm(dir));
                    const auto axis_x = yarp::math::cross(axis_y, axis_z);

                    // identify grasp frame in the robot root
                    yarp::sig::Matrix candidate = yarp::math::zeros(4, 4);
                    candidate.setSubcol(axis_x, 0, 0);
                    candidate.setSubcol(axis_y, 0, 1);
                    candidate.setSubcol(axis_z, 0, 2);
                    candidate.setSubcol(side, 0, 3);
                    candidate(3, 3) = 1.;
                    candidate = candidate * approach;
                    candidates.push_back(std::make_tuple(hand, evaluateCandidate(candidate, iarm), candidate));
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