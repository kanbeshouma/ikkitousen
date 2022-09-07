#include"SwordTrail.hlsli"
#include "constants.hlsli"
[maxvertexcount(3)]
void main(
	triangle VsOutput input[3] , 
	inout TriangleStream< PsInput > output
)
{
	for (uint i = 0; i < 3; i++)
	{
		PsInput element;
		element.Position = input[i].Position;
		element.Texcoord = input[i].Texcoord;
		output.Append(element);
	}
}