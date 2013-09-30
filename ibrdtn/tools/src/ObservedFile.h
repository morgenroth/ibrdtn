/*
 * ObservedFile.h
 *
 *  Created on: Sep 20, 2013
 *      Author: goltzsch
 */
#include <vector>
#include <list>
#include <string>

#ifndef OBSERVEDFILE_H_
#define OBSERVEDFILE_H_


using namespace std;

class ObservedFile
{
public:
	ObservedFile();
	virtual ~ObservedFile();

	virtual int getFiles(list<ObservedFile*> files) = 0;
	virtual string getPath() = 0;
	virtual bool exists() = 0;
	virtual string getBasename() = 0;
	virtual size_t size() = 0;
	virtual bool isSystem() = 0;
	virtual bool isDirectory() = 0;

	virtual unsigned char* getHash() = 0;


	bool operator==(ObservedFile* other);
	void log();

	static void setConfigImgPath(string path);
	static void setConfigRounds(size_t rounds);
protected:
	bool lastHashesEqual( size_t n );
	std::vector<unsigned char*> _hashes;
	size_t _last_sent;

	static string _conf_imgpath;
	static size_t _conf_rounds;
	static bool _conf_badclock;
};



#endif /* OBSERVEDFILE_H_ */
