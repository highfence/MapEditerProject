#pragma once

// ������ �������� MyTimer.

class MyTimer
{
public :

	MyTimer(void);
	virtual ~MyTimer(void);

	inline float GetElapsedTime() const
	{
		return m_fElapsedTime;
	};

	void Init();
	void ProcessTime();

private :

	bool		m_bUseQPF;
	float		m_fElapsedTime;
	LONGLONG	m_llQPFTicksPerSec;
	LONGLONG	m_llLastElapsedTime;
};