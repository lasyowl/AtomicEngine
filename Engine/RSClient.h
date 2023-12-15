#pragma once

#include "EngineEssential.h"
#include "Renderer.h"

class IRSClient
{
public:
	virtual void Launch( void* AppEntryParam ) {};

protected:
	CRenderer Renderer;
};

class CRSClientWindows : public IRSClient
{
public:
	virtual void Launch( void* AppEntryParam ) override;

protected:
	void InitGraphics();
	void RenderFrame();

private:
	void CreateClientWindow( HINSTANCE ProcessHandle );

	static LRESULT CALLBACK WindowMessageProcessor( HWND InHWnd, UINT Msg, WPARAM WParam, LPARAM LParam );

private:
	HWND WindowHandle;
	RECT WindowRect;
};

class CRSClientLinux : public IRSClient
{
public:
	virtual void Launch( void* AppEntryParam ) override {}
};