#pragma once
#include "GameProtocol.h"

class Renderer
{
public:
	void render(const StatePacket& state) const;

private:
	static const int GRID_SIZE = 11;
};