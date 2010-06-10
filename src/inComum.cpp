#include <iostream>
#include <regex.h>
#include <string>
#include <vector>

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
* verion 0.3.2
*
*/

using namespace std;

string get_var(const string url,const string var); //get some var from url
string get_path(const string url, const char removeQuery); //path only (optionally remove query string)
string get_domain(const string url); //get domain only (remove path)
string regexMatch(string er, string line); //ER's
void stringExplode(string str, string separator, vector<string>* results); //explode string
string getFileName(string url); //get filename for url

int main(int argc, char **argv)
{
	string url, urlf, id, domain;
	string::size_type x;

	if (argc > 1) {
		if(argv[1][0] == '-' && argv[1][1] == 'v' ) {
			cout << "inComum 0.3.2" << endl;
			_exit(0);
		}
	}

	while(getline(std::cin, url)){ // main loop receive all redirecting URL

		x = url.find_first_of(" ");
		if(x != string::npos){
			url = url.erase(x);
		}
		domain = get_domain(url);

		//https is just tunneling (can't cache) - don't cache ftp either
		if(url.substr(0,8) == "https://" || url.substr(0,6) != "ftp://"){
			cout << url << endl;
			continue;
		}

		//squid may send a blank line to exit
		if(url.empty() || url.substr(0,7) != "http://"){
			return 0;
		}

		//youtube plugin
		if ( ( (domain.find("googlevideo.com/") != string::npos) || (domain.find("youtube.com/") != string::npos) ) &&
		( (url.find("/get_video?") != string::npos) || (url.find("/videoplayback?") != string::npos) || (url.find("/videoplay?") != string::npos) ) &&
		( (url.find("id=") != string::npos) || (url.find("video_id=") != string::npos) ) &&
		( url.find("begin=") == string::npos ) )
		{
			if(url.find("noflv=") != string::npos && url.find("ptk=") == string::npos){ //(noflv AND !ptk) => redirecting URL
				urlf = url;
			}else{
				id = get_var(url, "id");
				if(id.length() == 0){
					id = get_var(url, "video_id");
				}
				urlf = "http://flv.youtube.inComum/?id="+id+"&quality="+get_var(url, "fmt")+get_var(url, "itag")+"&redirect_counter="+get_var(url, "redirect_counter")+get_var(url, "st");
			}

		//orkut img plugin
		}else if(domain.find(".orkut.com/") != string::npos && domain.find("http://img") != string::npos){
			urlf = "http://img.orkut.inComum/"+get_path(url,'N');

		//orkut static plugin
		}else if(domain.find(".orkut.com/") != string::npos && domain.find("http://static") != string::npos ){
			urlf = "http://static.orkut.inComum/"+get_path(url,'N');

		//avast plugin
		}else if(domain.find(".avast.com/") != string::npos && domain.find("http://download") != string::npos){
			urlf = "http://download.avast.inComum/"+get_path(url,'N');

		//vimeo plugin
		}else if(domain.find("http://av.vimeo.com/") != string::npos && url.find("?token=") != string::npos){
			urlf = "http://vimeo.inComum/"+get_path(url,'Y');

		//blip.tv plugin
		}else if(domain.find("blip.tv") != string::npos && domain.find("video") != string::npos){
			urlf = "http://bliptv.inComum/"+get_path(url,'Y');

		//globo plugin
		}else if(domain.find("flashvideo.globo.com") != string::npos && url.find("mp4") != string::npos){
			urlf = "http://flashvideo.globo.inComum/"+get_path(url,'Y');

		//msn catalog videos plugin
		}else if(domain.find("catalog.video.msn.com/") != string::npos && url.find("share") != string::npos){
			urlf = "http://catalog.msn.inComum/"+get_path(url,'N');

		//redtube plugin
		}else if((domain.find(".cdn.redtube.com") != string::npos) && 
		         (regexMatch("videos-[0-9]{3,4}\\.cdn\\.redtube\\.com\\/s\\/[0-9]{7}\\/[a-zA-Z0-9]{9,12}\\.flv", url ) != "")){
			urlf = "http://videos.redtube.inComum/"+ getFileName(url) +"?rs="+ get_var(url, "rs");

		//pornhub plugin
		}else if((regexMatch("nyc-v[0-9]{1,4}\\.pornhub\\.com\\/dl\\/", url ) != "") && 
		         (regexMatch("(flv|mp4)", url ) != "") && (regexMatch("start=", url ) == "")){
			urlf = "http://videos.pornhub.inComum/"+ getFileName(url);
			
		//pornotube plugin
		}else if((regexMatch("video[0-9]{1,2}\\.pornotube\\.com\\/", url ) != "") && (regexMatch("(flv|mp4)", url ) != "")){
			urlf = "http://videos.pornotube.inComum/"+ getFileName(url);

		//else, return the same URL
		}else{
			urlf = url;
		}

		//send urlf back to squid
		cout << urlf << endl;
	}
	return 0;
}

/* return the value of a URL var */
string get_var(const string url,const string var)
{
	string par, v, valor, vars;

	if(url.find("?") != string::npos){
		vars = url.substr(url.find("?")+1);
	}

	while(!vars.empty()){
		par = vars.substr(vars.find_last_of("&")+1);
		v = par.substr(0,par.find_first_of("="));
		valor = par.substr(par.find_first_of("=")+1);
		if(v == var){
			return par.substr(par.find_first_of("=")+1);
		}
		if(vars.find_last_of("&") != string::npos){
			vars.erase(vars.find_last_of("&"));
		}else{
			vars.erase();
		}
	}
	return "";
}

/* return URL path */
string get_path(const string url, const char removeQuery)
{
	string temp, path;
	string::size_type x;

	temp = url.substr(7);
	x = temp.find_first_of("/");
	if(x != string::npos){
		path = temp.substr(x+1);
		if(removeQuery == 'Y'){
			x = path.find_first_of("?");
			if(x != string::npos){
				path = path.erase(x);
			}
		}
		return path;
	}else{
		return "";
	}
}

/* return URL domain */
string get_domain(const string url)
{
	string retorno;
	string::size_type x;

	x = url.find_first_of("/", 7);
	if(x != string::npos){
		return url.substr(0,x+1);
	}else{
		return url;
	}

}

// usando ER's para facilitar a vida [risos]
string regexMatch(string er, string line) 
{
    int error;
    regmatch_t match;
    regex_t reg;
    if ((regcomp(&reg, er.c_str(), REG_EXTENDED | REG_NEWLINE)) == 0) {
        error = regexec(&reg, line.c_str(), 1, &match, 0);
        if (error == 0) {
            return line.substr(match.rm_so, match.rm_eo - match.rm_so);
        } else {
            return "";
        }
    } else {
        return "";
    }
}

// quebramos a string em vetores (arrais)
void stringExplode(string str, string separator, vector<string>* results)
{
    int found;
    found = str.find_first_of(separator);
    while(found != string::npos){
        if(found > 0){
            results->push_back(str.substr(0,found));
        }
        str = str.substr(found+1);
        found = str.find_first_of(separator);
    }
    if(str.length() > 0){
        results->push_back(str);
    }
}

// pegarmos apenas o nome do arquivo
string getFileName(string url)
{
    vector<string> resultado;
    if (url.find("?") != string::npos) {
        stringExplode(url, "?", &resultado);
        stringExplode(resultado.at(resultado.size() - 2), "/", &resultado);

        return resultado.at(resultado.size() -1);
    } else {
        stringExplode(url, "/", &resultado);

        return resultado.at(resultado.size() -1);
    }
}
