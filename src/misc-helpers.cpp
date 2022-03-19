// Copyright 2022 Eugen Hartmann.
// Licensed under the MIT License (MIT).

#include "misc-helpers.h"

namespace MiscHelpers {

std::vector<std::uint8_t> GenerateSquareRGBAPicture(std::uint32_t width) {
  const std::uint32_t rowPitch = width * 4;
  const std::uint32_t cellPitch = rowPitch >> 3; // The width of a cell in the checkboard texture.
  const std::uint32_t cellHeight = width >> 3;   // The height of a cell in the checkerboard texture.
  const std::uint32_t textureSize = rowPitch * width;

  std::vector<std::uint8_t> data(textureSize);
  std::uint8_t* p = &data.front();

  for (std::uint32_t n = 0; n < textureSize; n += 4) {
    std::uint32_t x = n % rowPitch;
    std::uint32_t y = n / rowPitch;
    std::uint32_t i = x / cellPitch;
    std::uint32_t j = y / cellHeight;

    if (i % 2 == j % 2) {
      p[n] = 0x00;      // R
      p[n + 1] = 0x00;  // G
      p[n + 2] = 0x00;  // B
      p[n + 3] = 0xFF;  // A
    }
    else {
      p[n] = 0xFF;      // R
      p[n + 1] = 0xFF;  // G
      p[n + 2] = 0xFF;  // B
      p[n + 3] = 0xFF;  // A
    }
  }

  return data;
}


std::vector<std::uint8_t> ConvertRGBAToBMP(const std::uint8_t* rgbaData,
    std::uint32_t width, std::uint32_t height, std::uint32_t stride) {

  std::uint32_t bmpStride = width * 3;
  std::uint32_t paddingSize = (4 - (bmpStride) % 4) % 4;
  
  std::uint32_t dataSize = 54 + bmpStride * height;
  std::uint32_t bufferSize = dataSize + paddingSize * height;

  std::vector<std::uint8_t> buffer(bufferSize);

  // BITMAPFILEHEADER
  std::uint8_t* bmpFileHeader = &buffer.front();
  std::memset(bmpFileHeader, 0, 14);

  // "BM"
  *static_cast<std::uint32_t*>(
    static_cast<void*>(bmpFileHeader)) = 19778; 

  // BGR data size (including padding).
  *static_cast<std::uint32_t*>(
    static_cast<void*>(bmpFileHeader + 2)) = dataSize;

  // The BGR data offset.
  *static_cast<std::uint32_t*>(
    static_cast<void*>(bmpFileHeader + 10)) = 54;

  // BITMAPINFOHEADER
  std::uint8_t* bmpInfoHeader = &buffer.front() + 14;
  std::memset(bmpInfoHeader, 0, 40);

  // The header size.
  *static_cast<std::uint32_t*>(
    static_cast<void*>(bmpInfoHeader)) = 40;

  // Picture width.
  *static_cast<std::uint32_t*>(
    static_cast<void*>(bmpInfoHeader + 4)) = width;

  // Picture height.
  *static_cast<std::uint32_t*>(
    static_cast<void*>(bmpInfoHeader + 8)) = height;

  // Planes.
  *static_cast<std::uint32_t*>(
    static_cast<void*>(bmpInfoHeader + 12)) = 1;

  // Bits per pixel
  *static_cast<std::uint32_t*>(
    static_cast<void*>(bmpInfoHeader + 14)) = 24;

  const std::uint8_t* src = rgbaData;
  std::uint8_t* dst = &buffer.front() + 54;

  for (std::uint32_t h = 0; h < height; ++h) {
    // Copy the row.
    for (std::uint32_t w = 0; w < width; ++w, src += 4, dst += 3) {
      dst[2] = src[0];
      dst[1] = src[1];
      dst[0] = src[2];
    }
    src += stride - width * 4; // ignore the remaining source row data.
    // Padding.
    for (std::uint32_t i = 0; i < paddingSize; ++i, ++dst) {
      *dst = 0;
    }
  }

  return buffer;
}

HRESULT SaveDataToFile(std::wstring_view filename,
    const void* data, std::size_t dataSizeInBytes) {
  HANDLE fileHandle = CreateFile(filename.data(),
    GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
  if (fileHandle == INVALID_HANDLE_VALUE) {
    return HRESULT_FROM_WIN32(GetLastError());
  }
  DWORD bytesWritten;
  if (!WriteFile(fileHandle, data, static_cast<DWORD>(dataSizeInBytes),
    &bytesWritten, NULL)) {
    return HRESULT_FROM_WIN32(GetLastError());
  }
  CloseHandle(fileHandle);
  return S_OK;
}

} // namespace FileHelpers