/*
 * @file vle/gvle/modeling/difference-equation/Parameters.cpp
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


#include <vle/gvle/modeling/difference-equation/Parameters.hpp>
#include <boost/lexical_cast.hpp>
#include <vle/value/Double.hpp>
#include <vle/utils/Tools.hpp>
#include <vle/gvle/Message.hpp>
#include <gtkmm/frame.h>
#include <gtkmm/menu.h>
#include <iostream>

namespace vu = vle::utils;

namespace vle { namespace gvle { namespace modeling { namespace de {

Parameters::ParametersTreeView::ParametersTreeView(
    BaseObjectType* cobject,
    const Glib::RefPtr < Gtk::Builder >& xml):
    Gtk::TreeView(cobject)
{
    xml->get_widget("DialogParameterBox", m_dialog);
    xml->get_widget("ParameterNameEntry", m_name);
    xml->get_widget("ParameterValueEntry", m_value);

    m_dialog->set_title("Parameter");

    m_refTreeParameter = Gtk::ListStore::create(m_columnsParameter);
    set_model(m_refTreeParameter);
    int name = append_column(_("Name"), m_columnsParameter.m_col_name);
    append_column(_("Value"), m_columnsParameter.m_col_value);

    Gtk::TreeViewColumn* columnName = get_column(name - 1);
    columnName->set_min_width(110);
    columnName->set_fixed_width(110);

    m_refTreeSelection = get_selection();
    m_refTreeSelection->signal_changed().connect(
        sigc::mem_fun(*this,
	&Parameters::ParametersTreeView::onSelect));

    mPopupActionGroup = Gtk::ActionGroup::create("ParametersTreeView");
    mPopupActionGroup->add(Gtk::Action::create("PTV_ContextMenu", _("Context Menu")));
    mPopupActionGroup->add(
        Gtk::Action::create("PTV_ContextAdd", _("Add")),
        sigc::mem_fun(*this, &Parameters::ParametersTreeView::onAdd));
    mPopupActionGroup->add(
        Gtk::Action::create("PTV_ContextRemove", _("Remove")),
        sigc::mem_fun(*this, &Parameters::ParametersTreeView::onRemove));
    mPopupActionGroup->add(
        Gtk::Action::create("PTV_ContextEdit", _("Edit")),
        sigc::mem_fun(*this, &Parameters::ParametersTreeView::onEdit));

    mPopupUIManager = Gtk::UIManager::create();
    mPopupUIManager->insert_action_group(mPopupActionGroup);

    Glib::ustring ui_info =
        "<ui>"
        "  <popup name='PTView_Popup'>"
        "      <menuitem action='PTV_ContextAdd'/>"
        "      <menuitem action='PTV_ContextRemove'/>"
        "      <menuitem action='PTV_ContextEdit'/>"
        "  </popup>"
        "</ui>";

    try {
        mPopupUIManager->add_ui_from_string(ui_info);
        mMenuPopup = (Gtk::Menu *) (
            mPopupUIManager->get_widget("/PTView_Popup"));
    } catch(const Glib::Error& ex) {
        std::cerr << "building menus failed: PTView_Popup " <<  ex.what();
    }

    if (!mMenuPopup)
        std::cerr << "menu not found : PTView_Popup\n";
}

Parameters::ParametersTreeView::~ParametersTreeView()
{
}

void Parameters::ParametersTreeView::init(const Parameters_t& parameters)
{
    std::vector < std::string>::const_iterator it = parameters.mNames.begin();
    while (it != parameters.mNames.end()) {
        Gtk::TreeModel::Row row = *(m_refTreeParameter->append());
        std::string name = *it;
        std::map < std::string, double >::const_iterator it2 =
            parameters.mValues.find(name);

        row[m_columnsParameter.m_col_name] = name;
        if (it2 != parameters.mValues.end()) {
            row[m_columnsParameter.m_col_value] =
                vu::toScientificString(it2->second, true);
        }
        ++it;
    }
}

void Parameters::ParametersTreeView::init(const vpz::Condition& condition,
                                          const Parameters_t& parameters)
{
    std::vector < std::string>::const_iterator it = parameters.mNames.begin();
    while (it != parameters.mNames.end()) {
        Gtk::TreeModel::Row row = *(m_refTreeParameter->append());
        std::string name = *it;

        row[m_columnsParameter.m_col_name] = name;

        row[m_columnsParameter.m_col_value] =
            vu::toScientificString(condition.firstValue(*it).
                                   toDouble().value(), true);
        ++it;
    }
}

Parameters::Parameters_t Parameters::ParametersTreeView::getParameters() const
{
    Parameters_t parameters;
    Gtk::TreeNodeChildren child = get_model()->children();
    Gtk::TreeModel::iterator it = child.begin();

    while (it != child.end()) {
        std::string name(it->get_value(m_columnsParameter.m_col_name));
        std::string val = it->get_value(m_columnsParameter.m_col_value);

	parameters.mNames.push_back(name);
        if (not val.empty()) {
            try {
                double value = vu::convert < double >(val, true);
                parameters.mValues[name] = value;
            } catch(const std::exception& e) {
                Error(e.what());
            }
        }
	++it;
    }
    return parameters;
}

bool Parameters::ParametersTreeView::on_button_press_event(
    GdkEventButton* event)
{
    bool return_value = Gtk::TreeView::on_button_press_event(event);
    if ( (event->type == GDK_BUTTON_PRESS) && (event->button == 3) ) {
        mMenuPopup->popup(event->button, event->time);
    }

    return return_value;
}

void Parameters::ParametersTreeView::onSelect()
{
    Gtk::TreeModel::iterator it = m_refTreeSelection->get_selected();

    if (it) {
	Gtk::TreeModel::Row row = *it;
	std::string name = row.get_value(m_columnsParameter.m_col_name);
    }
}

void Parameters::ParametersTreeView::onAdd()
{
    m_name->set_text("");
    m_value->set_text("");
    m_dialog->set_focus(*m_name);
    if (m_dialog->run() == Gtk::RESPONSE_ACCEPT) {
	if (not m_name->get_text().empty()) {
	    try {
		std::string name = m_name->get_text();
		if (not exist(m_name->get_text())) {
		    Gtk::TreeModel::Row row = *(
			m_refTreeParameter->append());
		    row[m_columnsParameter.m_col_name] = name;
		    if (not m_value->get_text().empty()) {
                        double value = vu::convert < double >(
                            m_value->get_text(),
                            true);
			row[m_columnsParameter.m_col_value] =
                            vu::toScientificString(value, true);
                    }
		}
            } catch(const boost::bad_lexical_cast& e ) {}
        }
    }
    m_dialog->hide();
}

void Parameters::ParametersTreeView::onRemove()
{
    Glib::RefPtr<Gtk::TreeView::Selection> refSelection = get_selection();

    if (refSelection) {
	Gtk::TreeModel::iterator iter = refSelection->get_selected();

	if (iter) {
	    Gtk::TreeRow row = *iter;
	    Glib::ustring name = row.get_value(m_columnsParameter.m_col_name);
            m_refTreeParameter->erase(iter);
	}
    }
}

void Parameters::ParametersTreeView::onEdit()
{
    Glib::RefPtr<Gtk::TreeView::Selection> refSelection = get_selection();

    if (refSelection) {
	Gtk::TreeModel::iterator iter = refSelection->get_selected();
	if (iter) {
	    Gtk::TreeRow row = *iter;
	    Glib::ustring oldName = row.get_value(
		m_columnsParameter.m_col_name);
	    Glib::ustring oldValue = row.get_value(
		m_columnsParameter.m_col_value);

	    m_name->set_text(oldName);
	    m_value->set_text(oldValue);
            m_dialog->set_focus(*m_name);
	    if (m_dialog->run() == Gtk::RESPONSE_ACCEPT) {
		try {
		    std::string name = m_name->get_text();

		    if (not exist(m_name->get_text())) {
			row[m_columnsParameter.m_col_name] = name;
		    }
		    if (not m_value->get_text().empty()) {

                        double value = vu::convert < double >(
                            m_value->get_text(),
                            true);
			row[m_columnsParameter.m_col_value] =
                            vu::toScientificString(value, true);
		    } else
			row[m_columnsParameter.m_col_value] = "";
		} catch(const boost::bad_lexical_cast& e ) {}
	    }
	}
    }
    m_dialog->hide();
}

bool Parameters::ParametersTreeView::exist(const std::string& name)
{
    Gtk::TreeNodeChildren child = get_model()->children();
    Gtk::TreeModel::iterator iter = child.begin();
    while (iter != child.end()) {
	if (iter->get_value(m_columnsParameter.m_col_name) == name)
	  return true;
	++iter;
    }
    return false;
}

/*  - - - - - - - - - - - - - --ooOoo-- - - - - - - - - - - -  */

Parameters::ExternalVariablesTreeView::ExternalVariablesTreeView(
    BaseObjectType* cobject,
    const Glib::RefPtr < Gtk::Builder >& xml):
    Gtk::TreeView(cobject)
{
    xml->get_widget("DialogExternalVariableBox", m_dialog);
    xml->get_widget("ExternalVariableNameEntry", m_name);

    m_dialog->set_title("External variable");

    m_refTreeVariable = Gtk::ListStore::create(m_columnsVariable);
    set_model(m_refTreeVariable);
    append_column_editable(_("Sync"), m_columnsVariable.m_col_sync);
    int name = append_column(_("Name"), m_columnsVariable.m_col_name);

    Gtk::TreeViewColumn* columnName = get_column(name - 1);
    columnName->set_min_width(110);
    columnName->set_fixed_width(110);

    m_refTreeSelection = get_selection();
    m_refTreeSelection->signal_changed().connect(
        sigc::mem_fun(*this,
	&Parameters::ExternalVariablesTreeView::onSelect));

    mPopupActionGroup = Gtk::ActionGroup::create("ExternalVariablesTreeView");
    mPopupActionGroup->add(Gtk::Action::create("EVTV_ContextMenu", _("Context Menu")));

    mPopupActionGroup->add(Gtk::Action::create("EVTV_ContextAdd", _("_Add")),
        sigc::mem_fun(*this, &Parameters::ExternalVariablesTreeView::onAdd));

    mPopupActionGroup->add(Gtk::Action::create("EVTV_ContextRemove", _("_Remove")),
        sigc::mem_fun(*this, &Parameters::ExternalVariablesTreeView::onRemove));

    mPopupActionGroup->add(Gtk::Action::create("EVTV_ContextRename", _("_Edit")),
        sigc::mem_fun(*this, &Parameters::ExternalVariablesTreeView::onEdit));

    mPopupUIManager = Gtk::UIManager::create();
    mPopupUIManager->insert_action_group(mPopupActionGroup);

    Glib::ustring ui_info =
        "<ui>"
        "  <popup name='EVTView_Popup'>"
        "      <menuitem action='EVTV_ContextAdd'/>"
        "      <menuitem action='EVTV_ContextRemove'/>"
        "      <menuitem action='EVTV_ContextRename'/>"
        "  </popup>"
        "</ui>";

    try {
        mPopupUIManager->add_ui_from_string(ui_info);
        mMenuPopup = (Gtk::Menu *) (
            mPopupUIManager->get_widget("/EVTView_Popup"));
    } catch(const Glib::Error& ex) {
        std::cerr << "building menus failed: EVTView_Popup " <<  ex.what();
    }

    if (!mMenuPopup)
        std::cerr << "menu not found : EVTView_Popup\n";


}

Parameters::ExternalVariablesTreeView::~ExternalVariablesTreeView()
{
}

void Parameters::ExternalVariablesTreeView::init(
    const ExternalVariables_t& variables)
{
    ExternalVariables_t::const_iterator it = variables.begin();
    while (it != variables.end()) {
        Gtk::TreeModel::Row row = *(m_refTreeVariable->append());

        row[m_columnsVariable.m_col_name] = it->first;
        row[m_columnsVariable.m_col_sync] = it->second;
        ++it;
    }
}

Parameters::ExternalVariables_t
Parameters::ExternalVariablesTreeView::getExternalVariables() const
{
    ExternalVariables_t variables;
    Gtk::TreeNodeChildren child = get_model()->children();
    Gtk::TreeModel::iterator it = child.begin();

    while (it != child.end()) {
        std::string name(it->get_value(m_columnsVariable.m_col_name));
        bool sync = it->get_value(m_columnsVariable.m_col_sync);

	variables.push_back(make_pair(name, sync));
	++it;
    }
    return variables;
}

bool Parameters::ExternalVariablesTreeView::on_button_press_event(
    GdkEventButton* event)
{
    bool return_value = Gtk::TreeView::on_button_press_event(event);
    if ( (event->type == GDK_BUTTON_PRESS) && (event->button == 3) ) {
        mMenuPopup->popup(event->button, event->time);
    }

    return return_value;
}

void Parameters::ExternalVariablesTreeView::onSelect()
{
    Gtk::TreeModel::iterator it = m_refTreeSelection->get_selected();

    if (it) {
	Gtk::TreeModel::Row row = *it;
	std::string name = row.get_value(m_columnsVariable.m_col_name);
    }
}

void Parameters::ExternalVariablesTreeView::onAdd()
{
    m_name->set_text("");
    if (m_dialog->run() == Gtk::RESPONSE_ACCEPT) {
	if (not m_name->get_text().empty()) {
	    try {
		std::string name = m_name->get_text();
		if (not exist(m_name->get_text())) {
		    Gtk::TreeModel::Row row = *(
			m_refTreeVariable->append());
		    row[m_columnsVariable.m_col_name] = name;
		    row[m_columnsVariable.m_col_sync] = true;
		}
	    } catch(const boost::bad_lexical_cast& e ) {}
	}
    }
    m_dialog->hide();
}

void Parameters::ExternalVariablesTreeView::onRemove()
{
    Glib::RefPtr<Gtk::TreeView::Selection> refSelection = get_selection();

    if (refSelection) {
	Gtk::TreeModel::iterator iter = refSelection->get_selected();

	if (iter) {
	    Gtk::TreeRow row = *iter;
	    Glib::ustring name = row.get_value(m_columnsVariable.m_col_name);
            m_refTreeVariable->erase(iter);
	}
    }
}

void Parameters::ExternalVariablesTreeView::onEdit()
{
    Glib::RefPtr<Gtk::TreeView::Selection> refSelection = get_selection();

    if (refSelection) {
	Gtk::TreeModel::iterator iter = refSelection->get_selected();
	if (iter) {
	    Gtk::TreeRow row = *iter;
	    Glib::ustring oldName = row.get_value(
		m_columnsVariable.m_col_name);
	    m_name->set_text(oldName);
	    if (m_dialog->run() == Gtk::RESPONSE_ACCEPT) {
                std::string name = m_name->get_text();
                if (not exist(m_name->get_text())) {
                    row[m_columnsVariable.m_col_name] = name;
                }
	    }
	}
    }
    m_dialog->hide();
}

bool Parameters::ExternalVariablesTreeView::exist(const std::string& name)
{
    Gtk::TreeNodeChildren child = get_model()->children();
    Gtk::TreeModel::iterator iter = child.begin();
    while (iter != child.end()) {
	if (iter->get_value(m_columnsVariable.m_col_name) == name)
	  return true;
	++iter;
    }
    return false;
}

/*  - - - - - - - - - - - - - --ooOoo-- - - - - - - - - - - -  */

void Parameters::assign(vpz::Condition& condition)
{
    Parameters::Parameters_t parameters = m_parametersTreeView->getParameters();

    for (std::vector < std::string >::const_iterator it =
             parameters.mNames.begin(); it != parameters.mNames.end(); ++it) {
        std::string name(*it);
        std::map < std::string, double >::const_iterator it2 =
            parameters.mValues.find(name);

        if (it2 != parameters.mValues.end()) {
            condition.add(name);
            condition.addValueToPort(name, value::Double::create(it2->second));
        }
    }
}

Gtk::Widget& Parameters::build(Glib::RefPtr < Gtk::Builder >& ref)
{
    Gtk::Frame* parametersFrame;
    Gtk::Frame* externalVariablesFrame;

    m_hbox = Gtk::manage(new Gtk::HBox);
    ref->get_widget_derived("ParametersTreeView",
			    m_parametersTreeView);
    ref->get_widget("ParametersFrame", parametersFrame);
    ref->get_widget_derived("ExternalVariablesTreeView",
			    m_externalVariablesTreeView);
    ref->get_widget("ExternalVariablesFrame", externalVariablesFrame);
    m_hbox->set_spacing(10);
    m_hbox->add(*parametersFrame);
    m_hbox->add(*externalVariablesFrame);
    m_hbox->show_all();
    return *m_hbox;
}

Gtk::Widget& Parameters::buildParameters(Glib::RefPtr < Gtk::Builder >& ref)
{
    Gtk::Frame* parametersFrame;

    ref->get_widget_derived("ParametersTreeView",
			    m_parametersTreeView);
    ref->get_widget("ParametersFrame", parametersFrame);
    return *parametersFrame;
}

void Parameters::deletePorts(vpz::Condition& condition)
{
    for (std::vector < std::string >::const_iterator it =
             m_oldParameters.begin(); it != m_oldParameters.end(); ++it) {
        condition.del(*it);
    }
}

void Parameters::fillFields(const Parameters_t& parameters)
{
    m_oldParameters.clear();
    for (std::vector < std::string >::const_iterator it =
             parameters.mNames.begin(); it != parameters.mNames.end(); ++it) {
        m_oldParameters.push_back(*it);
    }

    m_parametersTreeView->init(parameters);
}

void Parameters::fillFields(const vpz::Condition& condition,
                            const Parameters_t& parameters)
{
    m_oldParameters.clear();
    for (std::vector < std::string >::const_iterator it =
             parameters.mNames.begin(); it != parameters.mNames.end(); ++it) {
        m_oldParameters.push_back(*it);
    }

    m_parametersTreeView->init(condition, parameters);
}

void Parameters::fillFields(const Parameters_t& parameters,
			    const ExternalVariables_t& variables)
{
    fillFields(parameters);
    if (m_externalVariablesTreeView) {
        m_externalVariablesTreeView->init(variables);
    }
}

void Parameters::fillFields(const vpz::Condition& condition,
                            const Parameters_t& parameters,
			    const ExternalVariables_t& variables)
{
    fillFields(condition, parameters);
    if (m_externalVariablesTreeView) {
        m_externalVariablesTreeView->init(variables);
    }
}
}}}} // vle gvle modeling de
