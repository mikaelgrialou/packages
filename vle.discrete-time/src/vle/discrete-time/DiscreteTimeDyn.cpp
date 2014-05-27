/*
 * @file record/difference-equation/DifferenceEquation.cpp
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


#include <vle/discrete-time/DiscreteTimeDyn.hpp>
#include <vle/value/Tuple.hpp>
#include <iostream>
#include <sstream>

namespace vle {
namespace discrete_time {

namespace vd = vle::devs;
namespace vu = vle::utils;
namespace vz = vle::vpz;
namespace vt = vle::temporal_values;

DiscreteTimeDyn::DEVS_TransitionGuards::DEVS_TransitionGuards():
         has_sync(false), all_synchronized(false), bags_to_eat_eq_0(true),
         bags_eaten_eq_bags_to_eat(true), LWUt_sup_NCt(false),
         LWUt_eq_NCt(false)
{
}

DiscreteTimeDyn::DEVS_Options::DEVS_Options_Var::DEVS_Options_Var(
        unsigned int sync):
                sync(sync),no_sync_error(false), keep_first_value(true),
                history_size(1), init_value(0)
{
}

DiscreteTimeDyn::DEVS_Options::DEVS_Options_Var::~DEVS_Options_Var()
{
    delete init_value;
}

DiscreteTimeDyn::DEVS_Options::DEVS_Options():
        bags_to_eat(0), dt(1.0), generic(false), all_sync(0), var_options()
{
}

DiscreteTimeDyn::DEVS_Options::~DEVS_Options()
{
    DEVS_Options_Var_cont::iterator itb = var_options.begin();
    DEVS_Options_Var_cont::iterator ite = var_options.end();
    for (;itb != ite; itb++) {
        delete itb->second;
    }
}

bool
DiscreteTimeDyn::DEVS_Options::isSync(const std::string& var_name,
        unsigned int currTimeStep)
{
    DEVS_Options_Var_cont::iterator itf = var_options.find(var_name);
    if ((itf == var_options.end()) or (itf->second->sync == 0)) {
        return false;
    }
    return (currTimeStep % itf->second->sync) == 0 ;
}

DiscreteTimeDyn::DEVS_Options::DEVS_Options_Var*
DiscreteTimeDyn::DEVS_Options::getVarOption(
        const std::string& var_name)
{
    DEVS_Options_Var_cont::iterator itf = var_options.find(var_name);
    if (itf != var_options.end()) {
        return itf->second;
    } else {
        return var_options.insert(DEVS_Options_Var_cont::value_type(
                var_name, new DEVS_Options_Var(all_sync))).first->second;
    }
}

DiscreteTimeDyn::DiscreteTimeDyn(const vd::DynamicsInit& model,
    const vd::InitEventList& events) : vd::Dynamics(model, events),
    devs_state(INIT), devs_options(), devs_guards(), devs_internal(),
    declarationOn(true), currentTimeStep(0), tvp(getModelName())
{
    vd::InitEventList::const_iterator itf = events.find("all_sync");
    vd::InitEventList::const_iterator ite = events.end();
    if (itf != ite) {
        devs_options.all_sync = itf->second->toInteger().value();
    }

    vd::InitEventList::const_iterator itb = events.begin();
    std::string prefix;
    std::string var_name;
    DEVS_Options::DEVS_Options_Var* var_opt = 0;
    //first init
    for (; itb != ite; itb++) {
        const std::string& event_name = itb->first;
        if (event_name == "bags_to_eat") {
            devs_options.bags_to_eat = itb->second->toInteger().value();
        } else if (event_name == "time_step") {
            devs_options.dt = itb->second->toDouble().value();
            tvp.delta = devs_options.dt;
        } else if (event_name == "generic") {
            devs_options.generic = itb->second->toBoolean().value();
        } else if (!prefix.assign("sync").empty() and
                !event_name.compare(0, prefix.size(), prefix)) {
            var_name.assign(event_name.substr(prefix.size()+1,
                    event_name.size()));
            var_opt = devs_options.getVarOption(var_name);
            var_opt->sync = itb->second->toInteger().value();
        } else if (!prefix.assign("no_sync_error").empty() and
                !event_name.compare(0, prefix.size(), prefix)) {
            var_name.assign(event_name.substr(prefix.size()+1,
                    event_name.size()));
            var_opt = devs_options.getVarOption(var_name);
            var_opt->no_sync_error = itb->second->toBoolean().value();
        } else if (!prefix.assign("keep_first_value").empty() and
                !event_name.compare(0, prefix.size(), prefix)) {
            var_name.assign(event_name.substr(prefix.size()+1,
                    event_name.size()));
            var_opt = devs_options.getVarOption(var_name);
            var_opt->keep_first_value = itb->second->toBoolean().value();
        } else if (!prefix.assign("history_size").empty() and
                !event_name.compare(0, prefix.size(), prefix)) {
            var_name.assign(event_name.substr(prefix.size()+1,
                    event_name.size()));
            var_opt = devs_options.getVarOption(var_name);
            var_opt->history_size = itb->second->toInteger().value();
        } else if (!prefix.assign("init_value").empty() and
                !event_name.compare(0, prefix.size(), prefix)) {
            var_name.assign(event_name.substr(prefix.size()+1,
                    event_name.size()));
            var_opt = devs_options.getVarOption(var_name);
            var_opt->init_value = itb->second->clone();
        }
    }

    if (devs_options.generic) {
        vz::ConnectionList::const_iterator itb =
                getModel().getInputPortList().begin();
        vz::ConnectionList::const_iterator ite =
                getModel().getInputPortList().end();
        for (; itb != ite; itb++) {
            tvp.variables.insert(std::make_pair(itb->first,
                    new vt::VarMono(2, true, &tvp)));
            if (tvp.generic_vars == 0) {
                tvp.generic_vars = new std::vector<std::string>();
            }
            tvp.generic_vars->push_back(itb->first);
        }
    }
}

DiscreteTimeDyn::~DiscreteTimeDyn()
{
}




void DiscreteTimeDyn::initVarsAndGuards(const vd::Time& t)
{
    devs_guards.bags_to_eat_eq_0 = (devs_options.bags_to_eat == 0);

    vt::Variables::iterator itb = tvp.variables.begin();
    vt::Variables::iterator ite = tvp.variables.end();

    DEVS_Options::DEVS_Options_Var* var_opt = 0;
    vv::Double init_value_default(0);

    for ( ; itb!=ite; itb++) {
        const std::string& var_name = itb->first;
        vt::VarInterface* var = itb->second;
        var_opt = devs_options.getVarOption(var_name);
        if (var_opt->sync == 1) {
            devs_guards.has_sync = true;
        }
        for (unsigned int h=0; h <= var_opt->history_size; h++) {
            if (var_opt->init_value) {
                var->setHistory(t - h*devs_options.dt, *var_opt->init_value);
            } else {
                var->setHistory(t - h*devs_options.dt, init_value_default);
             }
        }
    }
}

void DiscreteTimeDyn::outputVar(const vle::devs::Time& time,
        vle::devs::ExternalEventList& output) const
{
    vt::Variables::const_iterator itb = tvp.variables.begin();
    vt::Variables::const_iterator ite = tvp.variables.end();
    for (; itb!=ite; itb++) {
        const std::string& var_name = itb->first;
        if (getModel().existOutputPort(var_name)) {
            vt::VarInterface* v = itb->second;
            if (v->isMono()) {
                vt::VarMono* vmono = static_cast < vt::VarMono* >(v);
                vd::ExternalEvent* e = new vd::ExternalEvent(var_name);
                e->putAttribute("name", new vv::String(var_name));
                e->putAttribute("value",
                        new vv::Double(vmono->getVal(time,0)));
                output.push_back(e);
            }
        }
    }
}

void DiscreteTimeDyn::updateAllSynchronized(const vle::devs::Time& t)
{
    vt::Variables::iterator itb = tvp.variables.begin();
    vt::Variables::iterator ite = tvp.variables.end();
    bool res = true;
    for (;itb!=ite;itb++) {
        const std::string& var_name = itb->first;
        if (devs_options.isSync(var_name, currentTimeStep+1)) {
            if (itb->second->lastUpdateTime() < t) {
                res = false;
            }
        }
    }
    devs_guards.all_synchronized = res;
}

void DiscreteTimeDyn::varOnSyncError(std::string& v)
{
    vt::Variables::iterator itb = tvp.variables.begin();
    vt::Variables::iterator ite = tvp.variables.end();
    for (;itb!=ite;itb++) {
        const std::string& var_name = itb->first;
        if (devs_options.isSync(var_name, currentTimeStep)) {
            if (itb->second->lastUpdateTime() < devs_internal.NCt) {
                v.assign(var_name);
                return ;
            }
        }
    }
    v.assign("");
}


vd::Time DiscreteTimeDyn::init(const vd::Time& t)
{
    devs_state = INIT;
    processIn(t, INTERNAL);
    return timeAdvance();
}

vd::Time DiscreteTimeDyn::timeAdvance() const
{
    switch (devs_state) {
    case INIT:
        return 0;
        break;
    case WAIT:
        return devs_internal.NCt - devs_internal.LWUt;
        break;
    case WAIT_SYNC:
        return devs_internal.NCt + devs_options.dt/2.0 - devs_internal.LWUt;
        break;
    case WAIT_BAGS:
        return 0;
        break;
    case COMPUTE:
        return 0;
        break;
    }
    return 0;
}


void DiscreteTimeDyn::internalTransition(
    const vd::Time& t)
{
    processOut(t, INTERNAL);
    updateGuards(t, INTERNAL);

    switch (devs_state) {
    case INIT:
        if (devs_guards.has_sync) {
            devs_state = WAIT_SYNC;
        } else {
            devs_state = WAIT;
        }
        break;
    case WAIT:
        if (devs_guards.bags_to_eat_eq_0) {
            devs_state = COMPUTE;
        } else {
            devs_state = WAIT_BAGS;
        }
        break;
    case WAIT_SYNC:
        {
            std::string varError;
            varOnSyncError(varError);
            throw vu::InternalError(
                    vle::fmt("[%1%] Error missing sync: '%2%'\n")
                    % getModelName() % varError);
        }
        break;
    case WAIT_BAGS:
        if (devs_guards.bags_eaten_eq_bags_to_eat) {
            devs_state = COMPUTE;
        } else {
            devs_state = WAIT_BAGS;
        }
        break;
    case COMPUTE:
        if (devs_guards.has_sync) {
            devs_state = WAIT_SYNC;
        } else {
            devs_state = WAIT;
        }
        break;
    }
    processIn(t, INTERNAL);
}

void DiscreteTimeDyn::externalTransition(
    const vd::ExternalEventList& event,
    const vd::Time& t)
{
    processOut(t, EXTERNAL);
    handleExtEvt(t, event);
    updateGuards(t, EXTERNAL);

    switch (devs_state) {
    case INIT:
        throw vu::InternalError("Error DEVS \n");
        break;
    case WAIT:
        devs_state = WAIT;
        break;
    case WAIT_SYNC:
        if (devs_guards.LWUt_sup_NCt) {
            {
                std::string varError;
                varOnSyncError(varError);
                throw vu::InternalError(
                        vle::fmt("[%1%] Error missing sync: '%2%'\n")
                        % getModelName() % varError);
            }
        } else if (devs_guards.LWUt_eq_NCt and devs_guards.all_synchronized) {
            if (devs_guards.bags_to_eat_eq_0) {
                devs_state = COMPUTE;
            } else {
                devs_state = WAIT_BAGS;
            }
        } else {
            devs_state = WAIT_SYNC;
        }
        break;
    case WAIT_BAGS:
        throw vu::InternalError("Error DEVS \n");
        break;
    case COMPUTE:
        throw vu::InternalError("Error DEVS \n");
        break;
    }
    processIn(t, EXTERNAL);
}

void DiscreteTimeDyn::confluentTransitions(
    const vd::Time& t,
    const vd::ExternalEventList& event)
{
    processOut(t, CONFLUENT);
    handleExtEvt(t, event);
    updateGuards(t, CONFLUENT);

    switch (devs_state) {
    case INIT:
        throw vu::InternalError("Error Unhandled error \n");
        break;
    case WAIT:
        if (devs_guards.bags_to_eat_eq_0) {
            devs_state = COMPUTE;
        } else {
            devs_state = WAIT_BAGS;
        }
        break;
    case WAIT_SYNC:
        {
            std::string varError;
            varOnSyncError(varError);
            throw vu::InternalError(
                    vle::fmt("[%1%] Error missing sync: '%2%'\n")
            % getModelName() % varError);
        }
        break;
    case WAIT_BAGS:
        if (devs_guards.bags_eaten_eq_bags_to_eat) {
            devs_state = COMPUTE;
        } else {
            devs_state = WAIT_BAGS;
        }
        break;
    case COMPUTE:
        if (devs_guards.has_sync) {
            devs_state = WAIT_SYNC;
        } else {
            devs_state = WAIT;
        }
        break;
    }
    processIn(t, EXTERNAL);
}


void DiscreteTimeDyn::output(const vle::devs::Time& time,
                        vle::devs::ExternalEventList& output) const
{
    switch (devs_state) {
    case INIT:
        break;
    case WAIT:
        break;
    case WAIT_SYNC:
        break;
    case WAIT_BAGS:
        break;
    case COMPUTE:
        outputVar(time, output);
        break;
    }
}

vle::value::Value* DiscreteTimeDyn::observation(
    const vle::devs::ObservationEvent& event) const
{
    const std::string& port = event.getPortName();
    vt::Variables::const_iterator itf = tvp.variables.find(port);
    if (itf != tvp.variables.end()) {
        vt::VarInterface* v = itf->second;
        if (v->isMono()) {
            vt::VarMono* vmono = static_cast < vt::VarMono* >(v);
            return new vv::Double(vmono->getVal(event.getTime(), 0));
        }
    }

    return 0;
}



void DiscreteTimeDyn::processIn(const vd::Time& t,
        DEVS_TransitionType /*trans*/)
{
    switch (devs_state) {
    case INIT:
        declarationOn = false;
        initVarsAndGuards(t);
        break;
    case WAIT:
        break;
    case WAIT_SYNC:
        break;
    case WAIT_BAGS:
        break;
    case COMPUTE:
        currentTimeStep ++;
        tvp.current_time = t;
        compute(t);
        break;
    }
}

void DiscreteTimeDyn::processOut(const vd::Time& t,
        DEVS_TransitionType /*trans*/)
{
    switch (devs_state) {
    case INIT:
        devs_internal.LWUt = t;
        devs_internal.NCt = t + devs_options.dt;
        break;
    case WAIT:
        devs_internal.LWUt = t;
        devs_internal.bags_eaten = 0;
        break;
    case WAIT_SYNC:
        devs_internal.LWUt = t;
        devs_internal.bags_eaten = 0;
        break;
    case WAIT_BAGS:
        devs_internal.LWUt = t;
        devs_internal.bags_eaten ++;
        break;
    case COMPUTE:
        devs_internal.LWUt = t;
        devs_internal.NCt = t + devs_options.dt;
        break;
    }
}

void DiscreteTimeDyn::updateGuards(const vd::Time& t,
        DEVS_TransitionType /*trans*/)
{
    switch (devs_state) {
    case INIT:
        break;
    case WAIT:
        break;
    case WAIT_SYNC:
        devs_guards.LWUt_eq_NCt = (devs_internal.LWUt == devs_internal.NCt);
        devs_guards.LWUt_sup_NCt = (devs_internal.LWUt > devs_internal.NCt);
        updateAllSynchronized(t);
        break;
    case WAIT_BAGS:
        devs_guards.bags_eaten_eq_bags_to_eat =
                (devs_internal.bags_eaten == devs_options.bags_to_eat);
        break;
    case COMPUTE:
        break;
    }
}

void DiscreteTimeDyn::handleExtEvt(const vd::Time& t,
        const vd::ExternalEventList& ext)
{
    vd::ExternalEventList::const_iterator itb = ext.begin();
    vd::ExternalEventList::const_iterator ite = ext.end();
    for (; itb != ite; itb++) {
        const std::string& portName = (*itb)->getPortName();
        vt::Variables::iterator it = tvp.variables.find(portName);
        if(it == tvp.variables.end()){
            throw vu::InternalError(
                vle::fmt("[%1%] Unrecognised port '%2%' "
                        "which does not match a variable \n")
                       % getModelName() % portName);
        }

        vt::VarInterface* var = it->second;
        if ((*itb)->existAttributeValue("value")) {
            const vv::Value& varValue = (*itb)->getAttributeValue("value");
            var->update(portName, t,varValue);
        }
    }
}

}} // namespace
