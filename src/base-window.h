// Copyright 2022 Eugen Hartmann.
// Licensed under the MIT License (MIT).

#pragma once

#include <Windows.h>
#include <string_view>

// A simple window class.
class BaseWindow {
public:

  // Constructs the window object.
  BaseWindow(std::wstring_view windowTitle);
  virtual ~BaseWindow();

  // Initializes the window object.
  virtual HRESULT Initialize(UINT width, UINT height);

  // Uninitialzes the window object.
  virtual void Uninitialize();

  // Gets the window handle (for a swap chain).
  virtual HWND GetHandle() const;  

  virtual void OnSize(UINT width, UINT height);
  virtual void OnPaint();
  virtual void OnDestroy();

protected:

  HRESULT CreateWindowClass();
  HRESULT Create(UINT width, UINT height);

  std::wstring windowTitle_;

  WNDCLASSEX windowClass_ = {};
  HWND windowHandle_ = NULL;
  HRESULT initializationResult_ = E_FAIL;
};  
