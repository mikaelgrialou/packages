/*
 * Copyright (c) 2014-2014 INRA http://www.inra.fr
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


#include <vle/extension/fsa/Statechart.hpp>
#include <iostream>

namespace vle {

namespace discrete_time {
namespace test {

namespace ve = vle::extension;
namespace vf = vle::extension::fsa;
namespace vd = vle::devs;

enum State { A, B };

class Perturb2 : public vf::Statechart
{
public:
    Perturb2(const vd::DynamicsInit& init, const vd::InitEventList& events) :
        vf::Statechart(init, events)
    {
        states(this) << A << B;
        transition(this, A, B) << after(5.) << send(&Perturb2::out1);
        transition(this, B, A) << after(5.) << send(&Perturb2::out2);

        initialState(A);
    }

    virtual ~Perturb2() { }

    void out1(const vd::Time&  /*time*/, vd::ExternalEventList& output) const
    {
        vd::ExternalEvent* e = new vd::ExternalEvent("out1");
        e->attributes().addString("name","out1");
        output.push_back(e);
        //std::cout << " Perturb2 send out1 " << time << std::endl;
    }

    void out2(const vd::Time& /* time */, vd::ExternalEventList& output) const
    {
        vd::ExternalEvent* e = new vd::ExternalEvent("out2");
        e->attributes().addString("name","out2");
        output.push_back(e);
        //std::cout << " Perturb2 send out2 " << time << std::endl;
    }

};

}}} // namespaces

DECLARE_DYNAMICS(vle::discrete_time::test::Perturb2)
