#include <SDL2/SDL.h>
#include <SDL/SDL_net.h>

#include "rivten.h"

struct program_arguments
{
	u32 Count;
	rvtn_string Arguments[20];
};

program_arguments GetProgramArguments(u32 ArgumentCount, char** Arguments)
{
	program_arguments Result = {};
	Assert(ArgumentCount < ArrayCount(Result.Arguments));
	Result.Count = ArgumentCount;
	for(u32 ArgumentIndex = 0;
			ArgumentIndex < ArgumentCount;
			++ArgumentIndex)
	{
		Result.Arguments[ArgumentIndex] = CreateString(Arguments[ArgumentIndex]);
	}

	return(Result);
}

void ShowSDLNetVersion(void)
{
	SDL_version CompileVersion;
	const SDL_version* LinkVersion = SDLNet_Linked_Version();
	SDL_NET_VERSION(&CompileVersion);
	SDL_Log("Program is compiled with SDL_Net version : %d.%d.%d\n",
			CompileVersion.major,
			CompileVersion.minor,
			CompileVersion.patch);

	SDL_Log("Program is runnning witch SDL_Net version : %d.%d.%d\n", 
			LinkVersion->major,
			LinkVersion->minor,
			LinkVersion->patch);
}

void SendMessageToClient(TCPsocket ClientSocket, rvtn_string Message)
{
	char* MessageCString = CString_(Message);
	SDLNet_TCP_Send(ClientSocket, MessageCString, Message.Size + 1);
}

void Server(void)
{
	IPaddress Address;
	s32 ResolveHostResult = SDLNet_ResolveHost(&Address, 0, 4000);

	SDLNet_SocketSet Sockets = SDLNet_AllocSocketSet(16);
	if(ResolveHostResult == 0)
	{
		TCPsocket ServerSocket = SDLNet_TCP_Open(&Address);
		if(ServerSocket)
		{
			while(true)
			{
				TCPsocket ClientSocket = SDLNet_TCP_Accept(ServerSocket);
				if(ClientSocket)
				{
#if 1
					IPaddress* ClientIP = SDLNet_TCP_GetPeerAddress(ClientSocket);
					Assert(ClientIP);
					SDL_Log("A connection was established from client IP %d on port %d.\n", 
							ClientIP->host, ClientIP->port);
#endif

					char MsgBuffer[1024];
					s32 MsgSize = SDLNet_TCP_Recv(ClientSocket, MsgBuffer, ArrayCount(MsgBuffer));
					if(MsgSize > 0)
					{
						SDL_Log("%i", MsgSize);
						Assert((u32)(MsgSize) < ArrayCount(MsgBuffer));
						MsgBuffer[MsgSize] = 0;
						rvtn_string MsgString = CreateString(MsgBuffer);

						// NOTE(hugo) : Do something with the HTTP message
						Print(MsgString);

						FreeString(&MsgString);
					}

					char* Text = "HTTP/1.0 200 OK\nContent-type: text/html\n\n<H1>Success!</H1><br/><H2>Youhou</H2>";
					u32 Length = (u32)(strlen(Text) + 1);
					SDLNet_TCP_Send(ClientSocket, Text, Length);
					SDLNet_TCP_Close(ClientSocket);
				}
			}
			SDLNet_TCP_Close(ServerSocket);
		}
		else
		{
			SDL_Log("TCP could not open: %s", SDLNet_GetError());
		}
	}
	else
	{
		SDL_Log("Host was not resolved: %s", SDLNet_GetError());
	}
}

int main(int ArgumentCount, char** Arguments)
{
	program_arguments ProgramArguments = GetProgramArguments(ArgumentCount, Arguments);

	u32 InitResult = SDLNet_Init();
	if(InitResult == 0)
	{
		SDL_Log("SDL_Net is properly initialized.\n");

		Server();

		SDLNet_Quit();
	}
	else
	{
		SDL_Log("SDL_Net encountered an error on init. Aborting Gardakan.\nReason: %s", SDLNet_GetError());
	}

	return(0);
}
