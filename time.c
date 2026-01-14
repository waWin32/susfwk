// time.c
//
#include "coreframe.h"
#include "include/susfwk/core.h"
#include "include/susfwk/time.h"

// --------------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 28159)

// Get the current time
sus_uint64_t SUSAPI sus_time() {
	return (sus_uint64_t)GetTickCount64();
}
// Get the current time
sus_uint32_t SUSAPI sus_time32() {
	return (sus_uint64_t)GetTickCount();
}

#pragma warning(pop)

// --------------------------------------------------------------------------------------

// Start the timer
VOID SUSAPI sus_timer_start(_In_ SUS_LPTIMER timer)
{
	QueryPerformanceFrequency(&timer->frequency);
	QueryPerformanceCounter(&timer->state.lastTime);
	timer->invFrequency = 1.0f / (sus_float_t)timer->frequency.QuadPart;
	timer->state = (struct sus_timer_state){ 0 };
}
// Reset the timer
VOID SUSAPI sus_timer_stop(SUS_LPTIMER timer)
{
	SUS_ASSERT(timer);
	timer->state.lastTime = timer->frequency = (LARGE_INTEGER){ 0 };
	timer->state.delta = timer->invFrequency = 0.0f;
}
// Update timer - an event has occurred
VOID SUSAPI sus_timer_update(SUS_LPTIMER timer)
{
	SUS_ASSERT(timer);
	LARGE_INTEGER currentTime; QueryPerformanceCounter(&currentTime);
	timer->state.delta = (currentTime.QuadPart - timer->state.lastTime.QuadPart) * timer->invFrequency;
	timer->state.lastTime = currentTime;
	timer->state.frameCount++;
	sus_uint32_t curr32 = sus_time32();
	if (timer->state.second <= curr32) {
		timer->state.FPS = timer->state.frameCount;
		timer->state.frameCount = 0;
		timer->state.second = curr32 + 1000;
	}
}

// --------------------------------------------------------------------------------------

