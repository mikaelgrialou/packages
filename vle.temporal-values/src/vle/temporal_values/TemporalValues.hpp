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


#ifndef VLE_TEMPORAL_VALUES_HPP
#define VLE_TEMPORAL_VALUES_HPP 1

#include <vle/devs/Dynamics.hpp>
#include <deque>
#include <vector>

namespace vle {
namespace temporal_values {

namespace vd = vle::devs;
namespace vv = vle::value;

class TemporalValuesProvider;

struct VarUpdate
{
    VarUpdate(const vd::Time& t, double val);
    vd::Time timeOfUpdate;
    double value;
};

struct VarInterface
{
    double historySize;
    bool generic;
    bool keepFirstValue;
    TemporalValuesProvider* tvp;

    VarInterface(double hsize, bool generic, TemporalValuesProvider* eq);
    virtual ~VarInterface() {}

    virtual bool isMono() const = 0;
    virtual void update(const std::string& varName,
            const vd::Time& t, const vv::Value& val) = 0;
    virtual void setHistory(const vd::Time& t, const vv::Value& val) = 0;
    virtual vd::Time lastUpdateTime() const = 0;
};

struct VarMono : public VarInterface
{

    typedef typename std::deque<VarUpdate> History;

    History history;

    VarMono(double hsize, bool generic, TemporalValuesProvider* eq);
    virtual ~VarMono() {}
    bool isMono() const;
    double getVal(const vd::Time& t, double delay) const;
    double* getPointer(const vd::Time& t);
    void update(const std::string& varName, const vd::Time& t, double val);
    void update(const std::string& varName, const vd::Time& t,
            const vv::Value& val);
    void setHistory(const vd::Time& t, const vv::Value& val);
    vd::Time lastUpdateTime() const;
    double lastVal(const vd::Time& beg, const vd::Time& end);
};

struct VarMulti : public VarInterface
{
    typedef typename std::vector<VarMono> Container;

    Container cont;
    unsigned int dim;

    VarMulti(unsigned int dim, unsigned int historySize,
            bool generic, TemporalValuesProvider* eq);

    bool isMono() const;

    VarMono& get(unsigned int i);

    double getVal(unsigned int i, const vd::Time& t,
            double delay) const;

    double* getPointer(const vd::Time& t, unsigned int i);

    void update(const std::string& varName,
            const vd::Time& /*t*/, const vv::Value& /*val*/);

    void setHistory(const vd::Time& t, const vv::Value& val);

    vd::Time lastUpdateTime() const {return 9999;/*TODO*/}
};


//struct VarValue : public VarInterface
//{
//
//};

typedef std::map<std::string, VarInterface*> Variables;




////////////////////
//User interface
////////////////////

struct Var
{
    std::string name;
    VarMono* itVar;
    TemporalValuesProvider* tvp;

    Var();

    Var(const std::string& n, TemporalValuesProvider* tvp);

    Var(VarMono* v, TemporalValuesProvider& tvp);

    double operator()(double delay) const;

    double operator()() const;

    double lastVal(double delayBeg, double delayEnd) const;

    double lastVal() const;

    void operator=(double v);

};

struct Vect
{
    std::string name;
    VarMulti* itVar;
    TemporalValuesProvider* tvp;

    Vect();

    Vect(const std::string& n, TemporalValuesProvider* atomic);

    Var operator[](unsigned int i);

};

class TemporalValuesProvider
{
public:
     TemporalValuesProvider();
     TemporalValuesProvider(const std::string& model_name);
     virtual ~TemporalValuesProvider(){};
     Var createVar(const std::string& name, double historySize);
     Var createVar(const std::string& name);
     Vect createVect(const std::string& name, unsigned int dim,
                double historySize);
     Vect createVect(const std::string& name, unsigned int dim);
     const std::vector<std::string>& genericVar() const;
     double genericVarSum(const vd::Time& t);

     std::string model_name;
     Variables variables;
     std::vector<std::string>* generic_vars;
     vd::Time current_time;
     double delta;

};

inline std::ostream&
operator<<(std::ostream& o, const VarMono::History& hs)
{
    VarMono::History::const_iterator itb = hs.begin();
    VarMono::History::const_iterator ite = hs.end();
    for (;itb != ite; itb++) {
        o <<" [" << itb->timeOfUpdate <<" : " << itb->value <<"], ";
    }
    return o;
}

inline std::ostream&
operator<<(std::ostream& o, const VarInterface& v)
{
    if (v.isMono()) {
        const VarMono& mono = static_cast<const VarMono&>(v);
        o << mono.history ;
    } else {
        const VarMulti& multi = static_cast<const VarMulti&>(v);
        o << " :: " ;
        for (unsigned int i =0 ; i < multi.cont.size() ; i++) {
            const VarMono& mono = multi.cont[i];
            o << mono <<"; ";
        }
    }
    return o;
}

}} // namespaces

#endif
