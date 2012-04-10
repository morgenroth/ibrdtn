/*
 * StressBLOB.h
 *
 *  Created on: 12.11.2010
 *      Author: morgenro
 */

#include <StressModule.h>
#include <ibrcommon/data/BLOB.h>
#include <ibrcommon/thread/Thread.h>
#include <list>

#ifndef STRESSBLOB_H_
#define STRESSBLOB_H_

class StressBLOB : public StressModule
{
public:
	StressBLOB();
	virtual ~StressBLOB();

	void stage1();
	void stage2();
	void stage3();

	bool check();

private:
	class BLOBWorker : public ibrcommon::JoinableThread
	{
	public:
		BLOBWorker(const size_t count, const size_t size);
		virtual ~BLOBWorker();

		void run();
		bool check();
		void __cancellation();

	private:
		size_t _count;
		size_t _size;
		std::string teststr;

		std::list<ibrcommon::BLOB::Reference> _refs;
	};

	std::list<BLOBWorker*> _worker;
};

#endif /* STRESSBLOB_H_ */
