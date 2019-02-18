/*
 * ImageGrabber.cpp: MulticopterSim support for acquisition of camera images and processing
 *
 * Adapted from https://answers.unrealengine.com/questions/193827/how-to-get-texture-pixels-using-utexturerendertarg.html
 *
 * Copyright (C) 2019 Simon D. Levy
 *
 * MIT License
 */

#include "ImageGrabber.h"

ImageGrabber::ImageGrabber(UTextureRenderTarget2D* visionTextureRenderTarget)
{
	// Get the size of the render target
	uint16_t rows = visionTextureRenderTarget->SizeY;
	uint16_t cols = visionTextureRenderTarget->SizeX;

	// Create Texture2D to store render content
	UTexture2D* texture = UTexture2D::CreateTransient(cols, rows, PF_B8G8R8A8);

#if WITH_EDITORONLY_DATA
	texture->MipGenSettings = TMGS_NoMipmaps;
#endif

	texture->SRGB = visionTextureRenderTarget->SRGB;

	_renderTarget = visionTextureRenderTarget->GameThread_GetRenderTargetResource();
}

ImageGrabber::~ImageGrabber(void)
{
}

// Runs on main thread
void ImageGrabber::grabImage(void)
{
	// Read the pixels from the RenderTarget
	TArray<FColor> SurfData;
	_renderTarget->ReadPixels(SurfData);

	// Copy the pixels to the image in the subcass
	copyImageData(SurfData.GetData(), SurfData.Num() * 4); // RGBA
}
