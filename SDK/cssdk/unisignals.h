#ifndef INCLUDE_UNISIGNALS_H
#define INCLUDE_UNISIGNALS_H

class CUnifiedSignals
{
public:
	CUnifiedSignals()
	{
		m_flSignal = 0;
		m_flState = 0;
	}
public:
	void Update()
	{
		m_flState = m_flSignal;
		m_flSignal = 0;
	}
	void Signal(int flags) { m_flSignal |= flags; }
	int GetSignal() const { return m_flSignal; }
	int GetState() const { return m_flState; }
public:
	int m_flSignal;
	int m_flState;
};

#endif