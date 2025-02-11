#ifndef SIMPLE_TASK_PROCESSOR_H
#define SIMPLE_TASK_PROCESSOR_H

#include "CesiumAsync/AsyncSystem.h"

class SimpleTaskProcessor : public CesiumAsync::ITaskProcessor {
	virtual void startTask(std::function<void()> f) override { f(); }
};

#endif // !SIMPLE_TASK_PROCESSOR_H
