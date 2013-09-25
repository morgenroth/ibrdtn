/*
 * ObservedFile.h
 *
 *  Created on: Sep 20, 2013
 *      Author: goltzsch
 */

#ifndef OBSERVEDFILE_H_
#define OBSERVEDFILE_H_

#include <vector>
#include <list>
#include <string>

using namespace std;

template <class T>
class ObservedFile
{
public:
	ObservedFile(std::string path);
	virtual ~ObservedFile();

	int getFiles(list<T> files);
	string getPath();
	bool exists();
	string getBasename();
	size_t size();
	bool isDirectory();

	time_t getLastTimestamp();
	bool lastSizesEqual( size_t n );
	void send();
	void addSize();
	size_t getLastSent();

private:
	T _file;
	size_t _last_sent;
	std::vector<size_t> _sizes;
};



#endif /* OBSERVEDFILE_H_ */
