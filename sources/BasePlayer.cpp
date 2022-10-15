#define _WINSOCKAPI_  // windows.hを定義した際に、winsock.hを自動的にインクルードしない

#include "BasePlayer.h"

void BasePlayer::SetColor(PlayerColor kind)
{
	switch (kind)
	{
	case PlayerColor::Red:
		color = { 1.0f,0.325f,0.325f,1.0f };
		break;
	case PlayerColor::Yellow:
		color = { 1.0f,0.921f,0.0f,1.0f };
		break;
	case PlayerColor::Blue:
		color = { 0.0f,0.327f,1.0f,1.0f };
		break;
	case PlayerColor::SkyBlue:
		color = { 0.0f,1.0f,0.984f,1.0f };
		break;
	case PlayerColor::Green:
		color = { 0.156f,1.0f,0.0f,1.0f };
		break;
	case PlayerColor::Purple:
		color = { 0.407f,0.0f,1.0f,1.0f };
		break;
	case PlayerColor::Pink:
		color = { 1.0f,0.0f,0.556f,1.0f };
		break;
	case PlayerColor::White:
		color = { 1.0f,1.0f,1.0f,1.0f };
		break;
	case PlayerColor::Black:
		color = { 0.0f,0.0f,0.0f,1.0f };
		break;
	default:
		break;
	}
}
