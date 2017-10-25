#include "libsx.h"
#include "tokenize.h"

char isOp(char c)
{
	if(c == ',' || c == '(' || c == ')' || c == '[' || c == ']' || c == '*' || c == '{' || c == '}') return 2;
	if(c == '!' || c == '~' || c == '#' || c == '@' || c == '$' || c == '%' || c == '^' || c == '&' || c == '-' || c == '+' || c == '=' || c == '\\' || c == '|' || c == '/' || c == '?' || c == '<' || c == '>' || c == '.' || c == ':') return 1;
	return false;
}

bool tokenize(const char* pszStr, STRINGS &tokens, bool ignore_newline)
{
        string token;

        char delimiters[3][2] =
        {
        { '"', '"' },
        { '\'', '\'' },
        { '`', '`' } };
        int delimiter_active = -1;

        for (long i = 0; pszStr[i]; i++)
        {
                if (delimiter_active != -1)
                {
                        if(pszStr[i] == '\\')
                        {
                                token += pszStr[++i];
                                continue;
                        }
                        else if (pszStr[i] == delimiters[delimiter_active][1])
                        {
                                if(pszStr[i+1] != delimiters[delimiter_active][1])
                                {
                                        switch(delimiters[delimiter_active][1])
                                        {
                                        case '`':
                                                tokens.push_back(token, string_field);
                                                break;
                                        case '"':
                                                tokens.push_back(token, string_text);
                                                break;
					case '\'':
						tokens.push_back(token, string_char);
						break;
                                        default:
throw "unknown delimiter";
						tokens.push_back(token, string_none);
                                        }
                                        token = "";
                                        delimiter_active = -1;
                                        continue;
                                }
                                i++;

                        }
                        token += pszStr[i];
                        continue;
                }

                for (int d = 0; d < 3; d++)
                {
                        if (pszStr[i] == delimiters[d][0])
                        {
                                delimiter_active = d;
                                break;
                        }
                }

                if (delimiter_active != -1)
                        continue;

                if(pszStr[i] == '-' && pszStr[i+1] == '-' )
                {
                        while(pszStr[i] && pszStr[i] != '\n') i++;
                        continue;
                }

                if(pszStr[i] == '/' && pszStr[i+1] == '*' )
                {
                        while(pszStr[i] && !(pszStr[i] == '*' && pszStr[i+1] == '/')) i++;
                        if(pszStr[i]) i++;
                        continue;
                }

		if(pszStr[i] == '/' && pszStr[i+1] == '/' )
		{
			while(pszStr[i] && (pszStr[i] != 0x0A && pszStr[i+1] != 0x0D)) i++;
			continue;
		}

		if(!token.size() && pszStr[i] >= '0' && pszStr[i] <= '9')
		{
			if(pszStr[i+1] == 'x')
			{
				i+= 2;
				while(pszStr[i] && ((pszStr[i] >= '0' && pszStr[i] <= '9') || (pszStr[i] >= 'a' && pszStr[i] <= 'f') || (pszStr[i] >= 'a' && pszStr[i] <= 'f')  ) )
				{
					token += pszStr[i++];
				}
			}
			else if(pszStr[i+1] == 'b')
			{
				i+= 2;
				token += pszStr[i++];
				while(pszStr[i] && (pszStr[i] == '0' || pszStr[i] == '1'))
				{
					token += pszStr[i++];
				}
			}
			else if(pszStr[i] == '0' && pszStr[i+1] >= '0' && pszStr[i+1] <= '7')
			{
				i+=2;
				while(pszStr[i] && pszStr[i] >= '0' && pszStr[i] <= '7')
				{
					token += pszStr[i++];
				}
			}
			else
			{
				while(pszStr[i] && pszStr[i] >= '0' && pszStr[i] <= '9')
				{
					token += pszStr[i++];
				}
			}
			i--;
			tokens.push_back(token, string_number);
			token = "";
		}
		else if(isOp(pszStr[i]))
		{
			if(token.length())
                        {
                                tokens.push_back(token);
                        }
                        token = pszStr[i];

			if(pszStr[i] != '[' && pszStr[i] != '(' && pszStr[i] != '{' && pszStr[i] != ']' && pszStr[i] != ')' && pszStr[i] != '}')
			{
				while(isOp(pszStr[i+1]) && pszStr[i+1] != ']' && pszStr[i+1] != ')' && pszStr[i+1] != '}' && pszStr[i+1] != '[' && pszStr[i+1] != '(' && pszStr[i+1] != '{')
				{
					token += pszStr[++i];
				}
			}

                        tokens.push_back(token);
                        token = "";
		}
                else switch (pszStr[i])
                {
		case '\n':
			if (token.length())
                        {
                                if (token.length())
                                {
                                        tokens.push_back(token);
					if(!ignore_newline) tokens.push_back("\n");
                                }
                                token = "";
                        }
			break;
                case ' ':
                case '\t':
                case '\r':
                        if (token.length())
                        {
                                if (token.length())
                                {
                                        tokens.push_back(token);
                                }
                                token = "";
                        }
                        break;
                case ';':
                //case '.':
                        if (token.length())
                        {
                                tokens.push_back(token);
                        }
                        token = pszStr[i];
                        tokens.push_back(token);
                        token = "";
                        break;
                /*case '#':
                        {
                                int64 nLength = boost::lexical_cast<int64>(token);

                                token.resize(nLength);
                                i++;
                                if(pszStr[i + nLength] != '#')
                                {
                                        throw std::string("Improper binary encoding");
                                }
                                memcpy((void*)token.c_str(), (void*)(pszStr+i), nLength);
                                tokens.push_back(token);
                                token = "";

                                i += nLength;
                        }
                        break;*/
                default:
                        token += tolower(pszStr[i]);
                        break;
                }
        }
        if(token.size()) tokens.push_back(token);

        //error(string("Tokens: ") + boost::lexical_cast<string>(tokens.size()));

        /*for(long i=0; i < tokens.size(); i++)
        {
                error(tokens[i]);
        }*/
        return true;
}
