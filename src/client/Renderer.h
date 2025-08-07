#pragma once
#include "GameProtocol.h"

class Renderer
{
public:
	void render(const std::vector<StatePacket>& stateBuffer) const;

private:
	static const int GRID_SIZE = 11;
};