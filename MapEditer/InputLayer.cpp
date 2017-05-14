#include "stdafx.h"
#include "InputLayer.h"

InputLayer::InputLayer()
{
}


InputLayer::InputLayer(const InputLayer& other)
{
}


InputLayer::~InputLayer()
{
}


void InputLayer::Initialize()
{
}

void InputLayer::Update()
{
	if (GetKeyboardState(m_ByKey))
	{
		for (int i = 0; i < KeyNumber; ++i)
		{
			// 현재 키입력이 있는 경우.
			if (m_ByKey[i] & 0x80)
			{
				// 그 전 키입력이 없었다면 시작상태로 만들어준다 (PUSHKEY)
				if (!m_OldKey[i])
				{
					m_OldKey[i] = 1;
					m_ByKey[i] |= 0x40;
				}
				// 아니라면 그냥 누르고 있는 중 (HOLDKEY)
			}
			// 키입력이 현재 없는 경우.
			else
			{
				// 그 전 키입력이 없었다면 띄는 상태로 만들어준다. (PULLKEY)
				if (m_OldKey[i])
				{
					m_OldKey[i] = 0;
					m_ByKey[i] = 0x20;
				}
				// 아니라면 그냥 안누르고 있는 상태.
				else
				{
					m_ByKey[i] = 0x10;
				}
			}
		}
	}
}

bool InputLayer::IsKeyDown(unsigned int key)
{
	// 인자에 해당하는 키가 눌렸는지 반환해준다.
	return (m_ByKey[key] & HOLDKEY);
}