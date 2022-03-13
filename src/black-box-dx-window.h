// Copyright 2022 Eugen Hartmann.
// Licensed under the MIT License (MIT).

#pragma once

#include <memory>

#include "base-window.h"

// A simple window class template
template<class T>
class BlackBoxDXWindow : public BaseWindow {
public:
  BlackBoxDXWindow(std::wstring_view windowTitle)
    : BaseWindow(windowTitle) {
  }

  ~BlackBoxDXWindow() override {
    Uninitialize();
  }

  HRESULT Initialize(UINT width, UINT height) override {
    HRESULT hr;

    hr = BaseWindow::Initialize(width, height);
    if (FAILED(hr)) {
      return hr;
    }

    hr = renderer_.Initialize(GetHandle(), width, height);
    if (FAILED(hr)) {
      return hr;
    }

    return hr;
  }

  void Uninitialize() override {
    BaseWindow::Uninitialize();
  }

  void Show(int cmdShow) {
    ShowWindow(windowHandle_, cmdShow);
  }

  void OnSize(UINT width, UINT height) override {
    // TODO
  }

  void OnPaint() override {
    renderer_.Render();
  }

  void OnDestroy() override {
    PostQuitMessage(0);
  }

private:

  T renderer_;
};