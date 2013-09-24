#include "ObservedFile.h"

template<typename T>
inline ObservedFile<T>::ObservedFile( std::string path )
		: _file(path),_last_sent(0)
{
}

template<typename T>
inline ObservedFile<T>::~ObservedFile()
{
}

template<typename T>
inline int ObservedFile<T>::getFiles( list<T> files )
{
	_file.getFiles(files);
}

template<typename T>
inline string ObservedFile<T>::getPath()
{
	return _file.getPath();
}

template<typename T>
inline bool ObservedFile<T>::exists()
{
	return _file.exists();
}

template<typename T>
inline time_t ObservedFile<T>::getLastTimestamp()
{
	return max(_file.lastmodify(), _file.laststatchange());
}

template<typename T>
inline string ObservedFile<T>::getBasename()
{
	return _file.getBasename();
}

template<typename T>
inline bool ObservedFile<T>::lastSizesEqual( size_t n )
{
	if (n > _sizes.size()) return false;

	for (size_t i = 1; i <= n; i++)
	{
		if (_sizes.at(_sizes.size() - i) != _sizes.at(_sizes.size() - i - 1)) return false;
	}
	return true;
}

template<typename T>
inline void ObservedFile<T>::send()
{
	time_t tm;
	_last_sent = time(&tm);
	_sizes.clear();
}

template<typename T>
inline void ObservedFile<T>::addSize()
{
	_sizes.push_back(_file.size());
}

template<typename T>
inline size_t ObservedFile<T>::getLastSent()
{
	return _last_sent;
}
