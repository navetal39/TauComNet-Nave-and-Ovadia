#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

int main(int argc, char* argv)
{
	if(argc==2)
	{
		/*ip+port were given*/
	}
	if(argc==1)
	{
		/*ip was given, default port*/
	}
	if(argc==0)
	{
		/*default ip and port*/
	}
}
