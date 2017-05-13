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
	int i;

	// ��� Ű�� �������� ���� ���·� �ʱ�ȭ.
	for (i = 0; i < 256; i++)
	{
		m_keys[i] = false;
	}

	return;
}

void InputLayer::KeyDown(unsigned int input)
{
	// Ű�� ������ ���, true�� �ٲپ��ش�.
	m_keys[input] = true;
	return;
}

void InputLayer::KeyUp(unsigned int input)
{
	// Ű�� ������ ���, false�� �ٲپ��ش�.
	m_keys[input] = false;
	return;
}

bool InputLayer::IsKeyDown(unsigned int key)
{
	// ���ڿ� �ش��ϴ� Ű�� ���ȴ��� ��ȯ���ش�.
	return m_keys[key];
}