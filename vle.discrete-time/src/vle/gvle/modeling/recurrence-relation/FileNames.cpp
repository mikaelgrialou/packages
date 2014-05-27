/*
 * @file vle/gvle/modeling/recurrence-relation/NameValue.cpp
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


#include <sstream>
#include <numeric>
#include <fstream>
#include <iostream>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_push_back_actor.hpp>
using namespace BOOST_SPIRIT_CLASSIC_NS;

#include <vle/gvle/modeling/recurrence-relation/FileNames.hpp>
#include <vle/gvle/Message.hpp>
#include <vle/gvle/SimpleTypeBox.hpp>
#include <vle/value/Double.hpp>
#include <vle/utils/Tools.hpp>
#include <vle/utils/Package.hpp>

namespace vu = vle::utils;

namespace vle { namespace gvle { namespace modeling { namespace rr {


/*  - - - - - - - - - - - - - --ooOoo-- - - - - - - - - - - -  */

struct createVar_grammar : public grammar<createVar_grammar>
{
    struct fill_var_name {
        fill_var_name(FileNames& fileNames):
            tofill_fn(fileNames)
        {
        }
        void operator()(const char* beg, const char* end) const
        {
            std::string id(beg, end);
            tofill_fn.mvars.push_back(id);
            std::string code = "ver::RecurrenceRelation::Var ";
            code.append(id);
            code.append(";\n");
            tofill_fn.mvarsDeclaration.append(code);
            //TODO
        }
        FileNames& tofill_fn;
    };

    FileNames& tofill_fn;
    fill_var_name fn;

    createVar_grammar(FileNames& fileNames):
        tofill_fn(fileNames), fn(fileNames)
    {
    }



    template <typename ScannerT>
    struct definition
    {
        //lexcial elements
        rule<ScannerT>  lex_createVar;
        //rules
        rule<ScannerT> rule_VarNames;
        //main rule
        rule<ScannerT>  rule_root;

        //elemtns to fill
        fill_var_name const& fn_loc;

        definition(createVar_grammar const& self) : fn_loc(self.fn)
        {
            //lexcial elements
            lex_createVar = (str_p("createVar"));

            //rules
            rule_VarNames =
                    (
                     (+alnum_p)[fn_loc]
                    );
            //root level
            rule_root =
                    (
                     *blank_p >> rule_VarNames >> *blank_p >> '='
                     >> *blank_p  >> lex_createVar >> "("
                     >> "\"" >> (+alnum_p) >> "\"" >> ")" >> ";"
                    );
        }
        rule<ScannerT> const& start() const { return rule_root; }
    };
};

void FileNames::process(const std::string& package)
{

    mvarsDeclaration.clear();
    mequations.clear();
    mfiles.clear();

    std::string eqFiles = meqFiles->get_text();

    std::vector<std::string> files;
    boost::split(files, eqFiles, boost::is_any_of(";"));
    vu::Package currPack(package);
    bool declarationVars;
    std::string line;
    std::vector<std::string>::const_iterator itb = files.begin();
    std::vector<std::string>::const_iterator ite = files.end();
    for (;itb != ite;itb++) {
        mfiles.push_back(*itb);
        std::string dataFile = currPack.getDataFile(*itb, vu::PKG_SOURCE);
        std::ifstream fileStream(dataFile.c_str());
        declarationVars = true;
        while (std::getline(fileStream, line)) {
            if (declarationVars &&
                (line.find("createVar") != std::string::npos)) {
                mvarsInitialization.append(line);
                mvarsInitialization.append("\n");
                boost::replace_all(line, "\r\n", "");
                boost::replace_all(line, "\n", "");
                createVar_grammar g(*this);
                parse_info<> resParsing =
                        BOOST_SPIRIT_CLASSIC_NS::parse(line.c_str(), g);
                if (resParsing.full ) {
                    //std::cout << resParsing << std::endl;
                    //std::cout << g.wp << std::endl;
                    //std::cout << " ok " << std::endl;
                } else {
                    //std::cout << " ERROR NOT ok " << std::endl;
                    //std::string parsed(fileString.c_str(), resParsing.stop);
                    //std::cout << " parsed:\n" << parsed << std::endl;
                    std::cout << " ERROR NOT ok " << std::endl;
                }
            } else if (declarationVars &&
                 (line.find("createVect") != std::string::npos)) {
                mvarsInitialization.append(line);
                mvarsInitialization.append("\n");
            } else {
                declarationVars = false;
                mequations.append(line);
                mequations.append("\n");
            }
        }
    }
}

void FileNames::fillField(const std::vector<std::string>& eqFiles)
{
    std::string text("");
    for (unsigned int i = 0; i < eqFiles.size(); i++) {
        text.append(eqFiles[i]);
        if (i < eqFiles.size() -1) {
            text.append(";");
        }
    }
    meqFiles->set_text(text);
}

Gtk::Widget& FileNames::build(Glib::RefPtr < Gtk::Builder >& ref)
{
    ref->get_widget("NameValueHBox", m_hbox);
    ref->get_widget("NameEntry", meqFiles);
    return *m_hbox;
}


}}}} // namespace vle gvle modeling de
