#include <stdio.h>
#include "lib/serving.h"

void home () {
    printf("Hey\n");
}

int main (void) {
	serving server = {0};
	serving_endpoint_set(&server, "GET", "/", &home);
	serving_endpoint_set(&server, "POST", "/login", &home);
	serving_endpoint_set(&server, "PATCH", "/func", &home);
	return serving_server_run(&server, 6970);
}
