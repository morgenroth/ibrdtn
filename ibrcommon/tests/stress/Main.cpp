/**
 * This programm tests some special mechanisms for stress.
 */

#include <StressBLOB.h>
#include <ibrcommon/data/BLOB.h>
#include <list>

int main()
{
	ibrcommon::File tmppath("./tmp");
	if (!tmppath.exists()) ibrcommon::File::createDirectory( tmppath );

	ibrcommon::BLOB::changeProvider(new ibrcommon::FileBLOBProvider(tmppath), true);

	std::list<StressModule*> list;
	list.push_back(new StressBLOB());

	for (std::list<StressModule*>::const_iterator iter = list.begin(); iter != list.end(); iter++)
	{
		(*iter)->stage1();
	}

	for (std::list<StressModule*>::const_iterator iter = list.begin(); iter != list.end(); iter++)
	{
		(*iter)->stage2();
	}

	for (std::list<StressModule*>::const_iterator iter = list.begin(); iter != list.end(); iter++)
	{
		(*iter)->stage3();
	}

	bool err = false;
	for (std::list<StressModule*>::const_iterator iter = list.begin(); iter != list.end(); iter++)
	{
		if (!(*iter)->check()) err = true;
	}

	for (std::list<StressModule*>::const_iterator iter = list.begin(); iter != list.end(); iter++)
	{
		delete (*iter);
	}

	if (err) return -1;

	return 0;
}
