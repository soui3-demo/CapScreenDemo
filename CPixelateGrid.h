#pragma once
class CPixelateGrid
{
	CPixelateGrid() {};
	~CPixelateGrid() {};
	static bool OffsetFilter(Bitmap &b, Point **offset)
	{
		Rect bitmapRc{ 0,0,(INT)b.GetWidth(),(INT)b.GetHeight()};
		Bitmap *bSrc = b.Clone(bitmapRc, PixelFormat32bppRGB);

		// GDI+ still lies to us - the return format is BGR, NOT RGB.

		BitmapData bmData;
		b.LockBits(&bitmapRc, ImageLockMode::ImageLockModeWrite, PixelFormat32bppRGB, &bmData);
		BitmapData bmSrc;
		bSrc->LockBits(&bitmapRc, ImageLockMode::ImageLockModeRead, PixelFormat32bppRGB, &bmSrc);

		int scanline = bmSrc.Stride;

		byte * p = (byte *)(void *)bmData.Scan0;
		byte * pSrc = (byte *)(void *)bmSrc.Scan0;

#define bits 4

		int nOffset = bmData.Stride - b.GetWidth() * bits;
		int nWidth = b.GetWidth();
		int nHeight = b.GetHeight();

		int xOffset, yOffset;

		for (int y = 0; y < nHeight; ++y)
		{
			for (int x = 0; x < nWidth; ++x)
			{
				xOffset = offset[x][y].X;
				yOffset = offset[x][y].Y;
				if (y + yOffset >= 0 && y + yOffset < nHeight && x + xOffset >= 0 && x + xOffset < nWidth)
				{
					p[0] = pSrc[((y + yOffset) * scanline) + ((x + xOffset) * bits)];
					p[1] = pSrc[((y + yOffset) * scanline) + ((x + xOffset) * bits) + 1];
					p[2] = pSrc[((y + yOffset) * scanline) + ((x + xOffset) * bits) + 2];
					p[3] = pSrc[((y + yOffset) * scanline) + ((x + xOffset) * bits) + 3];
				}
				p += bits;
			}
			p += nOffset;
		}

		b.UnlockBits(&bmData);
		bSrc->UnlockBits(&bmSrc);
		delete bSrc;
		return true;
	}
public:
	
	static bool Pixelate(Bitmap &b, short pixel, bool bGrid)
	{
		int nWidth = b.GetWidth();
		int nHeight = b.GetHeight();

		Point** pt = new Point*[nWidth];
		for (int i = 0; i<nWidth; i++)
			pt[i] = new Point[nHeight];

		int newX, newY;

		for (int x = 0; x < nWidth; ++x)
			for (int y = 0; y < nHeight; ++y)
			{
				newX = pixel - x % pixel;

				if (bGrid && newX == pixel)
					pt[x][y].X = -x;
				else if (x + newX > 0 && x + newX < nWidth)
					pt[x][y].X = newX;
				else
					pt[x][y].X = 0;

				newY = pixel - y % pixel;

				if (bGrid && newY == pixel)
					pt[x][y].Y = -y;
				else if (y + newY > 0 && y + newY < nHeight)
					pt[x][y].Y = newY;
				else
					pt[x][y].Y = 0;
			}

		OffsetFilter(b, pt);
		for (int i = 0; i<nWidth; i++)
		{
			delete[nHeight] pt[i];
		}
		delete[nWidth]pt;
		return true;
	}
};

