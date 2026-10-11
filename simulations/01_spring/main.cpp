#include "core/App.hpp"

int main()
{
	const int width = 1280;
	const int height = 800;
	const int ui_width = 300;

	App app(width, height, ui_width);
	app.loop();

	return 0;
}
