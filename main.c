#include <stdio.h>
#include <stdlib.h>
#include "lib/chaining.h"
#include "lib/serving.h"

void home () {
    printf("Hey\n");
}

int main (void) {
	serving server;
	serving_endpoint_set(&server, "GET", "/", &home);
	serving_endpoint_set(&server, "POST", "/login", &home);
	serving_endpoint_set(&server, "PATCH", "/func", &home);
	return serving_server_run(&server, 6969);
}
