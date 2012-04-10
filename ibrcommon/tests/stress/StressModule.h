/*
 * StressModule.h
 *
 *  Created on: 12.11.2010
 *      Author: morgenro
 */

#ifndef STRESSMODULE_H_
#define STRESSMODULE_H_

class StressModule
{
public:
	virtual ~StressModule() {};

	virtual void stage1() = 0;
	virtual void stage2() = 0;
	virtual void stage3() = 0;

	virtual bool check() = 0;
};

#endif /* STRESSMODULE_H_ */
