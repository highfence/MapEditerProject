#pragma once

class InputLayer
{
public:
	InputLayer();
	InputLayer(const InputLayer&);
	~InputLayer();

	void Initialize();

	void KeyDown(unsigned int);
	void KeyUp(unsigned int);

	bool IsKeyDown(unsigned int);

private:
	bool m_keys[KeyNumber];
};