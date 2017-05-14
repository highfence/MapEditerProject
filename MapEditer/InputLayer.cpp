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
			// ���� Ű�Է��� �ִ� ���.
			if (m_ByKey[i] & 0x80)
			{
				// �� �� Ű�Է��� �����ٸ� ���ۻ��·� ������ش� (PUSHKEY)
				if (!m_OldKey[i])
				{
					m_OldKey[i] = 1;
					m_ByKey[i] |= 0x40;
				}
				// �ƴ϶�� �׳� ������ �ִ� �� (HOLDKEY)
			}
			// Ű�Է��� ���� ���� ���.
			else
			{
				// �� �� Ű�Է��� �����ٸ� ��� ���·� ������ش�. (PULLKEY)
				if (m_OldKey[i])
				{
					m_OldKey[i] = 0;
					m_ByKey[i] = 0x20;
				}
				// �ƴ϶�� �׳� �ȴ����� �ִ� ����.
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
	// ���ڿ� �ش��ϴ� Ű�� ���ȴ��� ��ȯ���ش�.
	return (m_ByKey[key] & HOLDKEY);
}