/*
 * @file vle/gvle/modeling/recurrence-relation/NameValue.hpp
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


#ifndef VLE_GVLE_MODELING_DIFFERENCEEQUATION_NAMEVALUE_HPP
#define VLE_GVLE_MODELING_DIFFERENCEEQUATION_NAMEVALUE_HPP

#include <vle/vpz/Condition.hpp>
#include <gtkmm/box.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treestore.h>
#include <gtkmm/treeview.h>
#include <gtkmm/builder.h>

namespace vle { namespace gvle { namespace modeling { namespace rr {

class FileNames
{
public:
    FileNames() { }
    virtual ~FileNames() { }

    const std::string& getCodeVarsDeclaration() const
    { return mvarsDeclaration; }

    const std::string& getCodeVarsInitialization() const
    { return mvarsInitialization; }

    const std::string& getCodeEquations () const
    { return mequations; }


    const std::vector<std::string>& getEquationFiles () const
    { return mfiles; }

    void process(const std::string& package);

    Gtk::Widget& build(Glib::RefPtr < Gtk::Builder >& ref);

    void fillField(const std::vector<std::string>& eqFiles);


public:
    void createSetInit(vpz::Condition& condition);
    bool validPort(const vpz::Condition& condition,
                   const std::string& name);

    void onClickCheckButton();

    Gtk::HBox*   m_hbox;
    Gtk::Entry*  meqFiles;
    Gtk::Entry*  m_entryValue;
    //file names
    std::vector<std::string>  mfiles;
    //code
    std::string  mvarsDeclaration;
    std::string  mvarsInitialization;
    std::string  mequations;
    //code structure
    std::vector<std::string>  mvars;

};

}}}} // namespace vle gvle modeling de

#endif
