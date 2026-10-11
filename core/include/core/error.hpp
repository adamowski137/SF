#pragma once

void glClearError();
bool glCheckError();
#define Call(q0) q0; \
				glCheckError() 
