/*
 * ObservedFile.h
 *
 *  Created on: Sep 20, 2013
 *      Author: goltzsch
 */
#include <vector>
#include <list>
#include <string>
#include <string.h>
#include <iostream> //TODO weg

#ifndef OBSERVEDFILE_H_
#define OBSERVEDFILE_H_


using namespace std;

class ObservedFile
{
public:
	ObservedFile();
	virtual ~ObservedFile();

	virtual int getFiles(list<ObservedFile*>& files) = 0;
	virtual string getPath() = 0;
	virtual bool exists() = 0;
	virtual string getBasename() = 0;
	virtual size_t size() = 0;
	virtual bool isSystem() = 0;
	virtual bool isDirectory() = 0;

	virtual string getHash() = 0;

	void tick();
	void send();
	static bool compare(ObservedFile* a, ObservedFile* b);
	static void setConfigImgPath(string path);
	static void setConfigRounds(size_t rounds);
	static void setConfigBadclock(bool badclock);

	bool lastHashesEqual( size_t n );
protected:
	std::vector<string> _hashes;
	size_t _last_sent;

	static string _conf_imgpath;
	static size_t _conf_rounds;
	static bool _conf_badclock;
};



#endif /* OBSERVEDFILE_H_ */
