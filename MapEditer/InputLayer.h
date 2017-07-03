#pragma once
#include "KeyState.h"

namespace DXMapEditer
{
	class InputLayer
	{
	public:
		InputLayer();
		InputLayer(const InputLayer&);
		~InputLayer();

		void Initialize();

		void Update();

		bool IsKeyDown(unsigned int);

	private:

		BYTE m_ByKey[KeyNumber];
		BYTE m_OldKey[KeyNumber];
	};
}