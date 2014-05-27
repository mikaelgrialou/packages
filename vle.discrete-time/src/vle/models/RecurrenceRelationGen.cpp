/*
 * This file is part of VLE, a framework for multi-modeling, simulation
 * and analysis of complex dynamical systems.
 * http://www.vle-project.org
 *
 * Copyright (c) 2013-2013 INRA http://www.inra.fr
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <record/difference-equation/DifferenceEquation.hpp>
#include <vle/value/Set.hpp>
#include <boost/spirit/include/classic_chset.hpp>
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_confix.hpp>
#include <boost/spirit/include/classic_lists.hpp>
#include <boost/spirit/include/classic_escape_char.hpp>
#include <muParser.h>

namespace vd = vle::devs;
namespace vv = vle::value;
namespace ver = vle::extension::RecurrenceRelation;

namespace vle { namespace models {


class RecurrenceRelationGen : public ver::RecurrenceRelation
{
public:

    typedef std::map<std::string,Var> VarsContainer;
    typedef std::map<std::string,Vect> VectsContainer;

    RecurrenceRelationGen(const vd::DynamicsInit& init,
            const vd::InitEventList& events)
        : ver::RecurrenceRelation(init, events)
    {

        vv::Map::const_iterator itb = initVariables->begin();
        vv::Map::const_iterator ite = initVariables->end();
        for(;itb!=ite;itb++){
            if(itb->second->isTuple()) {
                vars[itb->first] = createVar(itb->first,
                        itb->second->toTuple().size());
            } else if(itb->second->isTable()) {
                vects[itb->first] = createVect(itb->first,
                        itb->second->toTable().height(),
                        itb->second->toTable().width());
            }
        }
        equations = dynamic_cast<vv::Set*>(events.getSet("equations").clone());
    }

    vd::Time init(const devs::Time& t)
    {
        using namespace boost::spirit::classic;
        vv::Set::const_iterator itb = equations->begin();
        vv::Set::const_iterator ite = equations->end();
        for(;itb!=ite;itb++){
//            std::string toAffect;
//            std::string expr;
//            rule<> grammar;
//            grammar = parse((*itb)->toString().value().c_str(), grammar);

            mu::Parser p;
            VarsContainer::iterator ivb = vars.begin();
            VarsContainer::iterator ive = vars.end();
            for (;ivb!=ive;ivb++) {
                p.DefineVar(ivb->first, ivb->second.itVar->getPointer(t));
            }

            VectsContainer::iterator iab = vects.begin();
            VectsContainer::iterator iae = vects.end();
            for (;iab!=iae;iab++) {
                Vect& v = iab->second;
                for (unsigned int i=0; i<v.itVar->dim; i++) {
                    std::stringstream ss;
                    ss << ivb->first << "_" << i ;
                    p.DefineVar(ss.str(), iab->second.itVar->getPointer(t, i));
                }
            }
            parsers.push_back(p);
        }
        return ver::RecurrenceRelation::init(t);
    }


    virtual ~RecurrenceRelationGen()
    {

    }

    void compute(const vd::Time& t)
    {

    }



    VarsContainer vars;
    VectsContainer vects;
    vv::Set* equations;
    std::vector<mu::Parser> parsers;
};

}} // namespaces

DECLARE_DYNAMICS(vle::models::RecurrenceRelationGen)


