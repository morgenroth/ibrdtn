/*
 * StressModule.h
 *
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
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
