/*
 *
 * This file is part of VLE, a framework for multi-modeling, simulation
 * and analysis of complex dynamical systems
 * http://www.vle-project.org
 *
 * Copyright (c) 2013-2013 INRA http://www.inra.fr
 *
 * See the AUTHORS or Authors.txt file for copyright owners and contributors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef VLE_DISCRETE_TIME_DYN_HPP
#define VLE_DISCRETE_TIME_DYN_HPP 1

#include <vle/discrete-time/DllDefines.hpp>
#include <vle/temporal_values/TemporalValues.hpp>
#include <vle/devs/Dynamics.hpp>
#include <deque>
#include <vector>

#include <iostream>

namespace vle {
namespace discrete_time {

namespace vd = vle::devs;
namespace vv = vle::value;

class VLE_EXPORT DiscreteTimeDyn : public vd::Dynamics
{
public:



    DiscreteTimeDyn(const vle::devs::DynamicsInit& init,
            const vle::devs::InitEventList&  events);

    virtual ~DiscreteTimeDyn();


    virtual void compute(const vd::Time& t) = 0;

    /*  - - - - - - - - - - - - - --ooOoo-- - - - - - - - - - - -  */

    virtual vd::Time init(const vd::Time& time);

    virtual vd::Time timeAdvance() const;

    virtual void internalTransition(
        const vd::Time& time);

    virtual void externalTransition(
        const vd::ExternalEventList& event,
        const vd::Time& time);

    virtual void confluentTransitions(
        const vd::Time& internal,
        const vd::ExternalEventList& extEventlist);

    virtual void output(const vle::devs::Time& /* time */,
                        vle::devs::ExternalEventList& /* output */) const;

    virtual vle::value::Value* observation(
    const vle::devs::ObservationEvent& /* event */) const;

    /*  - - - - - - - - - - - - - --ooOoo-- - - - - - - - - - - -  */
    ////////////////////
    //Internal
    ////////////////////


    /**
     * @brief DEVS state of the dynamic
     */
    enum DEVS_State
    {
        INIT,
        WAIT,
        WAIT_SYNC,
        WAIT_BAGS,
        COMPUTE
    };

    /**
     * @brief Types of DEVS transitions
     */
    enum DEVS_TransitionType
    {
        INTERNAL, EXTERNAL, CONFLUENT
    };

    /**
     * @brief Guards structure for transition
     * between DEVS states
     */
    struct DEVS_TransitionGuards
    {
        bool has_sync;
        bool all_synchronized;
        bool bags_to_eat_eq_0;
        bool bags_eaten_eq_bags_to_eat;
        bool LWUt_sup_NCt;
        bool LWUt_eq_NCt;
        DEVS_TransitionGuards();
    };

    /**
     * @brief Options structure for
     * DifferenceEquation models
     */
    struct VLE_EXPORT DEVS_Options
    {
        struct DEVS_Options_Var
        {
            unsigned int sync;
            bool no_sync_error;
            bool keep_first_value;
            unsigned int history_size;
            vv::Value* init_value;
            DEVS_Options_Var(unsigned int sync);
            ~DEVS_Options_Var();
        };
        typedef std::map < std::string, DEVS_Options_Var*>
                    DEVS_Options_Var_cont ;

        unsigned int bags_to_eat;
        double dt;
        bool generic;
        unsigned int all_sync;
        DEVS_Options_Var_cont var_options;

        DEVS_Options();
        ~DEVS_Options();

        bool isSync(const std::string& var_name, unsigned int currTimeStep);
        DEVS_Options_Var* getVarOption(const std::string& var_name);
    };

    /**
     * @brief Internal State
     */
    struct VLE_EXPORT DEVS_Internal
    {
        unsigned bags_eaten;
        double NCt; //next compute time
        double LWUt; //last wake up time
        DEVS_Internal() : bags_eaten(0), NCt(0), LWUt(0)
        {
        }
    };

    /**
     * @brief Process method used for DEVS state entrance
     * @param t, the current time
     * @param trans, the type of the transition
     */
    void processIn(const vd::Time& t, DEVS_TransitionType trans);

    /**
     * @brief Process method used for DEVS state exit
     * @param t, the current time
     * @param trans, the type of the transition
     */
    void processOut(const vd::Time& t, DEVS_TransitionType trans);

    /**
     * @brief Update guards for internal transition
     * @param t, the current time
     * @param trans, the type of the transition
     */
    void updateGuards(const vd::Time& t, DEVS_TransitionType trans);

    /**
     * @brief Handles external even
     * @param t, the current time
     * @param ext, the list of external event
     */
    void handleExtEvt(const vd::Time& t, const vd::ExternalEventList& ext);


    /*  - - - - - - - - - - - - - --ooOoo-- - - - - - - - - - - -  */
    ////////////////////
    //Internal
    ////////////////////
    void initVarsAndGuards(const vd::Time& t);
    void outputVar(const vle::devs::Time& time,
            vle::devs::ExternalEventList& output) const;
    void updateAllSynchronized(const vle::devs::Time& t);
    void varOnSyncError(std::string& v);


    DEVS_State devs_state;
    DEVS_Options devs_options;
    DEVS_TransitionGuards devs_guards;
    DEVS_Internal devs_internal;

    bool declarationOn;
    unsigned int currentTimeStep;
    vle::temporal_values::TemporalValuesProvider tvp;
};


inline std::ostream&
operator<<(std::ostream& o, const DiscreteTimeDyn::DEVS_TransitionGuards& g)
{
    o << " g.LWUt_eq_NCt:" << g.LWUt_eq_NCt << "\n";
    o << " g.LWUt_sup_NCt:" << g.LWUt_sup_NCt << "\n";
    o << " g.all_synchronized:" << g.all_synchronized << "\n";
    o << " g.bags_eaten_eq_bags_to_eat:" << g.bags_eaten_eq_bags_to_eat << "\n";
    o << " g.bags_to_eat_eq_0:" << g.bags_to_eat_eq_0 << "\n";
    o << " g.has_sync:" << g.has_sync << "\n";
    return o;
}


inline std::ostream&
operator<<(std::ostream& o, const DiscreteTimeDyn::DEVS_State& s)
{
    switch(s) {
    case DiscreteTimeDyn::INIT :
        o << "INIT" ;
        break;
    case DiscreteTimeDyn::WAIT :
        o << "WAIT" ;
        break;
    case DiscreteTimeDyn::WAIT_SYNC :
        o << "WAIT_SYNC" ;
        break;
    case DiscreteTimeDyn::WAIT_BAGS :
        o << "WAIT_BAGS" ;
        break;
    case DiscreteTimeDyn::COMPUTE :
        o << "COMPUTE" ;
        break;
    }
    return o;
}

}} // namespaces

#endif
