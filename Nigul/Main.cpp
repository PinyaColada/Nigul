#include "source/application.h"

int main()
{
	Application app(1200, 1200, "Nigul");
	app.init();
	app.loop();
	app.destroy();
	return 0;
}