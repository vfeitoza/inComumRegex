#include <iostream>
#include <string>

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


int main(int argc, char **argv)
{
	string url, urlf, id, domain;
	string::size_type x;

	if(argv[1][0] == '-' && argv[1][1] == 'v' ) {
		cout << "inComum 0.3.2" << endl;
		_exit(0);
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

