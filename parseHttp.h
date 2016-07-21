#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <map>
#include <sstream>
#include <fstream>
#include <boost/algorithm/string/trim.hpp>

std::string fromFile(const char* filename);

struct HttpMessage{
	//Request
	std::string type;
	std::string version;
	std::string path;
	std::string connection; //close
	std::string host;
	std::string userAgent;
	std::string accept;
	std::string acceptEncoding;
	std::string acceptLanguage;
	std::string referer;
	//Answer
	std::string contentLength; //len of message
	std::string contentType; // text/htmla
	std::string status;
	std::string dataToSend;
	std::string server;
	size_t dataSize = 0;
	std::string message;
	bool isFound = false;
}httpMessage;


std::string getType(const std::string& rawHeader){
	std::string type;
	std::istringstream rawstream(rawHeader);
	std::getline(rawstream, type);
	std::string::size_type npos = type.find(' ');
	type = boost::algorithm::trim_copy(type.substr(0, npos));
	return type;
}

std::string getPath(const std::string& rawHeader){
	std::string path;
	std::string::size_type nBegin, nEnd;
	nBegin = rawHeader.find('/');
	nEnd = rawHeader.find(' ', nBegin);
	if(nEnd - nBegin > 1){
		path = boost::algorithm::trim_copy( rawHeader.substr(nBegin + 1, nEnd-nBegin) );
	} else {
		path = boost::algorithm::trim_copy( rawHeader.substr(nBegin, nEnd-nBegin) );	
	}
	std::cout<<"path = "<<path<<std::endl;
	return path;
}

std::string getVersion(const std::string& rawHeader){
	std::string version;
	std::string::size_type nBegin, nEnd;
	nBegin = rawHeader.find("HTTP");
	nEnd = rawHeader.find('\r', nBegin);
	version = rawHeader.substr(nBegin, nEnd-nBegin);
	return version;
}

std::string getData(const std::string& rawHeader, 
					const char* from, 
					const char* to){
	std::string data;
	std::string::size_type nBegin, nEnd;
	nBegin = rawHeader.find(from);
	nEnd = rawHeader.find(to, nBegin);
	data = rawHeader.substr(nBegin, nEnd-nBegin);
	return data;
}
void parseHttpHeader(const std::string& rawHeader){

	//TYPE VERSION PATH	
	httpMessage.type = getType(rawHeader);
	httpMessage.version = getVersion(rawHeader);
	httpMessage.path = getPath(rawHeader);

	//Whether exists requested page or not
	if( (httpMessage.path == "index.html") || (httpMessage.path == "/") ){
		httpMessage.path = "index.html";
		httpMessage.isFound = true;
		httpMessage.dataToSend = fromFile( httpMessage.path.c_str() );
//		std::cout<<httpMessage.dataToSend<<std::endl;
	} else {
		httpMessage.isFound = false;
		httpMessage.dataToSend = fromFile("notfound.html");
//		std::cout<<httpMessage.dataToSend<<std::endl;
	}

	std::map<std::string, std::string> headers;
	
//	std::cout<<httpMessage.type<<std::endl;
	std::istringstream rawstream(rawHeader);
	std::string header;
	while (std::getline(rawstream, header) && header != "\r") {
	    std::string::size_type npos = header.find(':');
		
		if (npos != std::string::npos) {
			headers.insert(std::make_pair(
            boost::algorithm::trim_copy(header.substr(0, npos)), 
            boost::algorithm::trim_copy(header.substr(npos + 1))
			));
		}
	}

	//Other fields
	httpMessage.connection = headers["Connection"];
	httpMessage.host = headers["Host"];
	httpMessage.userAgent = headers["User-Agent"];
	httpMessage.accept = headers["Accept"];
	httpMessage.acceptEncoding = headers["Accept-Encoding"];
	httpMessage.acceptLanguage = headers["Accept-Language"];
	httpMessage.referer = headers["Referer"];
}

std::string fromFile(const char* filename){


	std::filebuf fileBuffer;
	fileBuffer.open(filename,std::ios::in);
	std::istream file(&fileBuffer);
	std::string fileData;
 
	while(file) {
		std::string str;
		std::getline(file, str);
		fileData = fileData + str + '\n';
		// Обработка строки str
	}
	return fileData;


/*
	using namespace std;
	std::string fileData;
	int length;
	char * buffer;
 
	ifstream is;
	is.open (filename, std::ifstream::binary );
 
	// get length of file:
	is.seekg (0, ios::end);
	length = is.tellg();
	is.seekg (0, ios::beg);
 
	// allocate memory:
	buffer = new char[length];
 
	// read data as a block:
	is.read (buffer,length);
	fileData = buffer;
	is.close();
	 
	delete[] buffer;
	return fileData;
*/
	
}


HttpMessage formAnswer(){
	httpMessage.dataSize = httpMessage.dataToSend.size();
	if(httpMessage.isFound){
		std::ostringstream num;
		num<<httpMessage.dataSize;
		
		httpMessage.status = "200 OK\r\n";
		httpMessage.connection = "Connection: close\r\n";
		httpMessage.contentType = "Content-Type: text/html\r\n";
		httpMessage.contentLength = "Content-Length: " + num.str() + "\r\n";

		httpMessage.message = httpMessage.version + ' ' +
							httpMessage.status +
							httpMessage.connection +
							httpMessage.contentType +
							httpMessage.contentLength +
							"\r\n" + httpMessage.dataToSend;
	//	std::cout<<httpMessage.message<<std::endl;
	//	std::cout<<httpMessage.message.size()<<std::endl;

	} else {
		httpMessage.status = "404 Not Found\r\n";
		httpMessage.contentType = "Content-Type: text/html\r\n";
		httpMessage.server = "Server: Dynamic Thread\r\n";

		httpMessage.message = httpMessage.version + ' ' +
							httpMessage.status +
							httpMessage.server +
							httpMessage.contentType +
							"\r\n" + httpMessage.dataToSend;
	//	std::cout<<httpMessage.message<<std::endl;
	//	std::cout<<httpMessage.message.size()<<std::endl;
	}
	return httpMessage;
}
