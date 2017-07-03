#include "stdafx.h"
#include "MyTimer.h"

namespace DXMapEditer
{
	MyTimer::MyTimer(void) : _bUseQPF(false)
		, _fElapsedTime(0.f)
		, _llQPFTicksPerSec(0)
		, _llLastElapsedTime(0)
	{
	}

	MyTimer::~MyTimer(void)
	{
	}

	void MyTimer::Init()
	{
		LARGE_INTEGER qwTicksPerSec, qwTime;

		_bUseQPF = (bool)(QueryPerformanceFrequency(
			&qwTicksPerSec) != 0);

		if (!_bUseQPF)	return;

		_llQPFTicksPerSec = qwTicksPerSec.QuadPart;

		QueryPerformanceCounter(&qwTime);
		_llLastElapsedTime = qwTime.QuadPart;
	}

	void MyTimer::ProcessTime()
	{
		if (!_bUseQPF)
		{
			return;
		}

		LARGE_INTEGER qwTime;
		QueryPerformanceCounter(&qwTime);

		_fElapsedTime = (float)
			((double)(qwTime.QuadPart - _llLastElapsedTime)
				/ (double)_llQPFTicksPerSec);

		_llLastElapsedTime = qwTime.QuadPart;
	}
}