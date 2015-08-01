//****************************
// Utility functions
// autor: Marcin Gozdziewski
// ***************************

void ZeroBuffer(void* buffer, int len)
{
	unsigned char* uchPtr = (unsigned char*) buffer;

	for(int i = 0; i != len; i++)
	{
		*uchPtr++ = 0;
	}
}
