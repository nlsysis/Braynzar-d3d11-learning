#include "Camera.h"
#include "DXinput.h"

///////Global Declartions
XMVECTOR DefaultForward = XMVectorSet(0.0f,0.0f,1.0f,0.0f);
XMVECTOR DefaultRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
XMVECTOR camForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
XMVECTOR camRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

XMVECTOR camTarget;
XMVECTOR camPosition;
XMVECTOR camUp;

XMMATRIX camRotationMatrix;
extern HWND hwnd;
extern XMMATRIX camView;

float moveLeftRight = 0.0f;
float moveBackForward = 0.0f;

float camYaw = 0.0f;
float camPitch = 0.0f;

float speed = 0.01f;

void InitCamera()
{
	//Camera information
	camPosition = XMVectorSet(0.0f, 3.0f, -8.0f, 0.0f);
	camTarget = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	camUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	//Set the View matrix
	camView = XMMatrixLookAtLH(camPosition, camTarget, camUp);
}
void UpdateCamera()
{
	
	if (isPressed(DIK_A))
	{
		moveLeftRight -= speed;
	}
	if (isPressed(DIK_D))
	{
		moveLeftRight += speed;
	}
	if (isPressed(DIK_W))
	{
		moveBackForward += speed;
	}
	if (isPressed(DIK_S))
	{
		moveBackForward -= speed;
	}
	if (isMouseMoved())
	{
		camYaw +=  0.01f;

		camPitch += 0.01f;

	}
	//update the camera target
	camRotationMatrix = XMMatrixRotationRollPitchYaw(camPitch, camYaw, 0);  //return matrix of rotation
	camTarget = XMVector3TransformCoord(DefaultForward, camRotationMatrix);
	camTarget = XMVector3Normalize(camTarget);

	XMMATRIX RotateYTempMatrix;
	RotateYTempMatrix = XMMatrixRotationY(camYaw);
	camRight = XMVector3TransformCoord(DefaultRight, RotateYTempMatrix);
	camUp = XMVector3TransformCoord(camUp, RotateYTempMatrix);
	camForward = XMVector3TransformCoord(DefaultForward, RotateYTempMatrix);

	camPosition += moveLeftRight * camRight;    //the move in right
	camPosition += moveBackForward * camForward;   //the move in forward

	moveLeftRight = 0.0f;
	moveBackForward = 0.0f;

	camTarget = camPosition + camTarget;

	camView = XMMatrixLookAtLH(camPosition, camTarget, camUp);

}