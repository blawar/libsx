#include "sst.h"
#include "strings.h"
#include <vector>
#include <iostream>
#include <boost/lexical_cast.hpp>

sstCssNode::sstCssNode()
{
}

sstCssNode::~sstCssNode()
{
}

sstCssNode* sstCssNode::operator[](std::string key)
{
	return cssMap[key].get();
}

void sstCssNode::dump(std::string &buffer, long depth)
{
	std::string value;
	std::string tabs = "";
	for(long j=0; j < depth; j++)
	{
		tabs += "\t";
	}
	//if(!cssMap.empty())
	{
	buffer += tabs + "{\n";
	//for(CSSMAP::const_iterator it = cssMap.begin(); it != cssMap.end(); ++it)
	for(long it=0; it < cssMap.size(); it++)
	{
		sstCssNode* n = cssMap[it].get();
		value = n->getValue();
		if(value.size())
		{
			buffer+= tabs + "\t" + cssMap.key(it) + ": " + value + "\n";
		}
		else
		{
			buffer+= tabs + "\t" + cssMap.key(it) + "\n";
			n->dump(buffer, depth+1);
		}
	}
	buffer += tabs + "}\n";
	}
}

SST::SST()
{
}

SST::~SST()
{
}

void SST::addXml(std::string &buffer)
{
	xml += buffer;
}

void SST::addStyleSheet(std::string &buffer)
{
	css += buffer;
}

/*void implode(std::string delimiter, std::string )
{
}*/

void SST::applySelector(TiXmlElement* element, sstCssNode *cssRoot)
{
	SSTELEMENTS elements;
	elements.push_back(element);
	applySelector(elements, cssRoot);
}

void SST::applySelector(SSTELEMENTS elements, sstCssNode *cssRoot)
{
	std::string style, value, tmp;
	CSSMAP* map = cssRoot->map();
	for(long it=0; it < map->size(); it++)
	{
		value = (*map)[map->key(it)].getValue();
		if(value.size())
		{
			for(long i=0; i < elements.size(); i++)
			{
				xmlStyle[elements[i]][map->key(it)] = value;
			}
		}
	}

	for(long it=0; it < map->size(); it++)
        {
                if((*map)[map->key(it)].map()->empty())
                {
			continue;
		}
		SSTELEMENTS e;
		for(long i=0; i < elements.size(); i++)
		{
			selector(map->key(it).c_str(), SSTELEMENTS(elements[i]->FirstChildElement()), &e);
                	applySelector(e, (*map)[map->key(it)].get());
		}
        }
}

void SST::applyStyles()
{
	std::string style;
	for(ATTRIBUTES::const_iterator it = xmlStyle.begin(); it != xmlStyle.end(); ++it)
	{
		style = "";
		TiXmlElement* e = it->first;
		const char* p = e->Attribute("style");
		if(p) style = p;
		for(std::map<std::string, std::string>::const_iterator at = it->second.begin(); at != it->second.end(); ++at)
		{
			style += at->first + ":" + at->second + ";";
		}
		e->SetAttribute("style", style.c_str());
	}
}

#ifdef WINDOWS
#define foreach(o, k, v) \
decltype((o).begin()) it = (o).begin(); \
decltype(it->first)* k; \
decltype(it->second)* v; \
for(; ((k = &it->first) && (v = &it->second)) && it != (o).end(); ++it)
#else
#define foreach(o, k, v) \
typeof((o).begin()) it = (o).begin(); \
typeof(it->first)* k; \
typeof(it->second)* v; \
for(; ((k = &it->first) && (v = &it->second)) && it != (o).end(); ++it)
#endif

#define $(selector) selector(selector, doc.RootElement())

void SST::applyClasses()
{
	bool bFound;
	long class_i;
	std::string cls;
	SSTELEMENTS classes;

	foreach(xmlStyle, element, attrs)
	{
		class_i = -1;
		for(long i=0; i < classes.size(); i++)
		{
			//ATTRIBUTES* classAttrs = xmlStyle.at(classes[i]);

			if(xmlStyle[classes[i]].size() != (*attrs).size()) continue;

			bFound = true;
			foreach(xmlStyle[classes[i]], key, value)
			{
				if(*value != (*attrs).operator[](*key))
				{
					bFound = false;
					break;
				}
			}

			if(!bFound) continue;

			class_i = i;
			break;
		}

		const char* p = (*element)->Attribute("class");
		if(p)
		{
			cls = p;
			cls += " ";
		}
		else cls = "";

		if(class_i == -1)
		{
			classes.push_back(*element);
			class_i = classes.size() - 1;
		}
		cls += "c" + boost::lexical_cast<std::string>(class_i);
		(*element)->SetAttribute("class", cls.c_str());
	}

	cls = "";
	{
		TiXmlElement* element = new TiXmlElement("style");

		for(long i=0; i < classes.size(); i++)
		{
			cls += ".c" + boost::lexical_cast<std::string>(i) + "{";

			foreach(xmlStyle[classes[i]], key, value)
			{
				cls += (*key) + ":" + (*value) + ";";
			}

			cls += "} ";
		}
		TiXmlText *text = new TiXmlText(cls.c_str());
		element->LinkEndChild(text);
		TiXmlElement  *head = selector("head", doc.RootElement());
		if(head) head->LinkEndChild(element);
		else printf("Could not find head doc\n");
	}
}

TiXmlElement* SST::selector(const char* pszSearch, TiXmlElement* parent)
{
	SSTELEMENTS result;
	selector(pszSearch, SSTELEMENTS(parent), &result);
	if(result.empty()) return NULL;
	return result[0];
}

SSTELEMENTS SST::selector(const char* pszSearch, SSTELEMENTS parents)
{
	SSTELEMENTS elements;
	return elements;
}

SSTELEMENTS* SST::selector(const char* pszSearch, SSTELEMENTS parents, SSTELEMENTS *result)
{
	if(!pszSearch) return result;
	//if(!parents) return result;
	if(!parents.size()) return result;

	STRINGS strings;
	tokenize(pszSearch, strings);
	return selectorDescendant(strings, 0, parents, result);
}

SSTELEMENTS* SST::getElementsByAttr(const char* attr, const char* value, SSTELEMENTS parents, SSTELEMENTS *result)
{
	const char* eAttr;

	for(long i=0; i < parents.size(); i++)
	{
	TiXmlElement* e = parents[i];//->FirstChildElement();

	if(!e) continue;

	do
	{
		eAttr = e->Attribute(attr);
		if(eAttr) 
		{
			if(!strcmp(eAttr, value))
			{
				result->push_back(e);
			} 
		}
		getElementsByAttr(attr, value, e->FirstChildElement(), result);
	}
	while(e = e->NextSiblingElement());
	}
	return result;
}

SSTELEMENTS* SST::getElementsByTag(const char* tag, SSTELEMENTS parents, SSTELEMENTS *result)
{
	for(long i=0; i < parents.size(); i++)
	{
        TiXmlElement* e = parents[i];//->FirstChildElement();

	if(!e) continue;
        do
        {
		if(e->Value())
		{
                	if(!strcmp(e->Value(), tag))
                	{
                		result->push_back(e);
                	}
                	getElementsByTag(tag, e->FirstChildElement(), result);
		}
        }
        while(e = e->NextSiblingElement());
	}
	return result;
}

SSTELEMENTS* SST::selectorDescendant(STRINGS &bits, long pos, SSTELEMENTS parents, SSTELEMENTS *result)
{
	SSTELEMENTS tmp;
	const char* pszTag, *pszClass, *pszId;

		for(long x=0; x < bits.size(); x++)
		{
			tmp.clear();
			//TiXmlElement *e = parents[i];
			if(*bits[pos] == '.')
			{
				if(!x) getElementsByAttr("class", bits[pos]+1, parents, &tmp);
				else getElementsByAttr("class", bits[pos]+1, tmp, &tmp);
			}
			else if(*bits[pos] == '#')
			{
				if(!x) getElementsByAttr("id", bits[pos]+1, parents, &tmp);
				else getElementsByAttr("id", bits[pos]+1, tmp, &tmp);
			}
			else if(*bits[pos] == '>')
			{
				//selectorChild(bits, pos, *result, result);
			}
			else
			{
				if(!x)
				{
					getElementsByTag(bits[pos], parents, &tmp);
				}
				else
				{
					getElementsByTag(bits[pos], tmp, &tmp);
				}
			}
			tmp.copy(result);
		}
	return result;
}

void SST::render(std::string &buffer)
{
	bool bIsValue;
	std::string attrName;
	std::string attrValue;
	std::vector<sstCssNode*> selectors;
	sstCssNode* rootSelector;
	long selector_i=0;
	std::vector<std::string> attributes;
	std::string w;
	const char* tmp;
	buffer = "";
	int64 x;
	STRINGS strings;
	tokenize(css.c_str(), strings);
	int64 len = strings.size();
	for(int64 i=0; i < len; i++)
	{
		selectors.clear();
		w = strings[i];
		selector_i = 0;
		rootSelector = cssRoot[w];

		for(i++; i < len; i++) // get selectors
		{
			if(*strings[i] == ',')
			{
				selectors.push_back(rootSelector);
				rootSelector = cssRoot[strings[++i]];
				selector_i++;
				continue;
			}
			else if(*strings[i] == '{') break;
			else
			{
				w = strings[i];
				rootSelector = rootSelector->operator[](w);
			}
		}
		selectors.push_back(rootSelector);
		selector_i++;

		attrName = "";
		attrValue = "";
		bIsValue = false;
		attributes.clear();

		for(i++; i < len; i++)
		{
			tmp = strings[i];
			if(*tmp == ':')
			{
				bIsValue = true;
				continue;
			}
			else if(*tmp == ';')
			{
				for(long j=0; j < selector_i; j++)
				{
					if(!attrName.compare("padding") || !attrName.compare("margin"))
					{
						switch(attributes.size())
						{
							case 4:
							(*selectors[j])[attrName+"-top"]->operator=(attributes[0]);
							(*selectors[j])[attrName+"-right"]->operator=(attributes[1]);
							(*selectors[j])[attrName+"-bottom"]->operator=(attributes[2]);
							(*selectors[j])[attrName+"-left"]->operator=(attributes[3]);
							break;
							case 3:
							(*selectors[j])[attrName+"-top"]->operator=(attributes[0]);
                                                        (*selectors[j])[attrName+"-right"]->operator=(attributes[1]);
                                                        (*selectors[j])[attrName+"-bottom"]->operator=(attributes[2]);
                                                        (*selectors[j])[attrName+"-left"]->operator=(attributes[1]);
							break;
							case 2:
							(*selectors[j])[attrName+"-top"]->operator=(attributes[0]);
                                                        (*selectors[j])[attrName+"-right"]->operator=(attributes[1]);
                                                        (*selectors[j])[attrName+"-bottom"]->operator=(attributes[0]);
                                                        (*selectors[j])[attrName+"-left"]->operator=(attributes[1]);
							break;
							case 1:
							default:
							(*selectors[j])[attrName+"-top"]->operator=(attributes[0]);
                                                        (*selectors[j])[attrName+"-right"]->operator=(attributes[0]);
                                                        (*selectors[j])[attrName+"-bottom"]->operator=(attributes[0]);
                                                        (*selectors[j])[attrName+"-left"]->operator=(attributes[0]);
						}
					}
					else if(!attrName.compare("border"))
					{
						if(attributes.size() >= 1)
						{
						(*selectors[j])["border-top-width"]->operator=(attributes[0]);
						(*selectors[j])["border-right-width"]->operator=(attributes[0]);
						(*selectors[j])["border-bottom-width"]->operator=(attributes[0]);
						(*selectors[j])["border-left-width"]->operator=(attributes[0]);
						}

						if(attributes.size() >= 2)
						{
						(*selectors[j])["border-top-style"]->operator=(attributes[1]);
                                                (*selectors[j])["border-right-style"]->operator=(attributes[1]);
                                                (*selectors[j])["border-bottom-style"]->operator=(attributes[1]);
                                                (*selectors[j])["border-left-style"]->operator=(attributes[1]);
						}

						if(attributes.size() >= 3)
						{
						(*selectors[j])["border-top-color"]->operator=(attributes[2]);
                                                (*selectors[j])["border-right-color"]->operator=(attributes[2]);
                                                (*selectors[j])["border-bottom-color"]->operator=(attributes[2]);
                                                (*selectors[j])["border-left-color"]->operator=(attributes[2]);
						}
					}
					else
					{
						(*selectors[j])[attrName]->operator=(attrValue);
					}
				}
				attrName = attrValue = "";
				bIsValue = false;
				attributes.clear();
			}
			else if(*tmp == '}') break;
			else
			{
				if(bIsValue)
				{
					attributes.push_back(tmp);
					attrValue += tmp;
				}
				else
				{
					attrName += tmp;
				}
			}
		}
	}

        if(!doc.Parse(xml.c_str(), 0, TIXML_ENCODING_UTF8))
        {
                buffer = "failed!";
		return;
        }
	applySelector(doc.RootElement(), &cssRoot);
	//applyStyles();
	applyClasses();
	TiXmlPrinter printer;
	printer.SetIndent("\t");
	doc.Accept(&printer);
	buffer = printer.CStr();
	//cssRoot.dump(buffer);
}
