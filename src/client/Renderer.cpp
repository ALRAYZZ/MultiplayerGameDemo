#include "Renderer.h"
#include <iostream>

void Renderer::render(const StatePacket& state) const
{
	char grid[GRID_SIZE][GRID_SIZE] = { {'.'} };

	for (int y = 0; y < GRID_SIZE; ++y)
	{
		for (int x = 0; x < GRID_SIZE; ++x)
		{
			grid[y][x] = '.'; // Building the dotted grid
		}
	}

	for (uint32_t i = 0; i < state.playerCount; ++i)
	{
		int x = static_cast<int>(state.players[i].x);
		int y = static_cast<int>(state.players[i].y);
		if (x >= 0 && x < GRID_SIZE && y >= 0 && y < GRID_SIZE)
		{
			grid[y][x] = 'P'; // Marking player positions with 'P'
		}
	}

	system("cls"); // Clear the console (Windows specific, use "clear" for Unix-like systems)
	for (int y = 0; y < GRID_SIZE; ++y)
	{
		for (int x = 0; x < GRID_SIZE; ++x)
		{
			std::cout << grid[y][x] << ' ';
		}
		std::cout << std::endl;
	}
}