#include "SpringSimApp.hpp"

int main()
{
	const int width = 1280;
	const int height = 800;
	const int ui_width = 300;

	SpringSimApp app(width, height, ui_width);
	app.loop();

	return 0;
}
