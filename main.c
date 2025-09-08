#include <stdio.h>
#include "lib/serving.h"

void home () {
    printf("Hey\n");
}

int main (void) {
	serving server = {0};
	serving_endpoint_set(&server, SERVING_METHOD_GET, "/", &home);
	serving_endpoint_set(&server, SERVING_METHOD_POST, "/login", &home);
	serving_endpoint_set(&server, SERVING_METHOD_PATCH, "/func", &home);
	return serving_server_run(&server, 6969);
}
