/*
 * BSD License
 *
Copyright (c) 2010, Luciano Pinheiro.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
3. The names of developers may not be used to endorse or promote
products derived from this software without specific prior written
permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*
* to compile, use following line:
* $ g++ -Wall -o "inComum" "inComum.cpp"
*
* verion 0.4.b
*
*/

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <regex.h>

using namespace std;

//Functions
std::vector<std::string> read_file(char file[20]);
std::vector<std::string> StringExplode(std::string str, std::string separator);
std::vector< std::vector<std::string> > serialize_rules(std::vector<std::string> content);
std::string get_var(const std::string url,const std::string var);
std::string get_path(const std::string url, const char removeQuery);
std::string get_domain(const std::string url);
std::string mount_query(std::string url, std::string rules);
std::string clean_domain_regex(std::string regex);
std::string remove_char(std::string str,std::string c);
bool regexMatch(string line, string er) ;

//Vars
std::vector< std::vector<std::string> > rules;

int main(int argc, char **argv){
	char conf_file[]="incomum.conf";
    rules=serialize_rules(read_file(conf_file));
    std::string url, domain, urlf;
    string::size_type x;
    
	if(argc > 1){
		if(argv[1][0] == '-' && argv[1][1] == 'v' ) {
			cout << "inComum 0.4.0" << endl;
			return 0;
		}
	}
	
    while (std::cin){
		std::getline(std::cin, url);
		x = url.find_first_of(" ");
		
		if(x != string::npos){
			url = url.erase(x);
		}

		//https is just tunneling (can't cache) - don't cache ftp either
		if(url.substr(0,8) == "https://" || url.substr(0,6) == "ftp://"){
			cout << url << endl;
			continue;
		}

		//squid may send a blank line to exit
		if(url.empty() || url.substr(0,7) != "http://"){
			cout << url << endl;
			return 0;
		}
        
        urlf=url;
		for (unsigned int i=0; i < rules.size(); i++)
		{
			if ((regexMatch(url, rules[i][1])) &&
			    (regexMatch(url, rules[i][2]) || (rules[i][2] == ".")) &&
			   (!regexMatch(url, rules[i][3])  || (rules[i][3] == "."))){
				urlf = "http://"+clean_domain_regex(rules[i][1])+".inComum/";
				
				if(rules[i][4].find("get_path") != std::string::npos){
					urlf = urlf + get_path(url, 'Y');
				}else if(rules[i][4].find("query") != std::string::npos){
					urlf=urlf+mount_query(url,rules[i][4]);
				}
			}
	    }
	    std::cout << urlf << std::endl;
    }
}

std::string mount_query(std::string url, std::string rules){
	std::string query;
	std::vector<string> vec_rules;
	vec_rules=StringExplode(rules.substr(rules.find_first_of("=")+1),"|");
	for (unsigned int i=0; i<vec_rules.size(); i++){
		if (url.find(vec_rules[i]) != std::string::npos){
			query=query+vec_rules[i]+"="+get_var(url,vec_rules[i])+"&";
	    }
	}
	
	return query.substr(0,query.size()-1);
}

std::vector<std::string> read_file(char file[20]){
  std::string line;
  std::ifstream myfile (file);
  std::vector<std::string> content;
  if (myfile.is_open()){
    while (! myfile.eof() ) {
      getline (myfile,line);
      if ( ! regexMatch(line,"^$")) {
	    content.push_back(line);
	  }
    }
    myfile.close();
  }else{
	std::cout << "Unable to open file";
  }
  return content;
}

std::vector< std::vector<std::string> > serialize_rules(std::vector<std::string> content){
  std::vector< std::vector<std::string> > serialized_rules;
  for (unsigned int i=0; i<content.size(); i++) {
	  serialized_rules.push_back(StringExplode(content[i], " "));
  }
  return serialized_rules;
}

std::vector<std::string> StringExplode(std::string str, std::string separator){
    std::string::size_type found;
    std::vector<std::string> results;
    found = str.find_first_of(separator);
    while(found != std::string::npos){
        if(found > 0){
            results.push_back(str.substr(0,found));
        }
        str = str.substr(found+1);
        found = str.find_first_of(separator);
    }
    if(str.length() > 0){
        results.push_back(str);
    }
    return results;
}

/* return the value of a URL var */
std::string get_var(const std::string url,const std::string var){
	std::string par, v, valor, vars;

	if(url.find("?") != std::string::npos){
		vars = url.substr(url.find("?")+1);
	}

	while(!vars.empty()){
		par = vars.substr(vars.find_last_of("&")+1);
		v = par.substr(0,par.find_first_of("="));
		valor = par.substr(par.find_first_of("=")+1);
		if(v == var){
			return par.substr(par.find_first_of("=")+1);
		}
		if(vars.find_last_of("&") != std::string::npos){
			vars.erase(vars.find_last_of("&"));
		}else{
			vars.erase();
		}
	}
	return "";
}

/* return URL path */
std::string get_path(const std::string url, const char removeQuery){
	std::string temp, path;
	std::string::size_type x;

	temp = url.substr(7);
	x = temp.find_first_of("/");
	if(x != std::string::npos){
		path = temp.substr(x+1);
		if(removeQuery == 'Y'){
			x = path.find_first_of("?");
			if(x != std::string::npos){
				path = path.erase(x);
			}
		}
		return path;
	}else{
		return "";
	}
}

/* return URL domain */
std::string get_domain(const std::string url){
	std::string retorno;
	std::string::size_type x;

	x = url.find_first_of("/", 7);
	if(x != std::string::npos){
		return url.substr(0,x+1);
	}else{
		return url;
	}
}

std::string remove_char(std::string str,std::string c){
	std::string::size_type found;
	found = str.find_first_of(c);
    while(found != std::string::npos){
        str=(str.erase(found,1));
        found = str.find_first_of(c);
    }
    return str;
}

std::string clean_domain_regex(std::string regex){
	regex=remove_char(regex,".");
	regex=remove_char(regex,"*");
	regex=remove_char(regex,"\\");
	return regex;
}

bool regexMatch(string line, string er) {
    int error;
    regmatch_t match;
    regex_t reg;
    if ((regcomp(&reg, er.c_str(), REG_EXTENDED | REG_NEWLINE)) == 0) {
        error = regexec(&reg, line.c_str(), 1, &match, 0);
        if (error == 0) {
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}
