#pragma once

// ������ �������� MyTimer.

class MyTimer
{
public :

	MyTimer(void);
	virtual ~MyTimer(void);

	inline float GetElapsedTime() const
	{
		return _fElapsedTime;
	};

	void Init();
	void ProcessTime();

private :

	bool		_bUseQPF;
	float		_fElapsedTime;
	LONGLONG	_llQPFTicksPerSec;
	LONGLONG	_llLastElapsedTime;
};
