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


#include <vle/temporal_values/TemporalValues.hpp>
#include <vle/value/Tuple.hpp>
#include <iostream>
#include <sstream>

namespace vle {
namespace temporal_values {

namespace vd = vle::devs;
namespace vu = vle::utils;
namespace vz = vle::vpz;

TemporalValuesProvider::TemporalValuesProvider():
      model_name("unknown"), variables(), generic_vars(0), current_time(0),
      delta(1)
{
}

TemporalValuesProvider::TemporalValuesProvider(const std::string& modelname):
      model_name(modelname), variables(), generic_vars(0), current_time(0),
      delta(1)
{
}

VarUpdate::VarUpdate(const vd::Time& t, double val):
                        timeOfUpdate(t), value(val)
{
}

VarInterface::VarInterface(
        double hsize, bool generic, TemporalValuesProvider* tvpin):
        historySize(hsize), generic(generic), keepFirstValue(true), tvp(tvpin)
{
}


VarMono::VarMono(double hsize, bool generic, TemporalValuesProvider* tvpin):
        VarInterface(hsize, generic, tvpin), history()
{
}

bool VarMono::isMono() const
{
    return true;
}

double* VarMono::getPointer(const vd::Time& t)
{
    VarUpdate& last = history.back();
    if (last.timeOfUpdate < t) {
        history.push_back(VarUpdate(t, last.value));//TODO only piecewise constant
        last = history.back();
    } else if (last.timeOfUpdate > t){
        throw "error"; //TODO
    }
    return &last.value;
}


double VarMono::getVal(const vd::Time& t, double delay) const
{
    //TODO only constant piecewise function



    double reqTime = t+delay;
    History::const_reverse_iterator itb = history.rbegin();
    History::const_reverse_iterator ite = history.rend();
    for (;itb != ite ; itb++){
        if(itb->timeOfUpdate <= reqTime){
            return itb->value;
        }
    }



    throw vu::InternalError(
        vle::fmt("[%1%] getVal not found, called "
                " with t='%2%', delay='%3%' and history ='%4%' \n")
       % tvp->model_name % t % delay % history);
}

void VarMono::update(const std::string& /*varName*/, const vd::Time& t, double val)
{
    VarUpdate& varUpdate = history.back();
    if (varUpdate.timeOfUpdate == t) {
        if (not keepFirstValue) {
            varUpdate.value = val;
        }
    } else {
        history.push_back(VarUpdate(t,val));
        if((t-history.front().timeOfUpdate) > historySize){
            history.pop_front();
        }
    }
}

void VarMono::update(const std::string& varName, const vd::Time& t,
        const vv::Value& val)
{
    this->update(varName, t,val.toDouble().value());
}

void  VarMono::setHistory(const vd::Time& t, const vv::Value& val)
{
    if (val.isTuple()) {
        const vv::Tuple& tuple = val.toTuple();
        for (unsigned int i = 0; i < tuple.size() ; i++) {
            history.push_front(VarUpdate(t-i,tuple.at(i)));
        }
    } else if (val.isDouble()) {
        history.push_front(VarUpdate(t, val.toDouble().value()));
    } else {
        throw vu::ModellingError(
                vle::fmt("[%1%] Initialisation value of state variable"
                        " missing\n") % "humm");
    }
}

vd::Time VarMono::lastUpdateTime() const
{
    return history.back().timeOfUpdate;
}

double VarMono::lastVal(const vd::Time& beg, const vd::Time& end)
{
    History::const_reverse_iterator itb = history.rbegin();
    History::const_reverse_iterator ite = history.rend();

    for (;itb != ite ; itb++){
        if((itb->timeOfUpdate >= beg) and
            (itb->timeOfUpdate < end)){
            return itb->value;
        }
    }
    throw vu::ModellingError(
            vle::fmt("[%1%] lastVal wrong interval [%2%;%3%[ \n")
            % tvp->model_name % beg % end);
}

VarMulti::VarMulti(unsigned int dim, unsigned int historySize,
        bool generic, TemporalValuesProvider* tvpin):
                VarInterface(historySize, generic, tvpin),
                cont(), dim(dim)
{
    for (unsigned int i = 0; i< dim; i++) {
        cont.push_back(VarMono(historySize, generic, tvpin));
    }

}

bool VarMulti::isMono() const
{
    return false;
}

VarMono& VarMulti::get(unsigned int i)
{
    return cont[i];
}

double* VarMulti::getPointer(const vd::Time& t,
        unsigned int i)
{
    return get(i).getPointer(t);
}

double VarMulti::getVal(unsigned int i, const vd::Time& t,
        double delay) const
{
    return cont[i].getVal(t,delay);
}

void VarMulti::update(const std::string& varName,
        const vd::Time& t, const vv::Value& val)
{
    if ( val.isTuple() && val.toTuple().size() == cont.size()) {
        const vv::Tuple& tuple = val.toTuple();
        vv::Tuple::const_iterator itbtuple = tuple.value().begin();
        Container::iterator itb = cont.begin();
        Container::iterator ite = cont.end();
        for(; itb!=ite; itb++, itbtuple++){
            itb->update(varName, t, *itbtuple);
        }
    } else {
        throw vu::ModellingError(
                vle::fmt("[%1%] updating of multi-variable "
                        " requires a tuple of size '%2%' and received '%3%'\n")
        % tvp->model_name % cont.size() % val);
    }
}

void VarMulti::setHistory(const vd::Time& t, const vv::Value& val)
{
    if (val.isTable()) {
        const vv::Table& table = val.toTable();
        for (unsigned int i = 0; i < table.height() ; i++) {
            vv::Tuple tuple;
            for (unsigned int j =0;j<table.width(); j++) {
                tuple.add(table.get(j,i));
            }
            get(i).setHistory(t,tuple);
        }
    } else {
        throw vu::ModellingError(
                vle::fmt("[%1%] Initialisation value "
                        "of state variable missing \n")
        % tvp->model_name );
    }
}

Var::Var() : name(), itVar(0), tvp(0)
{
}

Var::Var(const std::string& n, TemporalValuesProvider* tvpin) :
        name(n), itVar(0), tvp(tvpin)
{
    itVar = dynamic_cast<VarMono*>(tvp->variables.find(name)->second);
}

Var::Var(VarMono* v, TemporalValuesProvider& tvpin) :
                   name(""), itVar(v), tvp(&tvpin)
{
}

double
Var::operator()(double delay) const
{
    return itVar->getVal(tvp->current_time, delay * tvp->delta);
}

double
Var::operator()() const
{
    return itVar->history.back().value;
}

double
Var::lastVal(double delayBeg, double delayEnd) const
{
    return itVar->lastVal(tvp->current_time + delayBeg * tvp->delta,
            tvp->current_time + delayEnd * tvp->delta);
}

double
Var::lastVal() const
{
    return lastVal(-99999999/*TODO*/, 0);
}

void
Var::operator=(double v)
{
    itVar->update(name, tvp->current_time, v);
}

Vect::Vect() : name(), itVar(0), tvp(0)
{
}

Vect::Vect(const std::string& n, TemporalValuesProvider* tvpin) :
        name(n), itVar(0), tvp(tvpin)
{
    itVar = dynamic_cast<VarMulti*>(tvp->variables.find(name)->second);

}

Var
Vect::operator[](unsigned int i)
{
    return(Var(&(itVar->get(i)),*tvp));
}

Var
TemporalValuesProvider::createVar(const std::string& name, double historySize)
{
    variables.insert(std::make_pair(name,
            new VarMono(historySize, false, this)));
    return Var(name,this);
}

Var
TemporalValuesProvider::createVar(const std::string& name)
{
    return createVar(name, 2.0);
}

Vect
TemporalValuesProvider::createVect(
        const std::string& name, unsigned int dim, double historySize)
{
    variables.insert(std::make_pair(name,
            new VarMulti(dim, historySize, false, this)));
    return Vect(name, this);
}

Vect
TemporalValuesProvider::createVect(
        const std::string& name, unsigned int dim)
{
    return createVect(name, dim, 2.0);
}


const std::vector<std::string>&
TemporalValuesProvider::genericVar() const
{
    if (generic_vars == 0) {
        throw vu::ModellingError(vle::fmt(" TVP not configured for generic"
                " variables"));
    }
    return *generic_vars;
}

double
TemporalValuesProvider::genericVarSum(const vd::Time& t)
{
    const std::vector<std::string>& vars = genericVar();
    std::vector<std::string>::const_iterator itb = vars.begin();
    std::vector<std::string>::const_iterator ite = vars.end();
    double sum = 0;

    for (; itb != ite ; itb++) {
        VarMono* v = static_cast<VarMono*> (variables.find(*itb)->second);
        sum += v->getVal(t,0);
    }
    return sum;
}

}} // namespace
