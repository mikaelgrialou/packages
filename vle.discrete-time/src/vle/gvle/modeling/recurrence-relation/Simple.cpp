/*
 * @file vle/gvle/modeling/recurrence-relation/Simple.cpp
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


#include <vle/gvle/modeling/recurrence-relation/Simple.hpp>
#include <vle/utils/Package.hpp>
#include <vle/utils/Tools.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/lexical_cast.hpp>


namespace vle {
namespace gvle {
namespace modeling {
namespace rr {

Simple::Simple(const std::string& package, const std::string& library,
        const std::string& curr_package)
    : ModelingPlugin(package, library, curr_package), m_dialog(0),
      m_buttonSource(0)
{
}

Simple::~Simple()
{
}



void Simple::backup()
{
    mOldMode.assign("name");

//    Parameters::ExternalVariables_t externalVariables =
//        mParameters.getExternalVariables();
//    for (Parameters::ExternalVariables_t::const_iterator it =
//             externalVariables.begin(); it != externalVariables.end(); ++it) {
//        mOldExternalVariables.push_back(it->first);
//    }
}

void Simple::generate(vpz::AtomicModel& model,
                      vpz::Dynamic& dynamic,
                      vpz::Conditions& conditions,
                      vpz::Observables& observables,
                      const std::string& classname,
                      const std::string& namespace_,
                      bool generic)
{
    //generateDynamic
    std::cout << " DBG getCurrentPackage " << getCurrPackage() << std::endl;
    dynamic.setPackage(getCurrPackage());
    dynamic.setLibrary(classname);

    //generateSource;

    mFileNames.process(getCurrPackage());
    utils::Template tpl_(getTemplate());

    tpl_.stringSymbol().append("namespace", namespace_);
    tpl_.stringSymbol().append("classname", classname);

    // variables
    tpl_.stringSymbol().append("varsDeclaration",
                mFileNames.getCodeVarsDeclaration());
    tpl_.stringSymbol().append("varsInitialization",
            mFileNames.getCodeVarsInitialization());
    tpl_.stringSymbol().append("fileEquations",
            mFileNames.getCodeEquations());

    // parameters
    tpl_.listSymbol().append("par");
    tpl_.listSymbol().append("val");
    tpl_.listSymbol().append("file");

    std::vector<std::string>::const_iterator itb =
            mFileNames.getEquationFiles().begin();
    std::vector<std::string>::const_iterator ite =
            mFileNames.getEquationFiles().end();
    for (; itb!=ite; itb++) {
        tpl_.listSymbol().append("file",*itb);
    }




    //    Parameters::Parameters_t parameters = mParameters.getParameters();
    //    for (std::vector < std::string >::const_iterator it =
    //             parameters.mNames.begin(); it != parameters.mNames.end(); ++it) {
    //        std::string name(*it);
    //        std::map < std::string, double >::const_iterator it2 =
    //            parameters.mValues.find(name);
    //        tpl_.listSymbol().append("par", name);
    //        if (it2 != parameters.mValues.end()) {
    //            tpl_.listSymbol().append("val",
    //                                     utils::toScientificString(it2->second,
    //                                                               false));
    //
    //        } else {
    //            tpl_.listSymbol().append("val", "*");
    //        }
    //    }

    if (not generic) {
        // syncs / nosyncs
        tpl_.listSymbol().append("sync");
        tpl_.listSymbol().append("nosync");

        //        Parameters::ExternalVariables_t externalVariables =
        //            mParameters.getExternalVariables();
        //        for (Parameters::ExternalVariables_t::const_iterator it =
        //                 externalVariables.begin(); it != externalVariables.end();
        //             ++it) {
        //            std::string name(it->first);
        //
        //            if (it->second) {
        //                tpl_.listSymbol().append("sync", name);
        //            } else {
        //                tpl_.listSymbol().append("nosync", name);
        //            }
        //        }
    }

//    // includes/compute/initValue/user-functions
//    tpl_.stringSymbol().append("includes", mIncludes);
//    tpl_.stringSymbol().append("compute", mComputeFunction);
//    tpl_.stringSymbol().append("initValue", mInitValueFunction);
//    tpl_.stringSymbol().append("userFunctions", mUserFunctions);

    std::ostringstream out;
    tpl_.process(out);

    mSource = out.str();
}




void Simple::parseConf(const std::string& conf,
                       std::string& classname,
                       std::string& namespace_,
                       std::vector<std::string>& eqFiles
//                       Parameters::Parameters_t& parameters,
//                       Parameters::ExternalVariables_t& variables
                       )
{

    eqFiles.clear();
    std::vector < std::string > lst(3);

    boost::split(lst, conf, boost::is_any_of(";"));

// namespace
    namespace_ = std::string(lst[0], 10, lst[0].size() - 10);

// classname
    classname = std::string(lst[1], 6, lst[1].size() - 6);

// equation files
    {
        std::string files(lst[2], 6, lst[2].size() - 6);
        std::vector < std::string > lstfiles;
        boost::split(lstfiles, files, boost::is_any_of("|"));
        for (std::vector < std::string >::const_iterator it = lstfiles.begin();
                     it != lstfiles.end(); ++it) {
            if (not it->empty()) {
               eqFiles.push_back(*it);
            }
        }
    }

//// parameters
//    {
//        std::string pars(lst[2], 4, lst[2].size() - 4);
//        std::vector < std::string > lstpar;
//        boost::split(lstpar, pars, boost::is_any_of("|"));
//        for (std::vector < std::string >::const_iterator it = lstpar.begin();
//             it != lstpar.end(); ++it) {
//            if (not it->empty()) {
//                std::vector < std::string > par;
//                boost::split(par, *it, boost::is_any_of(","));
////                parameters.mNames.push_back(par[0]);
////                if (par[1] != "*") {
////                    parameters.mValues[par[0]] =
////                        boost::lexical_cast < double >(par[1]);
////                }
//            }
//        }
//    }
//
//    if (lst.size() > 3) {
//// syncs
//        {
//            std::string syncs(lst[3], 5, lst[3].size() - 5);
//            std::vector < std::string > lstsync;
//            boost::split(lstsync, syncs, boost::is_any_of("|"));
//            for (std::vector < std::string >::const_iterator it =
//                     lstsync.begin(); it != lstsync.end(); ++it) {
//                if (not it->empty()) {
////                    variables.push_back(make_pair(*it, true));
//                }
//            }
//        }
//
//// nosyncs
//        {
//            std::string nosyncs(lst[4], 7, lst[4].size() - 7);
//            std::vector < std::string > lstnosync;
//            boost::split(lstnosync, nosyncs, boost::is_any_of("|"));
//            for (std::vector < std::string >::const_iterator it =
//                     lstnosync.begin(); it != lstnosync.end(); ++it) {
//                if (not it->empty()) {
////                    variables.push_back(make_pair(*it, false));
//                }
//            }
//        }
//    }
}

std::string Simple::parseFunction(const std::string& buffer,
                                  const std::string& begin,
                                  const std::string& end,
                                  const std::string& name)
{
    boost::regex tagbegin(begin, boost::regex::grep);
    boost::regex tagend(end, boost::regex::grep);

    boost::sregex_iterator it(buffer.begin(), buffer.end(), tagbegin);
    boost::sregex_iterator jt(buffer.begin(), buffer.end(), tagend);
    boost::sregex_iterator itend;

    if (it == itend or jt == itend) {
        throw utils::ArgError(fmt(_("DifferenceEquation plugin error, " \
                                    "no begin or end tag (%1%)")) % name);
    }

    if ((*it)[0].second >= (*jt)[0].first) {
        throw utils::ArgError(fmt(_("DifferenceEquation plugin error, " \
                                    "bad tag (%1%)")) % name);
    }

    return std::string((*it)[0].second + 1, (*jt)[0].first);
}

void Simple::parseFunctions(const std::string& buffer)
{
//    mIncludes.assign(parseFunction(buffer, "//@@begin:includes@@",
//                                          "//@@end:includes@@", "includes"));
//    mComputeFunction.assign(parseFunction(buffer, "//@@begin:compute@@",
//                                          "//@@end:compute@@", "compute"));
//    mInitValueFunction.assign(parseFunction(buffer, "//@@begin:initValue@@",
//                                            "//@@end:initValue@@",
//                                            "initValue"));
//    mUserFunctions.assign(parseFunction(buffer, "//@@begin:user@@",
//                                        "//@@end:user@@", "user"));
}


void Simple::build(bool /*modeling*/)
{
    Gtk::VBox* vbox;

    vle::utils::Package pack(getPackage());

    std::string glade = pack.getPluginGvleModelingFile(
                "RecurrenceRelation.glade", vle::utils::PKG_BINARY);

    mXml = Gtk::Builder::create();
    mXml->add_from_file(glade.c_str());

    mXml->get_widget("DialogRecurrenceRelation", m_dialog);
    m_dialog->set_title("RecurrenceRelation");
    mXml->get_widget("SimplePluginVBox", vbox);

    vbox->pack_start(mFileNames.build(mXml));
//    vbox->pack_start(mTimeStep.build(mXml));
//    if (modeling) {
////        vbox->pack_start(mParameters.build(mXml));
//
//        {
//            m_buttonSource = Gtk::manage(
//                new Gtk::Button("Compute / InitValue / User section"));
//            m_buttonSource->show();
//            vbox->pack_start(*m_buttonSource);
//            mList.push_back(m_buttonSource->signal_clicked().connect(
//                                sigc::mem_fun(*this, &Plugin::onSource)));
//        }
//    }
    //vbox->pack_start(mMapping.build(mXml));
}

bool Simple::create(vpz::AtomicModel& model,
                    vpz::Dynamic& dynamic,
                    vpz::Conditions& conditions,
                    vpz::Observables& observables,
                    const std::string& classname,
                    const std::string& namespace_)
{
    std::string conditionName((fmt("cond_DE_%1%") % model.getName()).str());

    build(true);

    //Fill fields

//    mIncludes = "";
//    mComputeFunction = "return 0;\n";

    if (m_dialog->run() == Gtk::RESPONSE_ACCEPT) {
        generate(model, dynamic, conditions, observables, classname,
                 namespace_);
        m_dialog->hide_all();
        destroy();
        return true;
    }
    m_dialog->hide_all();
    destroy();
    return false;
}

void Simple::destroy()
{
    if (m_buttonSource) {
        Gtk::VBox* vbox;

        mXml->get_widget("SimplePluginVBox", vbox);
        vbox->remove(*m_buttonSource);
    }


    for (std::list < sigc::connection >::iterator it = mList.begin();
         it != mList.end(); ++it) {
        it->disconnect();
    }

    std::cout << " DBG destroy " << std::endl;
}



std::string Simple::getTemplate() const
{
    return
    "/**\n"                                                             \
    "  * @file {{classname}}.cpp\n"                                     \
    "  * @author ...\n"                                                 \
    "  * ...\n"                                                         \
    "  * @@tag RecurrenceRelation@vle.extension.recurrence-relation @@" \
    "namespace:{{namespace}};"                                          \
    "class:{{classname}};files:"                                        \
    "{{for i in file}}"                                                 \
    "{{file^i}}|"                                                       \
    "{{end for}}"                                                       \
    "@@end tag@@\n"                                                     \
    "  */\n\n"                                                          \
    "#include <vle/extension/RecurrenceRelation.hpp>\n\n"               \
    "namespace vd = vle::devs;\n"                                       \
    "namespace ve = vle::extension;\n"                                  \
    "namespace vv = vle::value;\n"                                      \
    "namespace ver = vle::extension::recurrenceRelation;\n\n"           \
    "namespace {{namespace}} {\n\n"                                     \
    "class {{classname}} : public ver::RecurrenceRelation\n"             \
    "{\n"                                                               \
    "public:\n"                                                         \
    "    {{classname}}(\n"                                              \
    "       const vd::DynamicsInit& atom,\n"                            \
    "       const vd::InitEventList& evts)\n"                           \
    "        : ver::RecurrenceRelation(atom, evts)\n"                    \
    "    {\n"                                                           \
    "{{varsInitialization}}        \n"                                  \
    "    }\n"                                                           \
    "\n"                                                                \
    "    virtual ~{{classname}}()\n"                                    \
    "    {}\n"                                                          \
    "\n"                                                                \
    "void compute(const vle::devs::Time& /*time*/)\n"                   \
    "{\n"                                                               \
    "{{fileEquations}}"                                                 \
    "}\n"                                                               \
    "private:\n"                                                        \
    "{{varsDeclaration}}        \n"                                     \
    "};\n\n"                                                            \
    "} // namespace {{namespace}}\n\n"                                  \
    "DECLARE_DYNAMICS({{namespace}}::{{classname}})\n\n";
}

bool Simple::modify(vpz::AtomicModel& model,
                    vpz::Dynamic& dynamic,
                    vpz::Conditions& conditions,
                    vpz::Observables& observables,
                    const std::string& conf,
                    const std::string& buffer)
{
    std::string namespace_;
    std::string classname;
//    Parameters::Parameters_t parameters;
//    Parameters::ExternalVariables_t externalVariables;
    std::vector<std::string> eqFiles;
    parseConf(conf, classname, namespace_, eqFiles);
    parseFunctions(buffer);
    std::string conditionName((fmt("cond_DE_%1%") % model.getName()).str());

    build(true);

    mFileNames.fillField(eqFiles);

    backup();

    if (m_dialog->run() == Gtk::RESPONSE_ACCEPT) {
        generate(model, dynamic, conditions, observables, classname,
                 namespace_);
        m_dialog->hide_all();
        destroy();
        return true;
    }
    m_dialog->hide_all();
    destroy();
    return false;
}

bool Simple::start(vpz::Condition& condition)
{
    build(false);

    if (m_dialog->run() == Gtk::RESPONSE_ACCEPT) {
    }
    m_dialog->hide();
    destroy();
    return true;
}


}}}} // namespace vle gvle modeling de

DECLARE_GVLE_MODELINGPLUGIN(vle::gvle::modeling::rr::Simple)

