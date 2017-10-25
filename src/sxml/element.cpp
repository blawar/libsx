#include <boost/lexical_cast.hpp>

#include "sxml.h"

//std::vector<string> htmlSelfClose("img", "link", "input", "br", "meta", "hr", "base", "basefont", "area");

cssNode::cssNode()
{
}

cssNode::~cssNode()
{
}

cssNode* cssNode::operator[](const char* key)
{
	return cssMap[key].get();
}

cssNode* cssNode::operator[](string key)
{
        return cssMap[key.c_str()].get();
}

void cssNode::dump(string &buffer, long depth)
{
	string value;
	string tabs = "";
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
		cssNode* n = cssMap[it].get();
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

SxmlDocument::operator SxmlElement*()
{
	return RootElement();
}

/*void implode(string delimiter, string )
{
}*/

void SxmlDocument::applySelector(SxmlElement* element, cssNode *cssRoot)
{
	ELEMENTS elements;
	elements.push_back(element);
	applySelector(elements, cssRoot);
}

void SxmlDocument::applySelector(ELEMENTS elements, cssNode *cssRoot)
{
	string style, value, tmp;
	CSSMAP* map = cssRoot->map();
	for(long it=0; it < map->size(); it++)
	{
		value = (*map)[map->key(it)].getValue();
		if(value.size())
		{
			for(long i=0; i < elements.size(); i++)
			{
				elements[i]->style[map->key(it)] = value;
				//styleList[elements[i]][map->key(it)] = value;
			}
		}
	}

	for(long it=0; it < map->size(); it++)
        {
                if((*map)[map->key(it)].map()->empty())
                {
			continue;
		}
		ELEMENTS e;
		for(long i=0; i < elements.size(); i++)
		{
			selector(map->key(it).c_str(), ELEMENTS(elements[i]->FirstChildElement()), &e);
                	applySelector(e, (*map)[map->key(it)].get());
		}
        }
}

void SxmlDocument::applyStyles()
{
/*
	string style;
	for(ATTRIBUTES::const_iterator it = styleList.begin(); it != styleList.end(); ++it)
	{
		style = "";
		SxmlElement* e = it->first;
		const char* p = e->Attribute("style");
		if(p) style = p;
		for(std::map<string, string>::const_iterator at = it->second.begin(); at != it->second.end(); ++at)
		{
			style += at->first + ":" + at->second + ";";
		}
		e->SetAttribute("style", style.c_str());
	}
*/
}

#define foreach(o, k, v) \
typeof((o).begin()) it = (o).begin(); \
typeof(it->first)* k; \
typeof(it->second)* v; \
for(; ((k = &it->first) && (v = &it->second)) && it != (o).end(); ++it)

#define $(selector) selector(selector, doc.RootElement())

int32 SxmlDocument::generateClass(SxmlElement* element)
{
	if(element->style.size() == 0) return 0;
	for(int32 i=0; i < styleList.size(); i++)
	{
		if(element->style.size() != styleList[i]->size()) continue;
		return i+1;
	}
	styleList.push_back(&element->style);
	return styleList.size();
}

int32 SxmlDocument::generateClasses(SxmlElement* element, SxmlElement* parent)
{
        if(!parent) parent = RootElement();
	if(!element) return 0;

        int32 classId;

        SxmlElement *child = element;
        for(int32 classId = 0; child; child = child->NextSiblingElement())
        {
		classId = generateClass(child);
		if(classId) child->SetAttribute("class", (string("c") + parseInt(classId)).c_str());
		generateClasses(child->FirstChildElement(), parent);
        }
        return 0;
}

void SxmlDocument::applyClasses()
{

	string cls;
	generateClasses(RootElement());

	cls = "";
	{
		SxmlElement* element = new SxmlElement("style");

		for(long i=0; i < styleList.size(); i++)
		{
			cls += ".c" + boost::lexical_cast<string>(i+1) + "{";

			sxmlstyle *style = styleList[i];

			for(long x=0; x < style->size(); x++)
			{
				cls += style->key(x) + ":" + (*style)[x] + ";";
			}

			cls += "} ";
		}
		SxmlText *text = new SxmlText(cls.c_str());
		element->LinkEndChild(text);
		SxmlElement  *head = selector("head", RootElement());
		if(head) head->LinkEndChild(element);
		else printf("Could not find head doc\n");
	}

}

SxmlElement* SxmlDocument::selector(const char* pszSearch, SxmlElement* parent)
{
	ELEMENTS result;
	selector(pszSearch, ELEMENTS(parent), &result);
	if(result.empty()) return NULL;
	return result[0];
}

ELEMENTS SxmlDocument::selector(const char* pszSearch, ELEMENTS parents)
{
	ELEMENTS elements;
	return elements;
}

ELEMENTS* SxmlDocument::selector(const char* pszSearch, ELEMENTS parents, ELEMENTS *result)
{
	if(!pszSearch) return result;
	//if(!parents) return result;
	if(!parents.size()) return result;

	STRINGS strings;
	tokenize(pszSearch, strings);
	return selectorDescendant(strings, 0, parents, result);
}

ELEMENTS* SxmlDocument::getElementsByAttr(const char* attr, const char* value, ELEMENTS parents, ELEMENTS *result)
{
	const char* eAttr;

	for(long i=0; i < parents.size(); i++)
	{
	SxmlElement* e = parents[i];//->FirstChildElement();

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

ELEMENTS* SxmlDocument::getElementsByTag(const char* tag, ELEMENTS parents, ELEMENTS *result)
{
	for(long i=0; i < parents.size(); i++)
	{
        SxmlElement* e = parents[i];//->FirstChildElement();

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

ELEMENTS* SxmlDocument::selectorDescendant(STRINGS &bits, long pos, ELEMENTS parents, ELEMENTS *result)
{
	ELEMENTS tmp;
	const char* pszTag, *pszClass, *pszId;

		for(long x=0; x < bits.size(); x++)
		{
			tmp.clear();
			//SxmlElement *e = parents[i];
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

void SxmlDocument::render(string &buffer)
{
	RootElement()->render(buffer);
}

void SxmlDocument::addStyleSheet(string &buffer)
{
	bool bIsValue;
	string attrName;
	string attrValue;
	std::vector<cssNode*> selectors;
	cssNode* rootSelector;
	long selector_i=0;
	std::vector<string> attributes;
	string w;
	const char* tmp;
	int64 x;
	STRINGS strings;
	tokenize(buffer.c_str(), strings);
	int64 len = strings.size();
	for(int64 i=0; i < len; i++)
	{
		selectors.clear();
		w = strings[i];
		selector_i = 0;
		rootSelector = cssRoot[w.c_str()];

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
				rootSelector = rootSelector->operator[](w.c_str());
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


	applySelector(RootElement(), &cssRoot);
	applyClasses();
}

void SxmlElement::render(string &buffer)
{
	string tmp;
	const char* text;
        SxmlElement* e = this, *child;//->FirstChildElement();

        if(!e) return;
        do
        {
                if(e->Value())
                {
			buffer += string("<") + e->Value();
			SxmlAttribute* attr = e->FirstAttribute();
			while(attr)
			{
				buffer += string(" ") + attr->Name();
				if(attr->Value() && *attr->Value())
				{
					buffer += string("=\"");
					SxmlBase::EncodeString(attr->ValueStr(), &buffer);
					buffer += "\"";
				}
				attr = attr->Next();
			}

			/*if(style.size())
			{
				buffer += " style=\"";
				for(int32 i=0; i < style.size(); i++)
				{
					buffer+= style.key(i) + ": " + style[i] + "; ";
				}
				buffer += "\"";
			}*/

			text = e->GetText();
			child = e->FirstChildElement();

			tmp = "";
			if(child) child->render(tmp);

			if((text && *text) || tmp.size() || e->ValueStr() == "script" || e->ValueStr() == "style")
                        {
				buffer += ">";
                                if(text && *text) buffer += text;
				buffer += tmp + "</" + e->Value() + ">";
                        }
                        else
                        {
                                buffer += " />";
                        }
                }
        }
        while(e = e->NextSiblingElement());
}
