#ifndef SX_SST_H
#define SX_SST_H

#include <vector>
#include <map>
#include <boost/scoped_ptr.hpp>
#include "tokenize.h"
#include "tinyxml/tinyxml.h"

class sstCssNode;

//#define CSSMAP std::map<std::string, sstCssNode>

class SSTELEMENTS : public std::vector<TiXmlElement*>
{
public:
	SSTELEMENTS(){}
	SSTELEMENTS(TiXmlElement* e){if(e) push_back(e);}
	bool operator!(){ return !empty(); }
	void copy(SSTELEMENTS* dest)
	{
		dest->clear();
		for(long i=0; i < size(); i++)
		{
			dest->push_back(at(i));
		}
	}
private:
};


#define ATTRIBUTES std::map<TiXmlElement*, std::map<std::string, std::string> >
typedef ARRAY<sstCssNode> CSSMAP;

class sstCssNode
{
public:
	sstCssNode();
	~sstCssNode();
	sstCssNode* operator[](std::string key);
	sstCssNode* get() { return this; }
	void operator=(std::string v) { value = v; }
	std::string setValue(std::string v) { value = v; }
	std::string getValue() { return value; }
	void dump(std::string &buffer, long depth=0);
	CSSMAP* map() { return &cssMap; }
private:
	std::string value;
	CSSMAP cssMap;
};

class SST
{
public:
	SST();
	~SST();
	void addXml(std::string &buffer);
	void addStyleSheet(std::string &buffer);
	void render(std::string &buffer);
	void applyClasses();
	void applySelector(TiXmlElement* element, sstCssNode *cssRoot=NULL);
	void applySelector(SSTELEMENTS elements, sstCssNode *cssRoot=NULL);
	SSTELEMENTS * selector(const char* pszSearch, SSTELEMENTS parents, SSTELEMENTS *result);
	SSTELEMENTS selector(const char* pszSearch, SSTELEMENTS parents);
	TiXmlElement* selector(const char* pszSearch, TiXmlElement* parent);
	SSTELEMENTS* selectorDescendant(STRINGS &bits, long pos, SSTELEMENTS parents, SSTELEMENTS *result);
	SSTELEMENTS* getElementsByAttr(const char* attr, const char* value, SSTELEMENTS parents, SSTELEMENTS *result);
	SSTELEMENTS* getElementsByTag(const char* tag, SSTELEMENTS parents, SSTELEMENTS *result);
	void applyStyles();
private:
	std::string xml;
	std::string css;

	sstCssNode cssRoot;
	ATTRIBUTES xmlStyle;
	TiXmlDocument doc;
};

#endif
