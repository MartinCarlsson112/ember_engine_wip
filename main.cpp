#include "game_app.h"
#include "ecs.h"
#include "voxels.h"
#include "components.h"

game_app app;

int main() 
{
	app.initialize();
	app.start();
	app.dispose();
	return 0;
}