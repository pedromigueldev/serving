#include <stdio.h>
#include <stdlib.h>
#include "lib/chaining.h"
#include "lib/serving.h"

int main (void) {
	serving server;

	return serving_server_run(&server, 6969);
}
