/*
 * synergy -- mouse and keyboard sharing utility
 * Copyright (C) 2012-2016 Symless Ltd.
 * Copyright (C) 2002 Chris Schoeneman
 *
 * This package is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * found in the file LICENSE that should have accompanied this file.
 *
 * This package is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <fstream>
#include <sstream>
#include <algorithm>

#include "base/Log.h"
#include "synergy/X11LayoutsParser.h"
#include "ISO639Table.h"
#include "pugixml.hpp"
#include <X11/XKBlib.h>
#include <X11/extensions/XKBrules.h>

bool
X11LayoutsParser::readXMLConfigItemElem(const pugi::xml_node* root, std::vector<Lang>& langList)
{
    auto configItemElem = root->child("configItem");
    if(!configItemElem) {
        LOG((CLOG_WARN "Failed to read \"configItem\" in xml file"));
        return false;
    }

    langList.emplace_back();
    auto nameElem = configItemElem.child("name");
    if(nameElem) {
        langList.back().name = nameElem.text().as_string();
    }

    auto languageListElem = configItemElem.child("languageList");
    if(languageListElem) {
        for (pugi::xml_node isoElem : languageListElem.children("iso639Id")) {
            langList.back().layoutBaseISO639_2.emplace_back(isoElem.text().as_string());
        }
    }

    return true;
}

std::vector<X11LayoutsParser::Lang>
X11LayoutsParser::getAllLanguageData(const String& pathToEvdevFile)
{
    std::vector<Lang> allCodes;
    pugi::xml_document doc;
    if(!doc.load_file(pathToEvdevFile.c_str())) {
        LOG((CLOG_WARN "Failed to open %s", pathToEvdevFile.c_str()));
        return allCodes;
    }

    auto xkbConfigElem = doc.child("xkbConfigRegistry");
    if(!xkbConfigElem) {
        LOG((CLOG_WARN "Failed to read xkbConfigRegistry in %s", pathToEvdevFile.c_str()));
        return allCodes;
    }

    auto layoutListElem = xkbConfigElem.child("layoutList");
    if(!layoutListElem) {
        LOG((CLOG_WARN "Failed to read layoutList in %s", pathToEvdevFile.c_str()));
        return allCodes;
    }

    for (pugi::xml_node layoutElem : layoutListElem.children("layout")) {
        if (!readXMLConfigItemElem(&layoutElem, allCodes)) {
            continue;
        }

        auto variantListElem = layoutElem.child("variantList");
        if (variantListElem) {
            for (pugi::xml_node variantElem : variantListElem.children("variant")) {
                readXMLConfigItemElem(&variantElem, allCodes.back().variants);
            }
        }
    }

    return allCodes;
}

void
X11LayoutsParser::appendVectorUniq(const std::vector<String>& source, std::vector<String>& dst) {
    for(const auto& elem : source) {
        if(std::find_if(dst.begin(), dst.end(), [elem](const String& s) {return s == elem;}) == dst.end()) {
            dst.push_back(elem);
        }
    }
};

void
X11LayoutsParser::convertLayoutToISO639_2(const String&        pathToEvdevFile,
                                          bool                 needToReloadEvdev,
                                          std::vector<String>  layoutNames,
                                          std::vector<String>  layoutVariantNames,
                                          std::vector<String>& iso639_2Codes)
{
    if(layoutNames.size() != layoutVariantNames.size()) {
        LOG((CLOG_WARN "Error in language layout or language layout variants list"));
        return;
    }

    static std::vector<X11LayoutsParser::Lang> allLang;
    if(allLang.empty() || needToReloadEvdev) {
        allLang = getAllLanguageData(pathToEvdevFile);
    }
    for (size_t i = 0; i < layoutNames.size(); i++) {
        auto langIter = std::find_if(allLang.begin(), allLang.end(), [n=layoutNames[i]](const Lang& l) {return l.name == n;});
        if(langIter == allLang.end()) {
            LOG((CLOG_WARN "Language \"%s\" is unknown", layoutNames[i].c_str()));
            continue;
        }

        const std::vector<String>* toCopy = nullptr;
        if(layoutVariantNames[i].empty()) {
            toCopy = &langIter->layoutBaseISO639_2;
        }
        else {
            auto langVariantIter = std::find_if(langIter->variants.begin(), langIter->variants.end(),
                                                [n=layoutVariantNames[i]](const Lang& l) {return l.name == n;});
            if(langVariantIter == langIter->variants.end()) {
                LOG((CLOG_WARN "Variant \"%s\" of language \"%s\" is unknown", layoutVariantNames[i].c_str(), layoutNames[i].c_str()));
                continue;
            }

            if(langVariantIter->layoutBaseISO639_2.empty()) {
                toCopy = &langIter->layoutBaseISO639_2;
            }
            else {
                toCopy = &langVariantIter->layoutBaseISO639_2;
            }
        }

        if(toCopy) {
            appendVectorUniq(*toCopy, iso639_2Codes);
        }
    }
}

std::vector<String>
X11LayoutsParser::getX11LanguageList(const String& pathToKeyboardFile, const String& pathToEvdevFile)
{
    std::vector<String> layoutNames;
    std::vector<String> layoutVariantNames;
    std::vector<String> iso639_2Codes;

    parseKeyboardFile(pathToKeyboardFile, layoutNames, layoutVariantNames);
    convertLayoutToISO639_2(pathToEvdevFile, true, layoutNames, layoutVariantNames, iso639_2Codes);
    return convertISO639_2ToISO639_1(iso639_2Codes);
}

String
X11LayoutsParser::convertLayotToISO(const String& pathToEvdevFile, const String& layoutLangCode, bool needToReloadFiles)
{
    std::vector<String> iso639_2Codes;
    convertLayoutToISO639_2(pathToEvdevFile, needToReloadFiles, {layoutLangCode}, {""}, iso639_2Codes);
    if(iso639_2Codes.empty()) {
        LOG((CLOG_WARN "Failed to convert layout lang code! Code: \"%s\"", layoutLangCode.c_str()));
        return "";
    }

    auto iso639_1Codes = convertISO639_2ToISO639_1(iso639_2Codes);
    if(iso639_1Codes.empty()) {
        LOG((CLOG_WARN "Failed to convert ISO639/2 lang code to ISO639/1!"));
        return "";
    }

    return *iso639_1Codes.begin();
}

void
X11LayoutsParser::parseKeyboardFile(const String&        pathToKeyboardFile,
                                    std::vector<String>& layoutNames,
                                    std::vector<String>& layoutVariantNames)
{
    layoutNames.clear();
    layoutVariantNames.clear();

    XkbRF_VarDefsRec vd;
    char *tmp = NULL;
    auto dpy = XkbOpenDisplay(NULL, NULL, NULL, NULL, NULL, NULL);

    auto splitLine = [](std::vector<String>& splitted, const String& line, char delim) {
            std::stringstream ss(line);
            String code;
            while(ss.good()) {
                getline(ss, code, delim);
                splitted.push_back(code);
            }
        };


    if (XkbRF_GetNamesProp(dpy, &tmp, &vd))
    {
        splitLine(layoutNames, vd.layout, ',');
        splitLine(layoutVariantNames, vd.variant, ',');
        LOG((CLOG_INFO "Layouts: %s", vd.layout));
        LOG((CLOG_INFO "Variants: %s", vd.variant));
    }
    else {
        LOG((CLOG_INFO "Error reading languages"));
    }

}

std::vector<String>
X11LayoutsParser::convertISO639_2ToISO639_1(const std::vector<String>& iso639_2Codes)
{
    std::vector<String> result;
    for (const auto& isoCode : iso639_2Codes) {
        const auto& tableIter = std::find_if(ISO_Table.begin(), ISO_Table.end(),
                                            [&isoCode](const std::pair<String, String>& c) {return c.first == isoCode;});
        if(tableIter == ISO_Table.end()) {
            LOG((CLOG_WARN "ISO 639-2 code \"%s\" is missed in table", isoCode.c_str()));
            continue;
        }

        appendVectorUniq({tableIter->second}, result);
    }

    return result;
}
