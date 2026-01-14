// time.h
//
#ifndef _SUS_TIME_
#define _SUS_TIME_

//////////////////////////////////////////////////////////////////////////////////////////
//									Working with time									//
//////////////////////////////////////////////////////////////////////////////////////////

// --------------------------------------------------------------------------------------

// get the current time
sus_uint64_t SUSAPI sus_time();
// get the current time
sus_uint32_t SUSAPI sus_time32();
// Get the current time in seconds
SUS_INLINE sus_double_t SUSAPI sus_timed() { return (sus_double_t)sus_time() / 1000.0f; }
// Get the current time in seconds
SUS_INLINE sus_float_t SUSAPI sus_timef() { return (sus_float_t)sus_time32() / 1000.0f; }

// --------------------------------------------------------------------------------------

// Important timer status information
typedef struct sus_timer_state {
	sus_float_t		delta;
	sus_uint16_t	FPS;
	sus_uint16_t	frameCount;
	sus_uint32_t	second;
	LARGE_INTEGER	lastTime;
} SUS_TIMER_STATE;
// The structure of the time delta
typedef struct sus_timer {
	LARGE_INTEGER	frequency;
	sus_float_t		invFrequency;
	SUS_TIMER_STATE state;
} SUS_TIMER, *SUS_LPTIMER;

// Start the timer
VOID SUSAPI sus_timer_start(_In_ SUS_LPTIMER timer);
// Reset the timer
VOID SUSAPI sus_timer_stop(SUS_LPTIMER timer);
// Update timer - an event has occurred
VOID SUSAPI sus_timer_update(SUS_LPTIMER timer);

// Get the delta from the time
SUS_INLINE sus_float_t SUSAPI sus_timer_delta(SUS_TIMER timer) {
	return (sus_float_t)timer.state.delta;
}
// Get FPS 
SUS_INLINE sus_uint16_t SUSAPI sus_timer_fps(SUS_TIMER timer) {
	return (sus_uint16_t)timer.state.FPS;
}

// --------------------------------------------------------------------------------------

#endif /* !_SUS_TIME_ */