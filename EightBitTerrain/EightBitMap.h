///////////////////////////////////////////////////////////////////////////////////////////////////
//developer:					yujj										                     //
//E_mail:						1554548129@qq.com							                     //
//date:							20121113 - 20121119							                     //
///////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef __EIGHTBITMAP
#define __EIGHTBITMAP
#include <iostream>


///////////////////////////////////////////////////////////////////////////////////////////////////


enum SEGMENTSTATE{
	INCREAS    = 0x00,			//单增段
	DECREAS    = 0x01,			//单减段
	PEAK	   = 0x02,			//凸段
	PIT        = 0x04,			//凹段
	SPEPIT	   = 0x03,			//特殊凹段,自身为背景色
	INVALID    = 0x05,			//(备用:无效段)
};

const __int8 DIRECTION[8][2] = {	
	{ 1,  0},                   //X+  Y0
	{ 1,  1},                   //X+  Y+
	{ 0,  1},                   //Y+  X0
	{-1,  1},                   //X-  Y+
	{-1,  0},                   //X-  Y0
	{-1, -1},                   //X-  X-
	{ 0, -1},                   //Y-  X0
	{ 1, -1}                    //X+  Y-
};


///////////////////////////////////////////////////////////////////////////////////////////////////


template <class T>
class PreventEightBitMapInherit
{
	friend						T;
private:
	explicit					PreventEightBitMapInherit (){}
	virtual						~PreventEightBitMapInherit(){}
};


///////////////////////////////////////////////////////////////////////////////////////////////////


class EightBitMap : public virtual PreventEightBitMapInherit<EightBitMap>
{
private:
	const unsigned char			m_RID_COLOR;
    const unsigned char			m_BOUNDARY_COLOR;
	const unsigned char			m_BACKGROUND_COLOR; 
	__int32						m_nFileLength;
	__int16						m_nWidth;
	__int16						m_nHeight;
	unsigned char*				m_pOrignCols;
	unsigned char*				m_pReviseCols;

public:
	explicit					EightBitMap(const char* strFilePath, unsigned char cRidCol = 0xff,unsigned char cBounaryCol = 0xff, unsigned char cBackgroundCol = 0x00);
								~EightBitMap();						
private:
								EightBitMap(const EightBitMap &rhs);
	EightBitMap&				operator = (const EightBitMap &rhs);

public:
	void						AllDirectionToGradual(void);
	void						BounaryColToMid(void);	
	void						SaveToFile(const char* strFilePath)const;
	void						LetMapSmooth(__int8 nSmooths = 1);

private:
	void						PadWhiteInOrign(void);
	void						PadSegmentBorder(__int16 nX1, __int16 nY1, __int16 nX2, __int16 nY2, const __int8 (&Dir)[2], unsigned char cCol);
	void						InitOrignMap(void);
	void						XDirToGradual(void);
	void						YDirToGradual(void);
	void						RigDowDirToGradual(void);
	void						LefDowDirToGradual(void);
	inline void					LetSegmentGradual(__int16& nX1, __int16& nY1, __int16 nX2, __int16 nY2, const __int8 (&Dir)[2]);
	inline SEGMENTSTATE			BeSureSegmentState(__int16 nX1, __int16 nY1, __int16 nX2, __int16 nY2, const __int8 (&Dir)[2], unsigned char& bCurClo, unsigned char& bLowClo, unsigned char& bHigClo);

};


///////////////////////////////////////////////////////////////////////////////////////////////////


inline SEGMENTSTATE	EightBitMap::BeSureSegmentState(__int16 nX1, __int16 nY1, __int16 nX2, __int16 nY2, const __int8 (&Dir)[2],unsigned char& bMidCol,unsigned char& bLowCol, unsigned char& bHigCol)
{
	if(nX1 < 0 || nX1 > m_nWidth - 1 || nY1 < 0 || nY1 > m_nHeight - 1 || nX2 < 0 || nX2 > m_nWidth - 1 || nY2 < 0 || nY2 > m_nHeight - 1)
		throw std::runtime_error("nX1 nX2 nY1 nY2 cross the border\n");

	__int16 nX, nY;

	for(nX = nX1, nY = nY1; m_pOrignCols[nY*m_nWidth + nX] == m_BOUNDARY_COLOR;
		 nX -= Dir[0], nY += Dir[1]);
	bLowCol = m_pOrignCols[nY*m_nWidth + nX];


	for(nX = nX1, nY = nY1; m_pOrignCols[nY*m_nWidth + nX] == m_BOUNDARY_COLOR;
		 nX += Dir[0], nY -= Dir[1]);
	bMidCol = m_pOrignCols[nY*m_nWidth + nX];


	for(nX = nX2, nY = nY2; m_pOrignCols[nY*m_nWidth + nX] == m_BOUNDARY_COLOR;
		nX += Dir[0], nY -= Dir[1]);
	bHigCol = m_pOrignCols[nY*m_nWidth + nX];

	
	if(bMidCol < bHigCol)
	{
		return (bMidCol < bLowCol) ? PIT : INCREAS;
	}
	else if(bMidCol > bHigCol)
	{
		return (bLowCol <=  bMidCol) ? PEAK : DECREAS;
	}
	else
	{
		return DECREAS;
	}

}


///////////////////////////////////////////////////////////////////////////////////////////////////


inline void EightBitMap::LetSegmentGradual(__int16& nX1, __int16& nY1, __int16 nX2, __int16 nY2, const __int8 (&Dir)[2])
{
	if(nX1 < 0 || nX1 > m_nWidth - 1 || nY1 < 0 || nY1 > m_nHeight - 1 || nX2 < 0 || nX2 > m_nWidth - 1 || nY2 < 0 || nY2 > m_nHeight - 1)
		throw std::runtime_error("nX1 nX2 nY1 nY2 cross the border\n");

	__int32       nX, nY, nMidX, nMidY;
	__int32		  nLowWeight, nDvalue;
	unsigned char bMidCol,bLowCol,bHigCol;

	SEGMENTSTATE eSegmentState = BeSureSegmentState(nX1,nY1,nX2,nY2,Dir,bMidCol,bLowCol,bHigCol);
	switch(eSegmentState)
	{
	case INCREAS:
		nDvalue = Dir[0] > 0 ? nX2 - nX1 : nY2 - nY1;
		for(nX = nX1 + Dir[0], nY= nY1 - Dir[1], nLowWeight = 0;  nLowWeight < nDvalue;  nX += Dir[0], nY -= Dir[1], nLowWeight++)
			m_pReviseCols[nY * m_nWidth + nX] = bHigCol*(static_cast<double>(nLowWeight)/nDvalue) + bMidCol*(static_cast<double>(nDvalue - nLowWeight)/nDvalue) > m_pReviseCols[nY * m_nWidth + nX] 
											  ? bHigCol*(static_cast<double>(nLowWeight)/nDvalue) + bMidCol*(static_cast<double>(nDvalue - nLowWeight)/nDvalue) : m_pReviseCols[nY * m_nWidth + nX];
	
		break;

	case DECREAS:
		nDvalue = Dir[0] > 0 ? nX2 - nX1 : nY2 - nY1;
		for(nX = nX1 + Dir[0], nY= nY1 - Dir[1], nLowWeight = 0;  nLowWeight < nDvalue;  nX += Dir[0], nY -= Dir[1], nLowWeight++)
			m_pReviseCols[nY * m_nWidth + nX] = bMidCol*(static_cast<double>(nLowWeight)/nDvalue) + bLowCol*(static_cast<double>(nDvalue - nLowWeight)/nDvalue) > m_pReviseCols[nY * m_nWidth + nX]
											  ? bMidCol*(static_cast<double>(nLowWeight)/nDvalue) + bLowCol*(static_cast<double>(nDvalue - nLowWeight)/nDvalue) : m_pReviseCols[nY * m_nWidth + nX];
		
		break;

	case PIT:
		nMidX = (nX1 + nX2)/2;
		nMidY = (nY1 + nY2)/2;

		nDvalue = Dir[0] > 0 ? nMidX - nX1 : nMidY - nY1;
		for(nX = nX1 + Dir[0], nY= nY1 - Dir[1], nLowWeight = 0;  nLowWeight < nDvalue;  nX += Dir[0], nY -= Dir[1], nLowWeight++)
			m_pReviseCols[nY * m_nWidth + nX] = bMidCol*(static_cast<double>(nLowWeight)/nDvalue) + bLowCol*(static_cast<double>(nDvalue - nLowWeight)/nDvalue) > m_pReviseCols[nY * m_nWidth + nX]
											  ? bMidCol*(static_cast<double>(nLowWeight)/nDvalue) + bLowCol*(static_cast<double>(nDvalue - nLowWeight)/nDvalue) : m_pReviseCols[nY * m_nWidth + nX];
	
		nDvalue = Dir[0] > 0 ? nX2 - nMidX : nY2 - nMidY;
		for(nX = nMidX, nY= nMidY, nLowWeight = 0;  nLowWeight < nDvalue;  nX += Dir[0], nY -= Dir[1], nLowWeight++)
			m_pReviseCols[nY * m_nWidth + nX] = bHigCol*(static_cast<double>(nLowWeight)/nDvalue) + bMidCol*(static_cast<double>(nDvalue - nLowWeight)/nDvalue) > m_pReviseCols[nY * m_nWidth + nX]
											  ? bHigCol*(static_cast<double>(nLowWeight)/nDvalue) + bMidCol*(static_cast<double>(nDvalue - nLowWeight)/nDvalue) : m_pReviseCols[nY * m_nWidth + nX];
		break;

	case PEAK:
		break;

	case SPEPIT:
		break;
		
	default :
		throw std::runtime_error("ePointState is not sure");
		break;
	}

	nX1 = nX2, nY1 = nY2;

}


///////////////////////////////////////////////////////////////////////////////////////////////////

#endif