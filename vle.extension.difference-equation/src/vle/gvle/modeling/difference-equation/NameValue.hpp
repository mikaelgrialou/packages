/*
 * @file vle/gvle/modeling/difference-equation/NameValue.hpp
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
#include <gtkmm/uimanager.h>
#include <gtkmm/menu.h>

namespace vle { namespace gvle { namespace modeling { namespace de {

class NameValue
{
public:
    NameValue() { }
    virtual ~NameValue() { }

    std::string getVariableName() const
	{ return m_entryName->get_text(); }

    Gtk::Widget& build(Glib::RefPtr < Gtk::Builder >& ref);
    void assign(vpz::Condition& condition);
    void deletePorts(vpz::Condition& condition);
    void fillFields(const vpz::Condition& condition);

protected:
    class InitColumns : public Gtk::TreeModelColumnRecord
    {
    public:
	InitColumns() {
	    add(m_col_value);
	}

	Gtk::TreeModelColumn<std::string> m_col_value;
    };

    class InitTreeView : public Gtk::TreeView
    {
    public:
	InitTreeView(BaseObjectType* cobject,
		     const Glib::RefPtr < Gtk::Builder >& refGlade);
	virtual ~InitTreeView();

	const InitColumns& getColumns() const { return m_columnsInit; }

	void init(const vpz::Condition& condition);

    protected:
	virtual bool on_button_press_event(GdkEventButton* ev);

	void onAdd();
	void onRemove();
	void onUp();
	void onDown();

    private:
        Glib::RefPtr <Gtk::UIManager> mPopupUIManager;
        Glib::RefPtr <Gtk::ActionGroup> mPopupActionGroup;
        Gtk::Menu *mMenuPopup;
	InitColumns                  m_columnsInit;
	Glib::RefPtr<Gtk::ListStore> m_refTreeInit;
    };


private:
    void createSetInit(vpz::Condition& condition);
    bool validPort(const vpz::Condition& condition,
                   const std::string& name);

    void onClickCheckButton();

    Gtk::Box*         m_hbox;
    Gtk::Entry*        m_entryName;
    Gtk::Entry*        m_entryValue;
    InitTreeView*      m_initTreeView;
};

}}}} // namespace vle gvle modeling de

#endif
