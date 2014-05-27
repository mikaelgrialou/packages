/*
 * @file vle/gvle/modeling/recurrence-relation/Simple.hpp
 *
 * This file is part of VLE, a framework for multi-modeling, simulation
 * and analysis of complex dynamical systems
 * http://www.vle-project.org
 *
 * Copyright (c) 2003-2007 Gauthier Quesnel <quesnel@users.sourceforge.net>
 * Copyright (c) 2003-2011 ULCO http://www.univ-littoral.fr
 * Copyright (c) 2007-2011 INRA http://www.inra.fr
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


#ifndef VLE_GVLE_MODELING_DIFFERENCEEQUATION_SIMPLE_HPP
#define VLE_GVLE_MODELING_DIFFERENCEEQUATION_SIMPLE_HPP

#include <vle/gvle/ModelingPlugin.hpp>
#include <vle/gvle/modeling/recurrence-relation/FileNames.hpp>
#include <vle/utils/Template.hpp>
#include <gtkmm/dialog.h>
#include <vle/vpz/AtomicModel.hpp>
#include <vle/utils/Template.hpp>
#include <vle/vpz/Dynamic.hpp>
#include <gtkmm/dialog.h>
#include <vle/vpz/Condition.hpp>
#include <gtkmm/button.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/frame.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/builder.h>


namespace vle { namespace gvle { namespace modeling { namespace rr {

class Simple : public ModelingPlugin
{
public:
    Simple(const std::string& package, const std::string& library,
            const std::string& curr_package);
    virtual ~Simple();

    virtual bool create(vpz::AtomicModel& model,
                        vpz::Dynamic& dynamic,
                        vpz::Conditions& conditions,
                        vpz::Observables& observables,
                        const std::string& classname,
                        const std::string& namespace_);

    virtual bool modify(vpz::AtomicModel& model,
                        vpz::Dynamic& dynamic,
                        vpz::Conditions& conditions,
                        vpz::Observables& observables,
                        const std::string& conf,
                        const std::string& buffer);

    virtual bool start(vpz::Condition& condition);

    virtual bool start(vpz::Condition&, const std::string&)
    { return true; }

    void onSource();
    void backup();
    void generate(vpz::AtomicModel& model,
                  vpz::Dynamic& dynamic,
                  vpz::Conditions& conditions,
                  vpz::Observables& observables,
                  const std::string& classname,
                  const std::string& namespace_,
                  bool generic = false);
    std::string getTemplate() const;
    void parseConf(const std::string& conf,
                   std::string& classname,
                   std::string& namespace_,
                   std::vector<std::string>& eqFiles
//                   Parameters::Parameters_t& parameters,
//                   Parameters::ExternalVariables_t& variables
                   );
    void parseFunctions(const std::string& buffer);


private:

    Glib::RefPtr < Gtk::Builder > mXml;

    std::string                 mOldMode;
    std::vector < std::string > mOldExternalVariables;



    Gtk::Dialog*         m_dialog;
    Gtk::Button*         m_buttonSource;
    FileNames            mFileNames;

    std::list < sigc::connection > mList;

    std::string parseFunction(const std::string& buffer,
                              const std::string& begin,
                              const std::string& end,
                              const std::string& name);
    void build(bool modeling);
    void destroy();
};

}}}} // namespace vle gvle modeling de

#endif
