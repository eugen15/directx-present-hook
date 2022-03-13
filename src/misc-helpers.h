// Copyright 2022 Eugen Hartmann.
// Licensed under the MIT License (MIT).

#pragma once

#include <Windows.h>

#include <string>
#include <vector>

namespace MiscHelpers {
  // Creates a sample RGBA picture.
  std::vector<std::uint8_t> GenerateSquareRGBAPicture(std::uint32_t width);

  // Converts an RGBA image to BMP format image.
  std::vector<std::uint8_t> ConvertRGBAToBMP(const std::uint8_t* rgbaData,
    std::uint32_t width, std::uint32_t height, std::uint32_t rowPitch);

  // Saves any binary data to a file.
  HRESULT SaveDataToFile(std::wstring_view filename,
    const void* data, std::size_t dataSizeInBytes);
} // namespace FileHelpers