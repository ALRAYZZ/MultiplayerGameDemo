#include "Renderer.h"
#include <iostream>
#include <vector>

void Renderer::render(const std::vector<StatePacket>& stateBuffer) const
{
	// Use the most recent state for rendering
	const StatePacket* state1 = &stateBuffer.back();
	const StatePacket* state0 = stateBuffer.size() > 1 ? &stateBuffer[stateBuffer.size() - 2] : state1;

	// Interpolation factor (0 to 1) based on current time
	uint32_t currentTime = static_cast<uint32_t>(time(nullptr));
	float t = 0.0f;
	if (state1->timestamp > state0->timestamp)
	{
		t = static_cast<float>(currentTime - state0->timestamp) / (state1->timestamp - state0->timestamp);
		t = std::min(std::max(t, 0.0f), 1.0f); // Clamp t between 0 and	1
	}


	char grid[GRID_SIZE][GRID_SIZE] = { {'.'} };
	for (int y = 0; y < GRID_SIZE; ++y)
	{
		for (int x = 0; x < GRID_SIZE; ++x)
		{
			grid[y][x] = '.'; // Building the dotted grid
		}
	}

	// Interpolate player positions
	for (uint32_t i = 0; i < state1->playerCount; ++i)
	{
		const Player& player1 = state1->players[i];
		// Find matching player in state0
		Player player0 = player1; // Default to player1 if not found
		for (uint32_t j = 0; j < state0->playerCount; ++j)
		{
			if (state0->players[j].id == player1.id)
			{
				player0 = state0->players[j];
				break;
			}
		}

		// Interpolate position
		float x = player0.x + (player1.x - player0.x) * t;
		float y = player0.y + (player1.y - player0.y) * t;
		int gridX = static_cast<int>(std::round(x));
		int gridY = static_cast<int>(std::round(y));

		if (gridX >= 0 && gridX < GRID_SIZE && gridY >= 0 && gridY < GRID_SIZE)
		{
			grid[gridY][gridX] = 'P'; // Mark players position
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