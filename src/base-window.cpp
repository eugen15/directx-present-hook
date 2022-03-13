// Copyright 2022 Eugen Hartmann.
// Licensed under the MIT License (MIT).

#include "base-window.h"

static LRESULT CALLBACK WindowProc(HWND windowHandle,
  UINT message, WPARAM wParam, LPARAM lParam) {
  BaseWindow* window = reinterpret_cast<BaseWindow*>(
    GetWindowLongPtr(windowHandle, GWLP_USERDATA));

  switch (message) {
  case WM_CREATE: {
    LPCREATESTRUCT createStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
    SetWindowLongPtr(windowHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(createStruct->lpCreateParams));
    return 0;
  }
  case WM_SIZE: {
    if (window) {
      UINT width = LOWORD(lParam);
      UINT height = HIWORD(lParam);
      window->OnSize(width, height);
    }
    return 0;
  }
  case WM_PAINT: {
    if (window) {
      window->OnPaint();
    }
    return 0;
  }
  case WM_DESTROY: {
    if (window) {
      window->OnDestroy();
    }
    return 0;
  }
  }

  // Handle any messages the switch statement didn't
  return DefWindowProc(windowHandle, message, wParam, lParam);
}


BaseWindow::BaseWindow(std::wstring_view windowTitle)
    : windowTitle_(windowTitle) {
}

BaseWindow::~BaseWindow() {
  if (windowHandle_) {
    DestroyWindow(windowHandle_);
  }
  if (windowClass_.hInstance) {
    UnregisterClass(windowTitle_.data(), windowClass_.hInstance);
  }
}

HRESULT BaseWindow::Initialize(UINT width, UINT height) {
  HRESULT hr;

  hr = CreateWindowClass();
  if (FAILED(hr)) {
    return hr;
  }

  hr = Create(width, height);
  if (FAILED(hr)) {
    return hr;
  }

  return hr;
}

void BaseWindow::Uninitialize() {
  if (windowHandle_) {
    DestroyWindow(windowHandle_);
  }

  if (windowClass_.hInstance) {
    UnregisterClass(windowClass_.lpszClassName, windowClass_.hInstance);
  }
}

HWND BaseWindow::GetHandle() const {
  return windowHandle_;
}

void BaseWindow::OnSize(UINT width, UINT height) {
  // Do nothing
}

void BaseWindow::OnPaint() {
  // Do nothing
}

void BaseWindow::OnDestroy() {
  // Do nothing
}

HRESULT BaseWindow::CreateWindowClass() {
  windowClass_ = {};
  windowClass_.cbSize = sizeof(WNDCLASSEX);
  windowClass_.style = CS_HREDRAW|CS_VREDRAW;;
  windowClass_.lpfnWndProc = WindowProc;
  windowClass_.hInstance = GetModuleHandle(NULL);
  windowClass_.lpszClassName = windowTitle_.data();

  if (!RegisterClassEx(&windowClass_)) {
    return HRESULT_FROM_WIN32(GetLastError());
  }

  return S_OK;
}

HRESULT BaseWindow::Create(UINT width, UINT height) {
  windowHandle_ = CreateWindow(
    windowClass_.lpszClassName,
    windowTitle_.data(),
    WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT,
    CW_USEDEFAULT,
    width,
    height,
    NULL,        // no parent window
    NULL,        // no menus
    windowClass_.hInstance,
    this);
  if (!windowHandle_) {
    return HRESULT_FROM_WIN32(GetLastError());
  }
  return S_OK;
}
