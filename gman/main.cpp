#include <Windows.h>
#include <d3d11.h>
#include <wrl.h>
#include <DirectXMath.h>
#include <xaudio2.h>

namespace Shaders
{
#include "VertexShader.shh"
#include "PixelShader.shh"
}

#include "GMan.h"

uint8_t input=0xFF;

LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CLOSE:
		PostQuitMessage(31);
		break;
	case WM_KEYDOWN:
		if (!(lParam & 0x40000000))
		{
			switch (wParam)
			{
			case VK_RIGHT:
				input &= ~0x01;
				break;
			case VK_LEFT:
				input &= ~0x02;
				break;
			case VK_UP:
				input &= ~0x04;
				break;
			case VK_DOWN:
				input &= ~0x08;
				break;
			case 'Z':
				input &= ~0x10;
				break;
			case 'X':
				input &= ~0x20;
				break;
			case VK_BACK:
				input &= ~0x40;
				break;
			case VK_RETURN:
				input &= ~0x80;
				break;
			}
		}
		break;
	case WM_KEYUP:
		switch (wParam)
		{
		case VK_RIGHT:
			input |= 0x01;
			break;
		case VK_LEFT:
			input |= 0x02;
			break;
		case VK_UP:
			input |= 0x04;
			break;
		case VK_DOWN:
			input |= 0x08;
			break;
		case 'Z':
			input |= 0x10;
			break;
		case 'X':
			input |= 0x20;
			break;
		case VK_BACK:
			input |= 0x40;
			break;
		case VK_RETURN:
			input |= 0x80;
			break;
		}
		break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

namespace dx = DirectX;
namespace wrl = Microsoft::WRL;

int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow
)
{
	char wClassName[] = "GMAN";

	static constexpr int width = 800;
	static constexpr int height = 720;

	WNDCLASSEX wc = {};

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.hCursor = nullptr;
	wc.hIcon = nullptr;
	wc.lpszClassName = wClassName;
	wc.hIconSm = nullptr;

	RegisterClassEx(&wc);

	RECT wr;
	wr.left = 100;
	wr.right = wr.left + width;
	wr.top = 100;
	wr.bottom = wr.top + height;

	AdjustWindowRect(&wr, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE);

	HWND hWnd = CreateWindowEx(
		0, wClassName, "GMAN",
		WS_BORDER | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
		CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top, NULL, NULL, hInstance, NULL);

	ShowWindow(hWnd, nCmdShow);


	DXGI_SWAP_CHAIN_DESC sd = {};

	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.RefreshRate.Denominator = 0;
	sd.BufferDesc.RefreshRate.Numerator = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 1;
	sd.OutputWindow = hWnd;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;

	wrl::ComPtr<IDXGISwapChain> pSwapChain;
	wrl::ComPtr<ID3D11Device> pDevice;
	wrl::ComPtr<ID3D11DeviceContext> pContext;

	if (FAILED(D3D11CreateDeviceAndSwapChain(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		0,
		NULL,
		0,
		D3D11_SDK_VERSION,
		&sd,
		&pSwapChain,
		&pDevice,
		NULL,
		&pContext
	)))
	{
		MessageBox(nullptr, "Failed to create device and swap chain.", "D3D11 Error", MB_OK | MB_ICONWARNING);
	}

	wrl::ComPtr<ID3D11RenderTargetView> pTargetView;
	wrl::ComPtr<ID3D11Resource> pBackBuffer;
	pSwapChain->GetBuffer(0, __uuidof(ID3D11Resource), &pBackBuffer);

	if (FAILED(pDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, pTargetView.GetAddressOf())))
	{
		MessageBox(nullptr, "Failed to create render target view.", "D3D11 Error", MB_OK | MB_ICONWARNING);
	}

	pContext->OMSetRenderTargets(1, pTargetView.GetAddressOf(), NULL);

	D3D11_VIEWPORT vp;
	vp.Width = float(width);
	vp.Height = float(height);
	vp.MinDepth = 0;
	vp.MaxDepth = 1;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	pContext->RSSetViewports(1u, &vp);


	struct Vertex
	{
		dx::XMFLOAT3 pos;
		dx::XMFLOAT2 texCoord;
	} vertices[4];

	vertices[0].pos = { -1.0f, 1.0f, 0.0f };
	vertices[1].pos = { 1.0f, 1.0f, 0.0f };
	vertices[2].pos = { -1.0f, -1.0f, 0.0f };
	vertices[3].pos = { 1.0f, -1.0f, 0.0f };

	vertices[0].texCoord = { 0.0f, 0.0f };
	vertices[1].texCoord = { 1.0f, 0.0f };
	vertices[2].texCoord = { 0.0f, 1.0f };
	vertices[3].texCoord = { 1.0f, 1.0f };

	D3D11_BUFFER_DESC vbd = {};
	vbd.ByteWidth = sizeof(vertices);
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = sizeof(Vertex);

	D3D11_SUBRESOURCE_DATA vertData = {};
	vertData.pSysMem = vertices;

	wrl::ComPtr<ID3D11Buffer> pVBuff;

	if (FAILED(pDevice->CreateBuffer(&vbd, &vertData, &pVBuff)))
	{
		MessageBox(nullptr, "Failed to create vertex buffer.", "D3D11 Error", MB_OK | MB_ICONWARNING);
	}

	unsigned int stride = sizeof(Vertex);
	unsigned int _offset = 0;

	pContext->IASetVertexBuffers(0, 1, pVBuff.GetAddressOf(), &stride, &_offset);

	wrl::ComPtr<ID3D11VertexShader> pVS;

	if (FAILED(pDevice->CreateVertexShader(Shaders::VertexShaderBytecode, sizeof(Shaders::VertexShaderBytecode), nullptr, &pVS)))
	{
		MessageBox(nullptr, "Failed to load vertex shader.", "D3D11 Error", MB_OK | MB_ICONWARNING);
	}

	pContext->VSSetShader(pVS.Get(), nullptr, 0);

	wrl::ComPtr<ID3D11PixelShader> pPS;

	if (FAILED(pDevice->CreatePixelShader(Shaders::PixelShaderBytecode, sizeof(Shaders::PixelShaderBytecode), nullptr, &pPS)))
	{
		MessageBox(nullptr, "Failed to load pixel shader.", "D3D11 Error", MB_OK | MB_ICONWARNING);
	}

	pContext->PSSetShader(pPS.Get(), nullptr, 0);

	unsigned short indices[] =
	{
		0, 1, 2, 1, 3, 2
	};

	D3D11_BUFFER_DESC ibd = {};
	ibd.ByteWidth = sizeof(indices);
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = sizeof(unsigned short);

	D3D11_SUBRESOURCE_DATA idxData = {};
	idxData.pSysMem = indices;

	wrl::ComPtr<ID3D11Buffer> pIBuff;

	if (FAILED(pDevice->CreateBuffer(&ibd, &idxData, &pIBuff)))
	{
		MessageBox(nullptr, "Failed to create index buffer.", "D3D11 Error", MB_OK | MB_ICONWARNING);
	}

	pContext->IASetIndexBuffer(pIBuff.Get(), DXGI_FORMAT_R16_UINT, 0);

	wrl::ComPtr<ID3D11InputLayout> pInputLayout;
	D3D11_INPUT_ELEMENT_DESC ied[] =
	{
		{"Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TexCoord", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	if (FAILED(pDevice->CreateInputLayout(ied, 2, Shaders::VertexShaderBytecode, sizeof(Shaders::VertexShaderBytecode), &pInputLayout)))
	{
		MessageBox(nullptr, "Failed to create input layout.", "D3D11 Error", MB_OK | MB_ICONWARNING);
	}

	pContext->IASetInputLayout(pInputLayout.Get());

	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	wrl::ComPtr<ID3D11Texture2D> pTex;

	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = 160;
	texDesc.Height = 144;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DYNAMIC;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	texDesc.MiscFlags = 0;

	if (FAILED(pDevice->CreateTexture2D(&texDesc, nullptr, &pTex)))
	{
		MessageBox(nullptr, "Failed to create texture.", "D3D11 Error", MB_OK | MB_ICONWARNING);
	}

	wrl::ComPtr<ID3D11ShaderResourceView> pTexView;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	if (FAILED(pDevice->CreateShaderResourceView(pTex.Get(),
		&srvDesc, &pTexView)))
	{
		MessageBox(nullptr, "Failed to create texture view.", "D3D11 Error", MB_OK | MB_ICONWARNING);
	}

	wrl::ComPtr<ID3D11SamplerState> pSamplerState;

	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	if (FAILED(pDevice->CreateSamplerState(&sampDesc, &pSamplerState)))
	{
		MessageBox(nullptr, "Failed to create texture sampler.", "D3D11 Error", MB_OK | MB_ICONWARNING);
	}

	struct Color
	{
		unsigned char b;
		unsigned char g;
		unsigned char r;
		unsigned char a;
	};

	Color* pBuffer = (Color*)malloc(sizeof(Color) * 144 * 160);

	pContext->PSSetShaderResources(0u, 1u, pTexView.GetAddressOf());
	pContext->PSSetSamplers(0u, 1u, pSamplerState.GetAddressOf());

	CoInitializeEx(NULL, COINITBASE_MULTITHREADED);

	wrl::ComPtr<IXAudio2> pxa;
	if(FAILED(XAudio2Create(&pxa, 0, XAUDIO2_DEFAULT_PROCESSOR)))
	{
		MessageBox(nullptr, "Failed to create sound interface.", "XAudio2 Error", MB_OK | MB_ICONWARNING);
	}

	IXAudio2MasteringVoice* pmv;
	if(FAILED(pxa->CreateMasteringVoice(&pmv)))
	{
		MessageBox(nullptr, "Failed to create masteringvoice.", "XAudio2 Error", MB_OK | MB_ICONWARNING);
	}

	static constexpr int aBufSize = 44100;

	WAVEFORMATEX format;
	format.nChannels = 1;
	format.nSamplesPerSec = 44100;
	format.wBitsPerSample = 8;
	format.nBlockAlign = (format.nChannels * format.wBitsPerSample) / 8;
	format.nAvgBytesPerSec = format.nBlockAlign * format.nSamplesPerSec;
	format.cbSize = 0;
	format.wFormatTag = WAVE_FORMAT_PCM;

	uint8_t* buffa = new uint8_t[aBufSize];
	uint8_t* backBuf = new uint8_t[aBufSize];

	XAUDIO2_BUFFER aBuf;
	aBuf.Flags = 0;
	aBuf.AudioBytes = aBufSize;
	aBuf.pAudioData = buffa;
	aBuf.PlayBegin = 0;
	aBuf.PlayLength = 0;
	aBuf.LoopBegin = 0;
	aBuf.LoopLength = 0;
	aBuf.LoopCount = XAUDIO2_LOOP_INFINITE;
	aBuf.pContext = NULL;

	IXAudio2SourceVoice* psv;
	if(FAILED(pxa->CreateSourceVoice(&psv, &format)))
	{
		MessageBox(nullptr, "Failed to create sourcevoice.", "XAudio2 Error", MB_OK | MB_ICONWARNING);
	}

	if(FAILED(psv->SubmitSourceBuffer(&aBuf)))
	{
		MessageBox(nullptr, "Failed to submitsourcebuffer.", "XAudio2 Error", MB_OK | MB_ICONWARNING);
	}

	float background[4] = { 0.4f,0.4f,0.4f,1.0f };

	GMan gman;

	char pathBuf[256];
	OPENFILENAME ofna;
	ZeroMemory(&ofna, sizeof(ofna));
	ofna.lStructSize = sizeof(OPENFILENAME);
	ofna.hwndOwner = hWnd;
	ofna.hInstance = hInstance;
	ofna.lpstrFilter = "All\0*.*\0Gameboy ROMs(.gb)\0*.GB\0";
	ofna.nFilterIndex = 2;
	ofna.lpstrCustomFilter = NULL;
	ofna.nMaxCustFilter = 0;
	ofna.lpstrFile = pathBuf;
	ofna.lpstrFile[0] = '\0';
	ofna.nMaxFile = 256;
	ofna.lpstrFileTitle = NULL;
	ofna.nMaxFileTitle = 0;
	ofna.lpstrInitialDir = NULL;
	ofna.lpstrTitle = NULL;
	ofna.Flags = OFN_DONTADDTORECENT | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_READONLY;

	if (GetOpenFileName(&ofna))
	{
		gman.LoadRom(pathBuf);
	}
	else
	{
		return -1;
	}
	bool loadBoot = false;
	if (loadBoot)
	{
		ofna.lpstrFile[0] = '\0';
		if (GetOpenFileName(&ofna))
		{
			gman.LoadBootRom(pathBuf);
		}
	}
	gman.SetPixelBuffer(pBuffer);
	gman.SetSoundBuffer(buffa, aBuf.AudioBytes, format.nSamplesPerSec);
	gman.BindKeyPtr(&input);

	if (FAILED(psv->Start()))
	{
		MessageBox(nullptr, "Failed to start.", "XAudio2 Error", MB_OK | MB_ICONWARNING);
	}
	
	MSG msg;
	while (1)
	{
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				return (int)msg.wParam;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		gman.DoFrame();
		//memcpy(buffa, backBuf, aBufSize);
		D3D11_MAPPED_SUBRESOURCE msr;
		if (FAILED(pContext->Map(pTex.Get(), 0u,
			D3D11_MAP_WRITE_DISCARD, 0u, &msr)))
		{
			MessageBox(nullptr, "Failed to map buffer.", "D3D11 Error", MB_OK | MB_ICONWARNING);
		}
		Color* pmb = (Color*)msr.pData;
		for (int y = 0; y < 144; y++)
		{
			memcpy(&pmb[y * 160], &pBuffer[y * 160], sizeof(Color) * 160);
		}
		pContext->Unmap(pTex.Get(), 0u);
		pContext->ClearRenderTargetView(pTargetView.Get(), background);
		pContext->DrawIndexed(6, 0, 0);
		pSwapChain->Present(1, 0);
		memset(pBuffer, 0, sizeof(Color) * 144 * 160);
 	}
}