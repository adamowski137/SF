#pragma once

#include "core/UI.hpp"

class SpringSimUI : public UI {
	float& cubeSize;
	float* cubeColor;
	float* lightPos;

public:
	SpringSimUI(int width, float& inCubeSize, float* inCubeColor, float* inLightPos);
	virtual ~SpringSimUI() = default;

	void renderUI(Window& window) override;
};
