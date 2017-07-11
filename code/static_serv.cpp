#include <SDL2/SDL.h>
#include <SDL/SDL_net.h>

#include "rivten.h"

/*
 * TODO(hugo)
      - better connection manipulation (SocketSet?)
	  - enable multiple connection with multithreading ?
	  - more request method handling
 */

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

void TCPSend(TCPsocket Socket, rvtn_string Message)
{
	char* MessageCString = CString_(Message);
	SDLNet_TCP_Send(Socket, MessageCString, Message.Size + 1);
	Free(MessageCString);
}

void TCPSend(TCPsocket Socket, char* Message)
{
	rvtn_string MessageString = CreateString(Message);
	TCPSend(Socket, MessageString);
	FreeString(&MessageString);
}

rvtn_string TCPReceive(TCPsocket Socket)
{
	char MsgBuffer[1024];
	rvtn_string Result = {};
	s32 MsgSize = SDLNet_TCP_Recv(Socket, MsgBuffer, ArrayCount(MsgBuffer));
	Assert(MsgSize != -1);
	if(MsgSize > 0)
	{
		Result = CreateString(MsgBuffer);
	}

	return(Result);
}

enum http_method
{
	HTTPMethod_GET,
	HTTPMethod_HEAD,
	HTTPMethod_POST,

	HTTPMethod_Count,
};

struct http_request
{
	http_method Method;
	rvtn_string URI;
};

http_request ParseHTTPRequest(rvtn_string Request)
{
	http_request Result = {};

	consume_token_result ConsumeTokenResult = ConsumeToken(Request, " ");

	// NOTE(hugo) : Parse method
	rvtn_string MethodString = ConsumeTokenResult.Token;
	if(StringMatch(MethodString, "GET"))
	{
		Result.Method = HTTPMethod_GET;
	}
	else if(StringMatch(MethodString, "HEAD"))
	{
		Result.Method = HTTPMethod_HEAD;
	}
	else if(StringMatch(MethodString, "POST"))
	{
		Result.Method = HTTPMethod_POST;
	}
	else
	{
		InvalidCodePath;
	}

	ConsumeTokenResult = ConsumeToken(ConsumeTokenResult.Remain, " ");
	Result.URI = CreateString(ConsumeTokenResult.Token);

	return(Result);
}

struct http_response
{
	u32 StatusCode;
	rvtn_string Body;
};

http_response RespondToRequest(http_request Request)
{
	http_response Result = {};

	Result.StatusCode = 200;

	Result.Body = CreateString("<H1>Success!</H1>");

	return(Result);
}

rvtn_string GenerateStringFromHTTPResponse(http_response Response)
{
	char StatusLineBuffer[2048];
	char* StatusReason = 0;
	switch(Response.StatusCode)
	{
		case 200:
			{
				StatusReason = "OK";
			} break;
		case 404:
			{
				StatusReason = "Not Found";
			} break;
		InvalidDefaultCase;
	}

	sprintf(StatusLineBuffer, "HTTP/1.0 %i %s\nContent-type: text/html\n\n", 
			Response.StatusCode, StatusReason);
	rvtn_string StatusLine = CreateString(StatusLineBuffer);
	rvtn_string Result = ConcatString(StatusLine, Response.Body);

	return(Result);
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
#if 0
					IPaddress* ClientIP = SDLNet_TCP_GetPeerAddress(ClientSocket);
					Assert(ClientIP);
					SDL_Log("A connection was established from client IP %d on port %d.\n", 
							ClientIP->host, ClientIP->port);
#endif

					rvtn_string ClientMessage = TCPReceive(ClientSocket);
					if(ClientMessage.Size > 0)
					{
						http_request HTTPRequest = ParseHTTPRequest(ClientMessage);
						FreeString(&ClientMessage);

						http_response HTTPResponse = RespondToRequest(HTTPRequest);

						rvtn_string ResponseString = GenerateStringFromHTTPResponse(HTTPResponse);
						Print(ResponseString);
						TCPSend(ClientSocket, ResponseString);
						SDLNet_TCP_Close(ClientSocket);
					}

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
