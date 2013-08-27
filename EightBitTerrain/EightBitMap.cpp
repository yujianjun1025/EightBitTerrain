#include <stdio.h>
#include <math.h>
#include "EightBitMap.h"



EightBitMap::EightBitMap(const char* strFilePath, unsigned char cRidCol,unsigned char cBounaryCol,unsigned char cBackgroundCol):
																									m_RID_COLOR(cRidCol),
																									m_BOUNDARY_COLOR(cBounaryCol),
																									m_BACKGROUND_COLOR(cBackgroundCol),
																									m_pOrignCols(0),
																									m_pReviseCols(0)
{
	FILE* pFileToRead;
	pFileToRead = fopen(strFilePath,"rb");

	if(pFileToRead != NULL)
	{
		fseek(pFileToRead,0,SEEK_END);
		m_nFileLength  = ftell(pFileToRead);
		fseek(pFileToRead,0,SEEK_SET);

		m_nWidth  = sqrt(static_cast<double>(m_nFileLength));
		m_nHeight = sqrt(static_cast<double>(m_nFileLength));

		m_pOrignCols  = new unsigned char [m_nFileLength];
		m_pReviseCols = new unsigned char [m_nFileLength];

		if(!m_pOrignCols || !m_pReviseCols)
			throw std::bad_alloc();

		fread(m_pOrignCols,sizeof(unsigned char),m_nFileLength,pFileToRead);
		fclose(pFileToRead);

		InitOrignMap();
	}
	else
		throw std::runtime_error(" can't  open the picture\n");

}																											

																									
EightBitMap::~EightBitMap()
{
	if(m_pOrignCols != NULL)
		delete [] m_pOrignCols;

	if(m_pReviseCols != NULL)
		delete [] m_pReviseCols;
}


void EightBitMap::PadWhiteInOrign()
{
	__int32 nPiexPos;

	for(__int16 nY = 1; nY < m_nHeight - 1; nY++)
	{
		for(__int16 nX = 1; nX < m_nWidth - 1; nX++)
		{
			nPiexPos = nY * m_nWidth + nX;

			if(m_pOrignCols[nPiexPos] == m_RID_COLOR  && m_pOrignCols[nPiexPos + m_nWidth] != m_RID_COLOR)
			{
				if(m_pOrignCols[nPiexPos - 1] != m_RID_COLOR && m_pOrignCols[nPiexPos + m_nWidth - 1] == m_RID_COLOR)
					m_pOrignCols[nPiexPos - 1] = m_RID_COLOR;

				if(m_pOrignCols[nPiexPos + 1] != m_RID_COLOR && m_pOrignCols[nPiexPos + m_nWidth + 1] == m_RID_COLOR )
					m_pOrignCols[nPiexPos + 1] = m_RID_COLOR;
			}
		}
	}
	
}

void EightBitMap::PadSegmentBorder(__int16 nX1, __int16 nY1, __int16 nX2, __int16 nY2, const __int8 (&Dir)[2], unsigned char cCol)
{
	if(nX1 < 0 || nX1 > m_nWidth - 1 || nY1 < 0 || nY1 > m_nHeight - 1 || nX2 < 0 || nX2 > m_nWidth - 1 || nY2 < 0 || nY2 > m_nHeight - 1)
		throw std::runtime_error("nX1 nX2 nY1 nY2 cross the border\n");

	for(; nX1 <= nX2 && nY1 <= nY2;)
	{
		m_pOrignCols[nY1 * m_nWidth + nX1] = cCol;
		
		nX1 += Dir[0];
		nY1 += Dir[1];
	}
}

void EightBitMap::InitOrignMap()
{
	PadWhiteInOrign();

	PadSegmentBorder(1,			   1, 			  m_nWidth - 2, 1,             DIRECTION[0], m_BOUNDARY_COLOR);
	PadSegmentBorder(1,			   m_nHeight - 2, m_nWidth - 2, m_nHeight - 2, DIRECTION[0], m_BOUNDARY_COLOR);
	PadSegmentBorder(1,		       1,			  1,			m_nHeight - 2, DIRECTION[2], m_BOUNDARY_COLOR);
	PadSegmentBorder(m_nWidth - 2, 1,			  m_nWidth - 2, m_nHeight - 2, DIRECTION[2], m_BOUNDARY_COLOR);

	PadSegmentBorder(0,			   0, 			  m_nWidth - 1, 0,             DIRECTION[0], m_BACKGROUND_COLOR);
	PadSegmentBorder(0,			   m_nHeight - 1, m_nWidth - 1, m_nHeight - 1, DIRECTION[0], m_BACKGROUND_COLOR);
	PadSegmentBorder(0,		       0,			  0,			m_nHeight - 1, DIRECTION[2], m_BACKGROUND_COLOR);
	PadSegmentBorder(m_nWidth - 1, 0,			  m_nWidth - 1, m_nHeight - 1, DIRECTION[2], m_BACKGROUND_COLOR);

	memcpy(m_pReviseCols,m_pOrignCols,m_nFileLength);

}

void EightBitMap::XDirToGradual()
{
	__int16 nX1, nY1, nX2, nY2;
	__int32 nBasePos, nPiexPos;

	for(__int16 nY = 1; nY < m_nHeight - 1; nY++)
	{
		nX1 = 0,   nX2 = 0;
		nY1 = nY,  nY2 = nY;

	    nBasePos = nY * m_nHeight;

		for(__int16 nX = 1; nX < m_nWidth - 1; nX++)
		{
		    nPiexPos = nBasePos + nX;
			
			if(m_pOrignCols[nPiexPos] == m_BOUNDARY_COLOR)
			{
				if(nX1 != 0)
				{
					nX2 = nX;
					LetSegmentGradual(nX1,nY1,nX2,nY2,DIRECTION[0]);
				}
				else
					nX1 = nX;
			}
		}
	
	}

}


void EightBitMap::YDirToGradual()
{
	__int16 nX1, nY1;
	__int16 nX2, nY2;
	__int32 nPiexPos;

	for(__int16 nX = 1; nX < m_nWidth - 1; nX++)
	{
		nY1 = 0,  nY2 = 0;
		nX1 = nX, nX2 = nX;

		for(__int16 nY = 1; nY < m_nHeight - 1; nY++)
		{
			nPiexPos = nY * m_nWidth +  nX;
			
			if(m_pOrignCols[nPiexPos] == m_BOUNDARY_COLOR)
			{
				if(nY1 != 0)
				{
					nY2 = nY;
					LetSegmentGradual(nX1,nY1,nX2,nY2,DIRECTION[6]);
				}
				else
					nY1 = nY;
			}
		}

	}

}

void EightBitMap::RigDowDirToGradual()
{
     __int16 nX1, nY1, nOriX, nOriY;
     __int16 nX2, nY2, nYNum, nXNum;
	 __int32 nPiexPos;

	// 左下方三角形

	for(nYNum = (m_nHeight - 1); nYNum >= 0; nYNum--) 
	{
		nX1   = 0,	nY1   = nYNum;
		nX2   = 0,	nY2   = nYNum;
		nOriX = 0,	nOriY = nYNum;
	
		for(nXNum = 0; nXNum <= m_nHeight - nYNum -1; nXNum++)
		{
			nOriX += 1;
			nOriY += 1;

		    nPiexPos = nOriY * m_nWidth + nOriX;

		    if(m_pOrignCols[nPiexPos] == m_BOUNDARY_COLOR)
			{
				if(nX1 != 0)
				{
					nX2 = nOriX, nY2 = nOriY;
					LetSegmentGradual(nX1,nY1,nX2,nY2,DIRECTION[7]);
				}
				else
					nX1 = nOriX, nY1 = nOriY;
			}

		}
	}

	//右上方三角形

	for(nXNum = 0; nXNum < m_nWidth - 1; nXNum++)
	{
		nX1   = nXNum,	 nY1   = 0;
		nX2   = nXNum,	 nY2   = 0;
		nOriX = nXNum,   nOriY = 0;

		for(nYNum = 0; nYNum <= m_nHeight - nXNum - 1; nYNum++)
		{
			nOriX += 1;
			nOriY += 1;

			
			nPiexPos = nOriY * m_nWidth + nOriX;

		    if(m_pOrignCols[nPiexPos] == m_BOUNDARY_COLOR)
			{
				if(nY1 != 0)
				{
					nX2 = nOriX, nY2 = nOriY;
					LetSegmentGradual(nX1,nY1,nX2,nY2,DIRECTION[7]);

				}
				else
					nX1 = nOriX, nY1 = nOriY;
			}
		}
	}

}

void EightBitMap::LefDowDirToGradual()
{
	__int16 nX1, nY1, nOriX, nOriY;
	__int16 nX2, nY2, nYNum, nXNum;
	__int32 nPiexPos;

	//左上方三角形

	for(nXNum = 0; nXNum < m_nWidth; nXNum++)
	{
		nX1   = nXNum, nY1   = 0;
		nX2   = nXNum, nY2   = 0;
		nOriX = nXNum, nOriY = 0;

		for(nYNum = 0; nYNum < nXNum; nYNum++)
		{
			nOriX -= 1;
			nOriY += 1;

			nPiexPos = nOriY * m_nWidth + nOriX;

		    if(m_pOrignCols[nPiexPos] == m_BOUNDARY_COLOR)
			{
				if(nY1 != 0)
				{
					nX2 = nOriX, nY2 = nOriY;
					LetSegmentGradual(nX1,nY1,nX2,nY2,DIRECTION[5]);
				}
				else
					nX1 = nOriX, nY1 = nOriY;

			}

		}
	}

	//右下方三角形

	for(nYNum = 1; nYNum < m_nHeight; nYNum++)
	{
		nX1   = m_nWidth - 1,    nY1   = nYNum;
		nX2   = m_nWidth - 1,    nY2   = nYNum;
		nOriX = m_nWidth - 1,    nOriY = nYNum;

		for(nXNum = 0; nXNum < m_nWidth - nYNum; nXNum++)
		{
			nPiexPos = nOriY * m_nWidth + nOriX;

		    if(m_pOrignCols[nPiexPos] == m_BOUNDARY_COLOR)
			{
				if(nX1 != m_nWidth - 1)
				{
					nX2 = nOriX, nY2 = nOriY;
					LetSegmentGradual(nX1,nY1,nX2,nY2,DIRECTION[5]);
				}
				else
					nX1 = nOriX, nY1 = nOriY; 
			}
			
			nOriX -= 1, nOriY += 1;
		}
	}

}

void EightBitMap::AllDirectionToGradual() 
{

	//原理： 无论重哪个方向渐变，都选取2个点,在两个点之间渐变,只是方向不一样，
	//DIRECTION[0],DIRECTION[7],DIRECTION[6],DIRECTION[5]四个方向

	XDirToGradual();
	YDirToGradual();
	RigDowDirToGradual();
	LefDowDirToGradual();
}

void EightBitMap::BounaryColToMid()
{
	__int32 nBasePos,nPiexPos;

	for(__int16 nY = 1; nY < m_nHeight - 1; ++nY)
	{
		nBasePos = nY * m_nWidth;

		for(__int16 nX = 1; nX < m_nWidth; ++nX)
		{
			nPiexPos = nBasePos + nX;

			if(m_pReviseCols[nPiexPos] == m_RID_COLOR)
			{
			    __int16 nMin = 300;
			    __int16 nMax = -1;

				for(__int8 k = 0; k < 8; k++)
				{
					int nNextPos = (nY + DIRECTION[k][0]) * m_nWidth + (nX + DIRECTION[k][1]);

					if(m_pReviseCols[nNextPos] == m_RID_COLOR)
						continue;

					if(m_pReviseCols[nNextPos] < nMin)
						nMin = m_pReviseCols[nNextPos];

					if(m_pReviseCols[nNextPos] > nMax)
						nMax = m_pReviseCols[nNextPos];
				}

				if(nMin != 300 && nMax != -1)
					m_pReviseCols[nPiexPos] = (nMin + nMax) / 2;
				 
			}
		}
	}
}

void EightBitMap::LetMapSmooth(__int8 nSmooths)
{
	__int32 nBasePos, nPiexPos, nSumPiex;

	for(; nSmooths > 0; --nSmooths)
	{
		for(int nY = 3; nY < m_nHeight - 3; nY++)
		{
			nBasePos =  nY * m_nWidth;

			for(int nX = 3; nX < m_nWidth - 3; nX++)
			{
			    nPiexPos = 	nBasePos + nX;
				nSumPiex = m_pReviseCols[nPiexPos];
				
				for(int k = 0; k < 8; k++)
				{
					int nNextPos = (nY + DIRECTION[k][0]) * m_nWidth + (nX + DIRECTION[k][1]);
					nSumPiex += m_pReviseCols[nNextPos];
				}

				m_pReviseCols[nPiexPos] = nSumPiex / 9;
			}
		}
	}
}

void EightBitMap::SaveToFile(const char* strFilePath)const
{
	FILE* pFileToWrite;
	
	pFileToWrite = fopen(strFilePath,"wb");

	if(pFileToWrite != NULL)
	{
		if(m_pReviseCols)
		{
			fwrite(m_pReviseCols,sizeof(unsigned char),m_nFileLength,pFileToWrite);
			fclose(pFileToWrite);
		}
		else
			throw std::runtime_error("m_pOrignCols is NULL content\n");

	}
	else
		throw std::runtime_error("can't create the file\n");
}
