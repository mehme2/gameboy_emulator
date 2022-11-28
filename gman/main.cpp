#include <Windows.h>
#include <d3d11.h>
#include <wrl.h>
#include <DirectXMath.h>

namespace Shaders
{
#include "VertexShader.shh"
#include "PixelShader.shh"
}

LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
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
		CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, hInstance, NULL);

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
	vp.Width = width;
	vp.Height = height;
	vp.MinDepth = 0;
	vp.MaxDepth = 1;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	pContext->RSSetViewports(1u, &vp);


	struct Vertex
	{
		dx::XMFLOAT3 pos;
	} vertices[4];

	vertices[0].pos = { -1.0f, 1.0f, 0.0f };
	vertices[1].pos = { 1.0f, 1.0f, 0.0f };
	vertices[2].pos = { -1.0f, -1.0f, 0.0f };
	vertices[3].pos = { 1.0f, -1.0f, 0.0f };

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
		{"Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	if (FAILED(pDevice->CreateInputLayout(ied, 1, Shaders::VertexShaderBytecode, sizeof(Shaders::VertexShaderBytecode), &pInputLayout)))
	{
		MessageBox(nullptr, "Failed to create input layout.", "D3D11 Error", MB_OK | MB_ICONWARNING);
	}

	pContext->IASetInputLayout(pInputLayout.Get());

	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	float background[4] = { 0.4f,0.4f,0.4f,1.0f };

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
		pContext->ClearRenderTargetView(pTargetView.Get(), background);
		pContext->DrawIndexed(6, 0, 0);
		pSwapChain->Present(1, 0);
	}
}